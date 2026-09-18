"""
***************************************************************************
    ModelerDialog.py
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

import sys
from pathlib import Path

from qgis.core import (
    Qgis,
    QgsApplication,
    QgsFileUtils,
    QgsProcessing,
    QgsProcessingModelAlgorithm,
    QgsProcessingModelChildAlgorithm,
    QgsProcessingModelParameter,
    QgsProcessingUtils,
    QgsProject,
    QgsSettings,
)
from qgis.gui import (
    QgsGui,
    QgsModelDesignerDialog,
    QgsModelGraphicsScene,
    QgsProcessingContextGenerator,
    QgsProcessingParameterDefinitionDialog,
)
from qgis.PyQt.QtCore import (
    QDir,
    QFileInfo,
    QPoint,
    QPointF,
    QUrl,
    pyqtSignal,
)
from qgis.PyQt.QtWidgets import QFileDialog, QMessageBox
from qgis.utils import iface

from processing.gui.algorithm_widget import AlgorithmWidget
from processing.script.ScriptEditorDialog import ScriptEditorDialog
from processing.tools.dataobjects import createContext


class ModelerDialog(QgsModelDesignerDialog):
    dlgs = []

    @staticmethod
    def create(model=None):
        """
        Workaround crappy sip handling of QMainWindow. It doesn't know that we are using the deleteonclose
        flag, so happily just deletes dialogs as soon as they go out of scope. The only workaround possible
        while we still have to drag around this Python code is to store a reference to the sip wrapper so that
        sip doesn't get confused. The underlying object will still be deleted by the deleteonclose flag though!
        """
        dlg = ModelerDialog(model)
        ModelerDialog.dlgs.append(dlg)
        return dlg

    def __init__(self, model=None, parent=None):
        super().__init__(parent)

        if model is not None:
            _model = model.create()
            _model.setSourceFilePath(model.sourceFilePath())
            self.setModel(_model)

        self.processing_context = createContext()

        class ContextGenerator(QgsProcessingContextGenerator):
            def __init__(self, context):
                super().__init__()
                self.processing_context = context

            def processingContext(self):
                return self.processing_context

        self.context_generator = ContextGenerator(self.processing_context)
        self.registerProcessingContextGenerator(self.context_generator)

    def createExecutionWidget(self):
        widget = AlgorithmWidget(
            self.model().create(),
            parent=self,
            initialState=Qgis.DockableWidgetInitialState.ForceDocked,
        )
        widget.registerProcessingFeedbackGenerator(self)
        return widget

    def autogenerate_parameter_name(self, parameter):
        """
        Automatically generates and sets a new parameter's name, based on the parameter's
        description and ensuring that it is unique for the model.
        """
        safeName = QgsProcessingModelAlgorithm.safeName(parameter.description())
        name = safeName.lower()
        i = 2
        while self.model().parameterDefinition(name):
            name = safeName.lower() + str(i)
            i += 1
        parameter.setName(name)

    def addInput(self, paramType, pos=None):
        if paramType not in [
            param.id()
            for param in QgsApplication.instance().processingRegistry().parameterTypes()
        ]:
            return

        new_param = None
        comment = None
        context = createContext()
        widget_context = self.createWidgetContext()
        dlg = QgsProcessingParameterDefinitionDialog(
            type=paramType,
            context=context,
            widgetContext=widget_context,
            algorithm=self.model(),
        )
        dlg.registerProcessingContextGenerator(self.context_generator)
        if dlg.exec():
            new_param = dlg.createParameter()
            self.autogenerate_parameter_name(new_param)
            comment = dlg.comments()

        if new_param is not None:
            if pos is None or not pos:
                pos = self.getPositionForParameterItem()
            if isinstance(pos, QPoint):
                pos = QPointF(pos)
            component = QgsProcessingModelParameter(new_param.name())
            component.setDescription(new_param.name())
            component.setPosition(pos)

            component.comment().setDescription(comment)
            component.comment().setPosition(
                component.position()
                + QPointF(component.size().width(), -1.5 * component.size().height())
            )

            self.beginUndoCommand(self.tr("Add Model Input"))
            self.model().addModelParameter(new_param, component)
            self.repaintModel()
            self.endUndoCommand()

    def getPositionForParameterItem(self):
        MARGIN = 20
        BOX_WIDTH = 200
        BOX_HEIGHT = 80
        if len(self.model().parameterComponents()) > 0:
            maxX = max(
                [
                    i.position().x()
                    for i in list(self.model().parameterComponents().values())
                ]
            )
            newX = MARGIN + BOX_WIDTH + maxX
        else:
            newX = MARGIN + BOX_WIDTH / 2
        return QPointF(newX, MARGIN + BOX_HEIGHT / 2)

    def addAlgorithm(self, alg_id, pos=None):
        alg = QgsApplication.processingRegistry().createAlgorithmById(alg_id)
        if not alg:
            return

        child_alg = QgsProcessingModelChildAlgorithm(alg_id)
        child_alg.setDescription(alg.displayName())

        if pos is None or not pos:
            child_alg.setPosition(self.getPositionForAlgorithmItem())
        else:
            child_alg.setPosition(pos)

        child_alg.comment().setPosition(
            child_alg.position()
            + QPointF(child_alg.size().width(), -1.5 * child_alg.size().height())
        )

        output_offset_x = child_alg.size().width()
        output_offset_y = 1.5 * child_alg.size().height()
        for out in child_alg.modelOutputs():
            child_alg.modelOutput(out).setPosition(
                child_alg.position() + QPointF(output_offset_x, output_offset_y)
            )
            output_offset_y += 1.5 * child_alg.modelOutput(out).size().height()

        self.beginUndoCommand(self.tr("Add Algorithm"))
        self.model().addChildAlgorithm(child_alg)
        self.repaintModel()
        self.endUndoCommand()

    def getPositionForAlgorithmItem(self):
        MARGIN = 20
        BOX_WIDTH = 200
        BOX_HEIGHT = 80
        if self.model().childAlgorithms():
            maxX = max(
                [
                    alg.position().x()
                    for alg in list(self.model().childAlgorithms().values())
                ]
            )
            maxY = max(
                [
                    alg.position().y()
                    for alg in list(self.model().childAlgorithms().values())
                ]
            )
            newX = MARGIN + BOX_WIDTH + maxX
            newY = MARGIN + BOX_HEIGHT + maxY
        else:
            newX = MARGIN + BOX_WIDTH / 2
            newY = MARGIN * 2 + BOX_HEIGHT + BOX_HEIGHT / 2
        return QPointF(newX, newY)

    def exportAsScriptAlgorithm(self):
        dlg = ScriptEditorDialog(parent=iface.mainWindow())

        dlg.editor.setText(
            "\n".join(
                self.model().asPythonCode(
                    QgsProcessing.PythonOutputType.PythonQgsProcessingAlgorithmSubclass,
                    4,
                )
            )
        )
        dlg.show()
