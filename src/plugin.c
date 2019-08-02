/* This file is part of openvpn-plugin-python.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include "plugin.h"

plugin_log_t ovpn_log = NULL;
plugin_vlog_t ovpn_vlog = NULL;


/**
 * Counts the number of elements in a string array.
 *
 * @param array a string array
 * @return the number of elements in the array
 */
size_t
array_length(const char **array) {
    size_t length;

    /* Iterate over all elements until NULL is encountered. */
    for (length = 0; array[length] != NULL; length++);

    return length;
}

/**
 * Tries to allocate and populate a PyList with all the elements from a given
 * string array.
 * 
 * @param array a string array
 * @return a PyList on success, NULL on error
 */
PyObject *
array_to_pylist(const char **array) {
    PyObject *pList;

    if (NULL != (pList = PyList_New(0))) {
        size_t length = array_length(array);
        size_t i;

        for (i = 0; i < length; i++) {
            PyObject *pValue = PyUnicode_FromString(array[i]);

            if (-1 == PyList_Append(pList, pValue)) {
                Py_DECREF(pList);
                Py_DECREF(pValue);

                ovpn_log(PLOG_ERR, PLUGIN_NAME,
                    "Failed to append item to list: %s", array[i]);

                return NULL;
            }

            Py_DECREF(pValue);
        }
    }

    return pList;
}

/**
 * Tries to allocate and populate a PyDict with all the keys and values from a
 * given string array.
 * 
 * Each entry in the array has to be delimited properly, or the operation will
 * fail.
 * 
 * @param array a string array
 * @param delim a delimiter character
 * @return a PyDict on success, NULL on error
 */
PyObject *
array_to_pydict(const char **array, const char *delim) {
    PyObject *pDict;

    if (NULL != (pDict = PyDict_New())) {
        size_t length = array_length(array);
        size_t i;

        for (i = 0; i < length; i++) {
            char *p = strchr(array[i], *delim);
            PyObject *pKey, *pValue;

            if (p == NULL) {
                /* No delimiter found, malformed input. */
                ovpn_log(PLOG_ERR, PLUGIN_NAME,
                    "Input string is not in key%cvalue format: %s", *delim,
                    array[i]);
                Py_DECREF(pDict);

                return NULL;
            }

            /* Get the key and value. */
            pKey = PyUnicode_FromStringAndSize(array[i],
                (p - array[i]) / sizeof(char));
            pValue = PyUnicode_FromString(p + 1);

            if (-1 == PyDict_SetItem(pDict, pKey, pValue)) {
                Py_DECREF(pDict);
                Py_DECREF(pValue);
                Py_DECREF(pKey);

                return NULL;
            }

            Py_DECREF(pValue);
            Py_DECREF(pKey);
        }
    }

    return pDict;
}

