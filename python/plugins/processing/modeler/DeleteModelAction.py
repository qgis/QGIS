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
from qgis.PyQt.QtWidgets import QMessageBox
from processing.gui.ContextAction import ContextAction
from processing.modeler.ModelerAlgorithm import ModelerAlgorithm
from processing.core.alglist import algList


class DeleteModelAction(ContextAction):

    def __init__(self):
        self.name = self.tr('Delete model', 'DeleteModelAction')

    def isEnabled(self):
        return isinstance(self.itemData, ModelerAlgorithm)

    def execute(self):
        reply = QMessageBox.question(
            None,
            self.tr('Confirmation', 'DeleteModelAction'),
            self.tr('Are you sure you want to delete this model?', 'DeleteModelAction'),
            QMessageBox.Yes | QMessageBox.No,
            QMessageBox.No)
        if reply == QMessageBox.Yes:
            os.remove(self.itemData.descriptionFile)
            algList.reloadProvider('model')
