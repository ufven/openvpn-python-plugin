*********************
OpenVPN Python Plugin
*********************

This plug-in adds support for a custom event handler written in Python.

Building
========

Be sure that you have the following dependencies installed:

* ``autoconf``
* ``automake``
* ``libtool``
* ``openvpn-devel``
* ``python3-devel``

To build, you will need to configure the sources, e.g.:

.. code-block:: console

  $ autoreconf -i
  $ ./configure --prefix=/usr

If OpenVPN is installed in a non-default location, the plugin directory can be
override using the ``PLUGINDIR`` environment variable, e.g.:

.. code-block:: console

  $ PLUGINDIR=/opt/openvpn/plugins ./configure --prefix=/usr

Once configured, build and install the plugin:

.. code-block:: console

  $ make
  # make install

Usage
=====

Python
------

There is a module provided (``openvpn_plugin``) containing an abstract base
class (``BaseHandler``) that should be extended by a custom class in a custom
module. This custom module has to be available for the plugin to import as it
is loaded by OpenVPN. The plugin expects a class named ``Handler`` to be
available within this module, with a method named ``handle`` (please see the
example below.)

Example
~~~~~~~

An example starting module could look like this:

.. code-block:: python

  from typing import Dict, List
  from openvpn_plugin import BaseHandler
  from openvpn_plugin import Event as OpenVPNEvent
  from openvpn_plugin import Result as OpenVPNResult


  class Handler(BaseHandler):
      def __init__(self, args: List[str], env: Dict[str, str]):
	  pass

      def shutdown(self):
          pass

      def handle(self, event: int, args: List[str], env: Dict[str, str]) -> int:
          if event == OpenVPNEvent.AUTH_USER_PASS_VERIFY:
              # ...
              return OpenVPNResult.DEFERRED
          elif event == OpenVPNEvent.ENABLE_PF:
              # Do not filter any packages.
              return OpenVPNResult.ERROR

          return OpenVPNResult.SUCCESS

This module must then be installed in such a way that it can be loaded by a
Python interpreter, e.g. by installing it in ``site-packages``, or extending
the ``PYTHONPATH`` environment variable.

OpenVPN Configuration
---------------------

Add the following to your OpenVPN configuration file (adjusting the path as
required):

.. code-block::

  plugin /usr/lib64/openvpn/plugins/openvpn-python-plugin.so <module> [args]

The ``module`` is the name of the Python module to load, containing the
``Handler`` class. Any additional arguments specified is sent to an instance of
the ``Handler`` class.

Example
~~~~~~~

Given the example module presented above, if it is placed in any of the
system's default ``sys.path`` directories (e.g.
``/usr/lib/python3.7/site-packages``) as ``myplugin.py``, it would be loaded
using the following configuration line:

.. code-block::

  plugin /usr/lib64/openvpn/plugins/openvpn-python-plugin.so myplugin

In order to place it outside of the default locations, set the ``PYTHONPATH``
environment variable for the OpenVPN process. If using ``systemd``, this could
accomplished by overriding the instance's unit file and then (re-)starting the
service, e.g.:

.. code-block:: console

  # mkdir /etc/systemd/system/openvpn-server@example.service.d
  # cat >/etc/systemd/system/openvpn-server@example.service.d/override.conf <<EOF
  [Service]
  Environment=PYTHONPATH=/my/custom/path
  EOF
  # systemctl daemon-reload
  # systemctl restart openvpn-server@example
