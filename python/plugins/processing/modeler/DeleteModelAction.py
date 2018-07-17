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
from qgis.core import (QgsApplication,
                       QgsProcessingModelAlgorithm,
                       QgsProject)
from qgis.PyQt.QtWidgets import QMessageBox
from qgis.PyQt.QtCore import QCoreApplication
from processing.gui.ContextAction import ContextAction
from processing.modeler.ProjectProvider import PROJECT_PROVIDER_ID


class DeleteModelAction(ContextAction):

    def __init__(self):
        self.name = QCoreApplication.translate('DeleteModelAction', 'Delete Model…')

    def isEnabled(self):
        return isinstance(self.itemData, QgsProcessingModelAlgorithm)

    def execute(self):
        model = self.itemData
        if model is None:
            return # shouldn't happen, but let's be safe

        project_provider = model.provider().id() == PROJECT_PROVIDER_ID

        if project_provider:
            msg = self.tr('Are you sure you want to delete this model from the current project?', 'DeleteModelAction')
        else:
            msg = self.tr('Are you sure you want to delete this model?', 'DeleteModelAction')

        reply = QMessageBox.question(
            None,
            self.tr('Delete Model', 'DeleteModelAction'),
            msg,
            QMessageBox.Yes | QMessageBox.No,
            QMessageBox.No)

        if reply == QMessageBox.Yes:
            if project_provider:
                provider = QgsApplication.processingRegistry().providerById(PROJECT_PROVIDER_ID)
                provider.remove_model(model)
                QgsProject.instance().setDirty(True)
            else:
                os.remove(model.sourceFilePath())
                QgsApplication.processingRegistry().providerById('model').refreshAlgorithms()
