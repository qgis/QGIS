# Copyright (c) 2007, Simon Edwards <simon@simonzone.com>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

import PyQt4.pyqtconfig

pyqtcfg = PyQt4.pyqtconfig.Configuration()
print("pyqt_version:%06.0x" % pyqtcfg.pyqt_version)
print("pyqt_version_str:%s" % pyqtcfg.pyqt_version_str)

pyqt_version_tag = ""
in_t = False
for item in pyqtcfg.pyqt_sip_flags.split(' '):
    if item=="-t":
        in_t = True
    elif in_t:
        if item.startswith("Qt_4"):
            pyqt_version_tag = item
    else:
        in_t = False
print("pyqt_version_tag:%s" % pyqt_version_tag)

print("pyqt_sip_dir:%s" % pyqtcfg.pyqt_sip_dir)
print("pyqt_sip_flags:%s" % pyqtcfg.pyqt_sip_flags)
