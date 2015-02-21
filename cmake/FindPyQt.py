# -*- coding: utf-8 -*-
#
#   Copyright (c) 2007, Simon Edwards <simon@simonzone.com>
#    All rights reserved.
#
#    Redistribution and use in source and binary forms, with or without
#    modification, are permitted provided that the following conditions are met:
#        * Redistributions of source code must retain the above copyright
#        notice, this list of conditions and the following disclaimer.
#        * Redistributions in binary form must reproduce the above copyright
#        notice, this list of conditions and the following disclaimer in the
#        documentation and/or other materials provided with the distribution.
#        * Neither the name of the  Simon Edwards <simon@simonzone.com> nor the
#        names of its contributors may be used to endorse or promote products
#        derived from this software without specific prior written permission.
#
#    THIS SOFTWARE IS PROVIDED BY Simon Edwards <simon@simonzone.com> ''AS IS'' AND ANY
#    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#    DISCLAIMED. IN NO EVENT SHALL Simon Edwards <simon@simonzone.com> BE LIABLE FOR ANY
#    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# FindPyQt.py
# Copyright (c) 2007, Simon Edwards <simon@simonzone.com>
# Copyright (c) 2015, Alexei Ardyakov <ardjakov@rambler.ru>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

import os

def get_pyqt_sip_dir():
    # Only works for a few popular locations of PyQt4 SIP directory.
    import sipconfig
    sipcfg = sipconfig.Configuration()
    default_sip_dir = sipcfg.default_sip_dir
    postfixes = ('Py2-PyQt4', 'PyQt4',)
    for postfix in postfixes:
        pyqt_sip_dir = os.path.join(default_sip_dir, postfix)
        if os.path.exists(pyqt_sip_dir) and os.path.exists(
                os.path.join(pyqt_sip_dir, 'QtCore')):
            return pyqt_sip_dir
    return default_sip_dir


def get_pyqt_bin_dir():
    import sipconfig
    sipcfg = sipconfig.Configuration()
    return sipcfg.default_bin_dir


try:
    import PyQt4.pyqtconfig
    pyqtcfg = PyQt4.pyqtconfig.Configuration()
    pyqt_version = pyqtcfg.pyqt_version
    pyqt_version_str = pyqtcfg.pyqt_version_str
    pyqt_mod_dir = pyqtcfg.pyqt_mod_dir
    pyqt_sip_dir = pyqtcfg.pyqt_sip_dir
    pyqt_sip_flags = pyqtcfg.pyqt_sip_flags
    pyqt_bin_dir = pyqtcfg.pyqt_bin_dir
except ImportError:
    # PyQt4 built with configure-ng.py has no pyqtconfig module
    import PyQt4.QtCore
    pyqt_version = PyQt4.QtCore.PYQT_VERSION
    pyqt_version_str = PyQt4.QtCore.PYQT_VERSION_STR
    pyqt_mod_dir = os.path.dirname(os.path.dirname(PyQt4.QtCore.__file__))
    pyqt_sip_dir = get_pyqt_sip_dir()
    pyqt_sip_flags = PyQt4.QtCore.PYQT_CONFIGURATION["sip_flags"]
    pyqt_bin_dir = get_pyqt_bin_dir()

pyqt_version_tag = ""
in_t = False
for item in pyqt_sip_flags.split(' '):
    if item=="-t":
        in_t = True
    elif in_t:
        if item.startswith("Qt_4"):
            pyqt_version_tag = item
    else:
        in_t = False
        
print("pyqt_version_tag:%s" % pyqt_version_tag)
print("pyqt_version:%06.0x" % pyqt_version)
print("pyqt_version_num:%d" % pyqt_version)
print("pyqt_version_str:%s" % pyqt_version_str)
print("pyqt_mod_dir:%s" % pyqt_mod_dir)
print("pyqt_sip_dir:%s" % pyqt_sip_dir)
print("pyqt_sip_flags:%s" % pyqt_sip_flags)
print("pyqt_bin_dir:%s" % pyqt_bin_dir)



