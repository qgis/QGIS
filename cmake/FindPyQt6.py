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
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

import os.path
import PyQt6.QtCore
import sipconfig
import sys

cfg = sipconfig.Configuration()
sip_dir = cfg.default_sip_dir
for p in (os.path.join(sip_dir, "PyQt6"),
          os.path.join(sip_dir, "PyQt6-3"),
          sip_dir,
          os.path.join(cfg.default_mod_dir, "PyQt6", "bindings")):
    if os.path.exists(os.path.join(p, "QtCore", "QtCoremod.sip")):
        sip_dir = p
        break

print("pyqt_version_str:%s" % PyQt6.QtCore.PYQT_VERSION_STR)
print("pyqt_mod_dir:%s" % os.path.join(cfg.default_mod_dir, "PyQt6"))
print("pyqt_sip_dir:%s" % sip_dir)
print("pyqt_sip_flags:%s" % PyQt6.QtCore.PYQT_CONFIGURATION['sip_flags'])
print("pyqt_bin_dir:%s" % cfg.default_bin_dir)

try:
    import PyQt6.sip

    print("pyqt_sip_module:PyQt6.sip")
except Exception as e:
    print("pyqt_sip_module:sip")
