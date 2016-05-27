# -*- coding: utf-8 -*-

"""
***************************************************************************
    DeletePreconfiguredAlgorithmAction.py
    ---------------------
    Date                 : April 2016
    Copyright            : (C) 2016 by Victor Olaya
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
__date__ = 'April 2016'
__copyright__ = '(C) 2016, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
from PyQt4.QtGui import QMessageBox
from processing.gui.ContextAction import ContextAction
from processing.preconfigured.PreconfiguredAlgorithm import PreconfiguredAlgorithm
from processing.core.alglist import algList


class DeletePreconfiguredAlgorithmAction(ContextAction):

    def __init__(self):
        self.name = self.tr('Delete preconfigured algorithm', 'DeletePreconfiguredAlgorithmAction')

    def isEnabled(self):
        return isinstance(self.itemData, PreconfiguredAlgorithm)

    def execute(self):
        reply = QMessageBox.question(None,
                                     self.tr('Confirmation', 'DeletePreconfiguredAlgorithmAction'),
                                     self.tr('Are you sure you want to delete this algorithm?',
                                             'DeletePreconfiguredAlgorithmAction'),
                                     QMessageBox.Yes | QMessageBox.No,
                                     QMessageBox.No)
        if reply == QMessageBox.Yes:
            os.remove(self.itemData.descriptionFile)
            algList.reloadProvider('preconfigured')
