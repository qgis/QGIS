# -*- coding: utf-8 -*-
#
#   Copyright (c) 2012, Larry Shaffer <larry@dakotacarto.com>
#    All rights reserved.
#
#    Redistribution and use in source and binary forms, with or without
#    modification, are permitted provided that the following conditions are met:
#        * Redistributions of source code must retain the above copyright
#        notice, this list of conditions and the following disclaimer.
#        * Redistributions in binary form must reproduce the above copyright
#        notice, this list of conditions and the following disclaimer in the
#        documentation and/or other materials provided with the distribution.
#        * Neither the name of the  Larry Shaffer <larry@dakotacarto.com> nor the
#        names of its contributors may be used to endorse or promote products
#        derived from this software without specific prior written permission.
#
#    THIS SOFTWARE IS PROVIDED BY Larry Shaffer <larry@dakotacarto.com> ''AS IS'' AND ANY
#    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#    DISCLAIMED. IN NO EVENT SHALL Larry Shaffer <larry@dakotacarto.com> BE LIABLE FOR ANY
#    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
"""Find QScintilla2 PyQt4 module version.

.. note:: Redistribution and use is allowed according to the terms of the BSD
license. For details see the accompanying COPYING-CMAKE-SCRIPTS file.
"""
__author__ = 'Larry Shaffer (larry@dakotacarto.com)'
__date__ = '22/10/2012'
__copyright__ = 'Copyright 2012, The QGIS Project'


try:
    from PyQt4.Qsci import QSCINTILLA_VERSION_STR
    VER = QSCINTILLA_VERSION_STR
except ImportError:
    VER = ""


print("qsci_version_str:%s" % VER)
