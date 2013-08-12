#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
***************************************************************************
    ParametersDialog.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

# -*- coding: latin-1 -*-
from processing.gui.ParametersPanel import ParametersPanel
from processing.gui.AlgorithmExecutionDialog import AlgorithmExecutionDialog
from PyQt4 import QtCore


class ParametersDialog(AlgorithmExecutionDialog):

    NOT_SELECTED = "[Not selected]"
    '''the default parameters dialog, to be used when an algorithm is called from the toolbox'''
    def __init__(self, alg):
        self.paramTable = ParametersPanel(self, alg)
        AlgorithmExecutionDialog.__init__(self, alg, self.paramTable)
        self.executed = False
