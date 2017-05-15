# -*- coding: utf-8 -*-

"""
***************************************************************************
    QgisAlgorithm.py
    ----------------
    Date                 : May 2017
    Copyright            : (C) 2017 by Nyall Dawson
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
__date__ = 'May2017'
__copyright__ = '(C) 2017, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from processing.core import GeoAlgorithm
from processing.algs.help import shortHelp


class QgisAlgorithm(QgisAlgorithm):

    def __init__(self):
        super().__init__()

    def shortHelpString(self):
        return shortHelp.get(self.id(), None)
