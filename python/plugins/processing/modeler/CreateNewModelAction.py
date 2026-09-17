"""
***************************************************************************
    CreateNewModelAction.py
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

__author__ = "Victor Olaya"
__date__ = "August 2012"
__copyright__ = "(C) 2012, Victor Olaya"

import os

from qgis.core import QgsApplication
from qgis.gui import QgsProcessingToolboxAction
from qgis.PyQt.QtCore import QCoreApplication

from processing.modeler.ModelerDialog import ModelerDialog

pluginPath = os.path.split(os.path.dirname(__file__))[0]


class CreateNewModelAction(QgsProcessingToolboxAction):
    def __init__(self):
        super().__init__(
            QCoreApplication.translate("CreateNewModelAction", "Create New Model…"),
            QCoreApplication.translate("ToolboxAction", "Tools"),
        )

    def icon(self):
        return QgsApplication.getThemeIcon("/processingModel.svg")

    def trigger(self, context):
        dlg = ModelerDialog.create()
        dlg.modelUpdated.connect(self.updateModel)
        dlg.show()

    def updateModel(self):
        QgsApplication.processingRegistry().providerById("model").refreshAlgorithms()
