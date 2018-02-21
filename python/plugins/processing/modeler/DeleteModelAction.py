# -*- coding: utf-8 -*-

"""
***************************************************************************
    DeleteModelAction.py
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

import os
from qgis.core import QgsApplication, QgsProcessingModelAlgorithm
from qgis.PyQt.QtWidgets import QMessageBox
from qgis.PyQt.QtCore import QCoreApplication
from processing.gui.ContextAction import ContextAction


class DeleteModelAction(ContextAction):

    def __init__(self):
        self.name = QCoreApplication.translate('DeleteModelAction', 'Delete Model…')

    def isEnabled(self):
        return isinstance(self.itemData, QgsProcessingModelAlgorithm)

    def execute(self):
        reply = QMessageBox.question(
            None,
            self.tr('Delete Model', 'DeleteModelAction'),
            self.tr('Are you sure you want to delete this model?', 'DeleteModelAction'),
            QMessageBox.Yes | QMessageBox.No,
            QMessageBox.No)
        if reply == QMessageBox.Yes:
            os.remove(self.itemData.sourceFilePath())
            QgsApplication.processingRegistry().providerById('model').refreshAlgorithms()
