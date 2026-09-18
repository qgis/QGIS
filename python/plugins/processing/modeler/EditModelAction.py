"""
***************************************************************************
    EditModelAction.py
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

from qgis.core import QgsApplication
from qgis.gui import QgsProcessingToolboxContextAction
from qgis.PyQt.QtCore import QCoreApplication

from processing.modeler.ModelerDialog import ModelerDialog


class EditModelAction(QgsProcessingToolboxContextAction):
    def __init__(self):
        super().__init__(QCoreApplication.translate("EditModelAction", "Edit Model…"))

    def isCompatibleWithAlgorithm(self, provider_id: str, algorithm_name: str):
        return provider_id in ("model", "project")

    def trigger(self, context):
        provider = QgsApplication.processingRegistry().providerById(
            context.providerId()
        )
        model = provider.algorithm(context.algorithmName())
        dlg = ModelerDialog.create()

        _model = model.create()
        _model.setSourceFilePath(model.sourceFilePath())
        dlg.setModel(_model)

        dlg.modelUpdated.connect(self.updateModel)
        dlg.show()
        dlg.activate()

    def updateModel(self):
        QgsApplication.processingRegistry().providerById("model").refreshAlgorithms()
