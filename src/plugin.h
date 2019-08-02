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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <Python.h>
#include <openvpn-plugin.h>

#define PLUGIN_NAME "python-plugin"
#define PLUGIN_HANDLER "Handler"
#define PLUGIN_METHOD_HANDLE "handle"
#define PLUGIN_METHOD_SHUTDOWN "shutdown"

#ifdef __GNUC__
#  define UNUSED(x) __attribute__((unused)) x
#else
#  define UNUSED(x) x
#endif


typedef struct {
    PyObject *instance;
    PyThreadState *thread_state;
} plugin_context_t;


typedef enum {
    ARG_IDX_NAME = 0,
    ARG_IDX_MODULE,
    ARG_IDX_COUNT
} plugin_arg_idx_t;


/**
 * Counts the number of elements in a string array.
 *
 * @param array a string array
 * @return the number of elements in the array
 */
size_t
array_length(const char **array);


/**
 * Tries to allocate and populate a PyList with all the elements from a given
 * string array.
 * 
 * @param array a string array
 * @return a PyList on success, NULL on error
 */
PyObject *
array_to_pylist(const char **array);


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
array_to_pydict(const char **array, const char *delim);
