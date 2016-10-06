# -*- coding: utf-8 -*-

"""
***************************************************************************
    pyuic.py
    ---------------------
    Date                 : February 2016
    Copyright            : (C) 2016 by Jürgen E. Fischer
    Email                : jef at norbit dot de
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Jürgen E. Fischer'
__date__ = 'February 2016'
__copyright__ = '(C) 2016, Jürgen E. Fischer'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


import sip
for api in ["QDate", "QDateTime", "QString", "QTextStream", "QTime", "QUrl", "QVariant"]:
    sip.setapi(api, 2)

from PyQt4.uic import pyuic