OPENVPN_EXPORT int
openvpn_plugin_open_v3(const int v3structver,
                       struct openvpn_plugin_args_open_in const *args,
                       struct openvpn_plugin_args_open_return *ret) {
    const char **argv = args->argv;
    const char **envp = args->envp;
    size_t argc = array_length(argv);
    plugin_context_t *context;
    PyObject *pModule, *pClass, *pArgs, *pInstance;
    PyObject *pArgsList, *pEnvDict;

    /* Hook into the exported functions from OpenVPN. */
    ovpn_log = args->callbacks->plugin_log;
    ovpn_vlog = args->callbacks->plugin_vlog;

    /* Check that we are API compatible. */
    if (v3structver != OPENVPN_PLUGINv3_STRUCTVER) {
        ovpn_log(PLOG_ERR, PLUGIN_NAME,
            "Incompatible plug-in interface between this plug-in and OpenVPN");

        return OPENVPN_PLUGIN_FUNC_ERROR;
    }

    /* Ensure there are enough arguments passed. */
    if (argc < ARG_IDX_COUNT) {
        ovpn_log(PLOG_ERR, PLUGIN_NAME, "Too few arguments");

        return OPENVPN_PLUGIN_FUNC_ERROR;
    }

    /* Initialize Python. */
    Py_Initialize();
    PyEval_InitThreads();

    /* Convert the arguments to a PyList. */
    if (NULL == (pArgsList = array_to_pylist(argv + 2))) {
        ovpn_log(PLOG_ERR, PLUGIN_NAME, "Failed to convert arguments");
        PyErr_Print();

        return OPENVPN_PLUGIN_FUNC_ERROR;
    }

    /* Convert environment to a PyDict. */
    if (NULL == (pEnvDict = array_to_pydict(envp, "="))) {
        ovpn_log(PLOG_ERR, PLUGIN_NAME, "Failed to convert environment");
        PyErr_Print();

        return OPENVPN_PLUGIN_FUNC_ERROR;
    }

    /* Build an argument list. */
    if (NULL == (pArgs = Py_BuildValue("(OO)", pArgsList, pEnvDict))) {
        ovpn_log(PLOG_ERR, PLUGIN_NAME, "Failed to construct arguments");
        PyErr_Print();

        return OPENVPN_PLUGIN_FUNC_ERROR;
    }

    /* Import the module. */
    if (NULL == (pModule = PyImport_ImportModule(argv[ARG_IDX_MODULE]))) {
        ovpn_log(PLOG_ERR, PLUGIN_NAME, "Failed to import module: %s",
            argv[ARG_IDX_MODULE]);
        PyErr_Print();

        return OPENVPN_PLUGIN_FUNC_ERROR;
    }

    /* Locate the handler class. */
    if (NULL == (pClass = PyObject_GetAttrString(pModule, PLUGIN_HANDLER))) {
        ovpn_log(PLOG_ERR, PLUGIN_NAME, "Failed to find class: %s",
            PLUGIN_HANDLER);
        PyErr_Print();

        return OPENVPN_PLUGIN_FUNC_ERROR;
    }

    if (NULL == (pInstance = PyObject_CallObject(pClass, pArgs))) {
        ovpn_log(PLOG_ERR, PLUGIN_NAME, "Failed to instantiate class: %s",
            PLUGIN_HANDLER);
        PyErr_Print();

        return OPENVPN_PLUGIN_FUNC_ERROR;
    }

    Py_DECREF(pClass);
    Py_DECREF(pModule);
    Py_DECREF(pArgs);
    Py_DECREF(pEnvDict);
    Py_DECREF(pArgsList);

    /* Allocate our context. */
    context = (plugin_context_t *)calloc(1, sizeof(plugin_context_t));
    context->instance = pInstance;

    if (context->instance == NULL)
        return OPENVPN_PLUGIN_FUNC_ERROR;

    /* Define plugin types which our script can serve. */
    ret->type_mask = OPENVPN_PLUGIN_MASK(OPENVPN_PLUGIN_UP)
        | OPENVPN_PLUGIN_MASK(OPENVPN_PLUGIN_DOWN)
        | OPENVPN_PLUGIN_MASK(OPENVPN_PLUGIN_ROUTE_UP)
        | OPENVPN_PLUGIN_MASK(OPENVPN_PLUGIN_IPCHANGE)
        | OPENVPN_PLUGIN_MASK(OPENVPN_PLUGIN_TLS_VERIFY)
        | OPENVPN_PLUGIN_MASK(OPENVPN_PLUGIN_AUTH_USER_PASS_VERIFY)
        | OPENVPN_PLUGIN_MASK(OPENVPN_PLUGIN_CLIENT_CONNECT)
        | OPENVPN_PLUGIN_MASK(OPENVPN_PLUGIN_CLIENT_DISCONNECT)
        | OPENVPN_PLUGIN_MASK(OPENVPN_PLUGIN_LEARN_ADDRESS)
        | OPENVPN_PLUGIN_MASK(OPENVPN_PLUGIN_CLIENT_CONNECT_V2)
        | OPENVPN_PLUGIN_MASK(OPENVPN_PLUGIN_TLS_FINAL)
        | OPENVPN_PLUGIN_MASK(OPENVPN_PLUGIN_ENABLE_PF)
        | OPENVPN_PLUGIN_MASK(OPENVPN_PLUGIN_ROUTE_PREDOWN);
    
    /* Point the global context handle to our newly created context. */
    ret->handle = (void *)context;
    context->thread_state = PyEval_SaveThread();

    ovpn_log(PLOG_NOTE, PLUGIN_NAME, "Loaded");

    return OPENVPN_PLUGIN_FUNC_SUCCESS;
}

