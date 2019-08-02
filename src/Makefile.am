plugin_LTLIBRARIES = \
  openvpn-python-plugin.la

openvpn_python_plugin_la_SOURCES = \
  plugin.c \
  plugin.h

openvpn_python_plugin_la_LDFLAGS = \
  $(AM_LDFLAGS) \
  $(PYTHON3_LDFLAGS) \
  -module \
  -avoid-version \
  -shared \
  -no-undefined

openvpn_python_plugin_la_CFLAGS = \
  -D_POSIX_C_SOURCE=200809L \
  $(AM_CFLAGS) \
  $(PYTHON3_CFLAGS) \
  $(PYTHON3_LIBS)

python_PYTHON = openvpn_plugin.py
