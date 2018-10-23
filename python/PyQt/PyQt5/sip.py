# -*- coding: utf-8 -*-

"""
***************************************************************************
    sip.py
    ---------------------
    Date                 : October 2018
    Copyright            : (C) 2018 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nyall Dawson'
__date__ = 'October 2017'
__copyright__ = '(C) 2018, Nyall Dawson'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

try:
    from PyQt5.sip import *
except ModuleNotFoundError:
    from sip import *