OPENVPN_EXPORT int
openvpn_plugin_func_v3(UNUSED(const int version),
                       struct openvpn_plugin_args_func_in const *args,
                       UNUSED(struct openvpn_plugin_args_func_return *retptr)) {
    PyGILState_STATE pGILState = PyGILState_Ensure();
    plugin_context_t *context = (plugin_context_t *)args->handle;
    const char **argv = args->argv;
    const char **envp = args->envp;
    int result = OPENVPN_PLUGIN_FUNC_SUCCESS;
    PyObject *pInstance = context->instance;    
    PyObject *pArgsList, *pEnvDict, *pArgs, *pValue;
    PyObject *pMethod = PyObject_GetAttrString(pInstance, PLUGIN_METHOD_HANDLE);

    if (pMethod != NULL && PyCallable_Check(pMethod) == 1) {
        pArgsList = array_to_pylist(argv + 1);
        pEnvDict = array_to_pydict(envp, "=");
        pArgs = Py_BuildValue("(iOO)", args->type, pArgsList, pEnvDict);

        if (pArgs != NULL) {
            pValue = PyObject_CallObject(pMethod, pArgs);

            if (pValue != NULL) {
                int method_result = PyLong_AsLong(pValue);

                ovpn_log(PLOG_DEBUG,
                         PLUGIN_NAME,
                         "Call result: %i",
                         method_result);

                switch (method_result) {
                    case 0: /* OPENVPN_PLUGIN_FUNC_SUCCESS */
                    case 1: /* OPENVPN_PLUGIN_FUNC_ERROR */
                    case 2: /* OPENVPN_PLUGIN_FUNC_DEFERRED */
                        result = method_result;
                        break;
                    default:
                        result = OPENVPN_PLUGIN_FUNC_ERROR;
                }

                Py_DECREF(pValue);
            } else {
                ovpn_log(PLOG_NOTE, PLUGIN_NAME, "Call failed");
                PyErr_Print();
                
                result = OPENVPN_PLUGIN_FUNC_ERROR;
            }

            Py_DECREF(pArgs);
        }

        if (pArgsList != NULL) Py_DECREF(pArgsList);
        if (pEnvDict != NULL) Py_DECREF(pEnvDict);

        Py_DECREF(pMethod);
    } else
        ovpn_log(PLOG_WARN,
                 PLUGIN_NAME,
                 "No method \"" PLUGIN_METHOD_HANDLE "\" available.");

    PyGILState_Release(pGILState);

    return result;
}

OPENVPN_EXPORT void
openvpn_plugin_close_v1(openvpn_plugin_handle_t handle) {
    plugin_context_t *context = (plugin_context_t *) handle;
    PyObject *instance = context->instance;
    PyGILState_STATE pGILState = PyGILState_Ensure();

    /* Tell the instance to shut down. */
    PyObject *method = PyObject_GetAttrString(instance, PLUGIN_METHOD_SHUTDOWN);

    if (method != NULL && PyCallable_Check(method) == 1)
        PyObject_CallObject(method, NULL);
    else
        ovpn_log(PLOG_WARN,
                 PLUGIN_NAME,
                 "No method \"" PLUGIN_METHOD_SHUTDOWN "\" available.");

    PyGILState_Release(pGILState);
    PyEval_RestoreThread(context->thread_state);

    if (instance != NULL)
        Py_DECREF(instance);

    free(context);

    /* Finalize Python. */
    Py_Finalize();

    ovpn_log(PLOG_NOTE, PLUGIN_NAME, "Unloaded");
}
