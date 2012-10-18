# -*- coding: utf-8 -*-
"""Find QScintilla2 PyQt4 module version.

.. note:: Redistribution and use is allowed according to the terms of the BSD
license. For details see the accompanying COPYING-CMAKE-SCRIPTS file.
"""
__author__ = 'Larry Shaffer (larry@dakotacarto.com)'
__date__ = '22/10/2012'
__copyright__ = 'Copyright 2012, The Quantum GIS Project'


try:
    from PyQt4.Qsci import QSCINTILLA_VERSION_STR
    VER = QSCINTILLA_VERSION_STR
except ImportError, e:
    VER = ""


print("qsci_version_str:%s" % VER)
