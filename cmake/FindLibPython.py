
# Copyright (c) 2007, Simon Edwards <simon@simonzone.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

import sys
import distutils.sysconfig

print("exec_prefix:%s" % sys.exec_prefix)
print("short_version:%s" % sys.version[:3])
print("long_version:%s" % sys.version.split()[0])
print("py_inc_dir:%s" % distutils.sysconfig.get_python_inc())
print("site_packages_dir:%s" % distutils.sysconfig.get_python_lib(plat_specific=1))
