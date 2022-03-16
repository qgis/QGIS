# -*- coding: utf-8 -*-

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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

import os
import re
import sys
from pathlib import Path

from qgis.PyQt.QtCore import (
    QCoreApplication,
    QDir,
    QRectF,
    QPoint,
    QPointF,
    pyqtSignal,
    QUrl,
    QFileInfo)
from qgis.PyQt.QtWidgets import (QMessageBox,
                                 QFileDialog)
from qgis.core import (Qgis,
                       QgsApplication,
                       QgsProcessing,
                       QgsProject,
                       QgsProcessingModelParameter,
                       QgsSettings,
                       QgsProcessingContext,
                       QgsFileUtils
                       )
from qgis.gui import (QgsProcessingParameterDefinitionDialog,
                      QgsProcessingParameterWidgetContext,
                      QgsModelGraphicsScene,
                      QgsModelDesignerDialog,
                      QgsProcessingContextGenerator,
                      QgsProcessingParametersGenerator)
from qgis.utils import iface

from processing.gui.AlgorithmDialog import AlgorithmDialog
from processing.modeler.ModelerParameterDefinitionDialog import ModelerParameterDefinitionDialog
from processing.modeler.ModelerParametersDialog import ModelerParametersDialog
from processing.modeler.ModelerScene import ModelerScene
from processing.modeler.ModelerUtils import ModelerUtils
from processing.modeler.ProjectProvider import PROJECT_PROVIDER_ID
from processing.script.ScriptEditorDialog import ScriptEditorDialog
from processing.tools.dataobjects import createContext

pluginPath = os.path.split(os.path.dirname(__file__))[0]


class ModelerDialog(QgsModelDesignerDialog):
    CANVAS_SIZE = 4000

    update_model = pyqtSignal()

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

        if iface is not None:
            self.toolbar().setIconSize(iface.iconSize())
            self.setStyleSheet(iface.mainWindow().styleSheet())

        scene = ModelerScene(self)
        scene.setSceneRect(QRectF(0, 0, self.CANVAS_SIZE, self.CANVAS_SIZE))
        self.setModelScene(scene)

        self.view().ensureVisible(0, 0, 10, 10)
        self.view().scale(QgsApplication.desktop().logicalDpiX() / 96, QgsApplication.desktop().logicalDpiX() / 96)

        self.actionOpen().triggered.connect(self.openModel)
        self.actionSaveInProject().triggered.connect(self.saveInProject)
        self.actionRun().triggered.connect(self.runModel)

        if model is not None:
            _model = model.create()
            _model.setSourceFilePath(model.sourceFilePath())
            self.setModel(_model)

        self.view().centerOn(0, 0)

        self.processing_context = createContext()

        class ContextGenerator(QgsProcessingContextGenerator):

            def __init__(self, context):
                super().__init__()
                self.processing_context = context

            def processingContext(self):
                return self.processing_context

        self.context_generator = ContextGenerator(self.processing_context)

    def runModel(self):
        valid, errors = self.model().validate()
        if not valid:
            message_box = QMessageBox()
            message_box.setWindowTitle(self.tr('Model is Invalid'))
            message_box.setIcon(QMessageBox.Warning)
            message_box.setText(self.tr('This model is not valid and contains one or more issues. Are you sure you want to run it in this state?'))
            message_box.setStandardButtons(QMessageBox.Yes | QMessageBox.Cancel)
            message_box.setDefaultButton(QMessageBox.Cancel)

            error_string = ''
            for e in errors:
                e = re.sub(r'<[^>]*>', '', e)
                error_string += '• {}\n'.format(e)

            message_box.setDetailedText(error_string)
            if message_box.exec_() == QMessageBox.Cancel:
                return

        def on_finished(successful, results):
            self.setLastRunChildAlgorithmResults(dlg.results().get('CHILD_RESULTS', {}))
            self.setLastRunChildAlgorithmInputs(dlg.results().get('CHILD_INPUTS', {}))

        dlg = AlgorithmDialog(self.model().create(), parent=self)
        dlg.setLogLevel(QgsProcessingContext.Verbose)
        dlg.setParameters(self.model().designerParameterValues())
        dlg.algorithmFinished.connect(on_finished)
        dlg.exec_()

        if dlg.wasExecuted():
            self.model().setDesignerParameterValues(dlg.createProcessingParameters(flags=QgsProcessingParametersGenerator.Flags(QgsProcessingParametersGenerator.Flag.SkipDefaultValueParameters)))

    def saveInProject(self):
        if not self.validateSave(QgsModelDesignerDialog.SaveAction.SaveInProject):
            return

        self.model().setSourceFilePath(None)

        project_provider = QgsApplication.processingRegistry().providerById(PROJECT_PROVIDER_ID)
        project_provider.add_model(self.model())

        self.update_model.emit()
        self.messageBar().pushMessage("", self.tr("Model was saved inside current project"), level=Qgis.Success,
                                      duration=5)

        self.setDirty(False)
        QgsProject.instance().setDirty(True)

    def saveModel(self, saveAs) -> bool:
        if not self.validateSave(QgsModelDesignerDialog.SaveAction.SaveAsFile):
            return False

        model_name_matched_file_name = self.model().modelNameMatchesFilePath()
        if self.model().sourceFilePath() and not saveAs:
            filename = self.model().sourceFilePath()
        else:
            if self.model().sourceFilePath():
                initial_path = Path(self.model().sourceFilePath())
            elif self.model().name():
                initial_path = Path(ModelerUtils.modelsFolders()[0]) / (self.model().name() + '.model3')
            else:
                initial_path = Path(ModelerUtils.modelsFolders()[0])

            filename, _ = QFileDialog.getSaveFileName(self,
                                                      self.tr('Save Model'),
                                                      initial_path.as_posix(),
                                                      self.tr('Processing models (*.model3 *.MODEL3)'))
            if not filename:
                return False

            filename = QgsFileUtils.ensureFileNameHasExtension(filename, ['model3'])
            self.model().setSourceFilePath(filename)

            if not self.model().name() or self.model().name() == self.tr('model'):
                self.setModelName(Path(filename).stem)
            elif saveAs and model_name_matched_file_name:
                # if saving as, and the model name used to match the filename, then automatically update the
                # model name to match the new file name
                self.setModelName(Path(filename).stem)

        if not self.model().toFile(filename):
            if saveAs:
                QMessageBox.warning(self, self.tr('I/O error'),
                                    self.tr('Unable to save edits. Reason:\n {0}').format(str(sys.exc_info()[1])))
            else:
                QMessageBox.warning(self, self.tr("Can't save model"),
                                    self.tr(
                                        "This model can't be saved in its original location (probably you do not "
                                        "have permission to do it). Please, use the 'Save as…' option."))
            return False

        self.update_model.emit()
        if saveAs:
            self.messageBar().pushMessage("", self.tr("Model was saved to <a href=\"{}\">{}</a>").format(
                QUrl.fromLocalFile(filename).toString(), QDir.toNativeSeparators(filename)), level=Qgis.Success,
                duration=5)

        self.setDirty(False)
        return True

    def openModel(self):
        if not self.checkForUnsavedChanges():
            return

        settings = QgsSettings()
        last_dir = settings.value('Processing/lastModelsDir', QDir.homePath())
        filename, selected_filter = QFileDialog.getOpenFileName(self,
                                                                self.tr('Open Model'),
                                                                last_dir,
                                                                self.tr('Processing models (*.model3 *.MODEL3)'))
        if filename:
            settings.setValue('Processing/lastModelsDir',
                              QFileInfo(filename).absoluteDir().absolutePath())
            self.loadModel(filename)

    def repaintModel(self, showControls=True):
        scene = ModelerScene(self)
        scene.setSceneRect(QRectF(0, 0, self.CANVAS_SIZE,
                                  self.CANVAS_SIZE))

        if not showControls:
            scene.setFlag(QgsModelGraphicsScene.FlagHideControls)

        showComments = QgsSettings().value("/Processing/Modeler/ShowComments", True, bool)
        if not showComments:
            scene.setFlag(QgsModelGraphicsScene.FlagHideComments)

        context = createContext()
        self.setModelScene(scene)
        # create items later that setModelScene to setup link to messageBar to the scene
        scene.createItems(self.model(), context)

    def create_widget_context(self):
        """
        Returns a new widget context for use in the model editor
        """
        widget_context = QgsProcessingParameterWidgetContext()
        widget_context.setProject(QgsProject.instance())
        if iface is not None:
            widget_context.setMapCanvas(iface.mapCanvas())
            widget_context.setActiveLayer(iface.activeLayer())
        widget_context.setModel(self.model())
        return widget_context

    def autogenerate_parameter_name(self, parameter):
        """
        Automatically generates and sets a new parameter's name, based on the parameter's
        description and ensuring that it is unique for the model.
        """
        validChars = \
            'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'
        safeName = ''.join(c for c in parameter.description() if c in validChars)
        name = safeName.lower()
        i = 2
        while self.model().parameterDefinition(name):
            name = safeName.lower() + str(i)
            i += 1
        parameter.setName(name)

    def addInput(self, paramType, pos=None):
        if paramType not in [param.id() for param in QgsApplication.instance().processingRegistry().parameterTypes()]:
            return

        new_param = None
        comment = None
        if ModelerParameterDefinitionDialog.use_legacy_dialog(paramType=paramType):
            dlg = ModelerParameterDefinitionDialog(self.model(), paramType)
            if dlg.exec_():
                new_param = dlg.param
                comment = dlg.comments()
        else:
            # yay, use new API!
            context = createContext()
            widget_context = self.create_widget_context()
            dlg = QgsProcessingParameterDefinitionDialog(type=paramType,
                                                         context=context,
                                                         widgetContext=widget_context,
                                                         algorithm=self.model())
            dlg.registerProcessingContextGenerator(self.context_generator)
            if dlg.exec_():
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
            component.comment().setPosition(component.position() + QPointF(
                component.size().width(),
                -1.5 * component.size().height()))

            self.beginUndoCommand(self.tr('Add Model Input'))
            self.model().addModelParameter(new_param, component)
            self.repaintModel()
            # self.view().ensureVisible(self.scene.getLastParameterItem())
            self.endUndoCommand()

    def getPositionForParameterItem(self):
        MARGIN = 20
        BOX_WIDTH = 200
        BOX_HEIGHT = 80
        if len(self.model().parameterComponents()) > 0:
            maxX = max([i.position().x() for i in list(self.model().parameterComponents().values())])
            newX = min(MARGIN + BOX_WIDTH + maxX, self.CANVAS_SIZE - BOX_WIDTH)
        else:
            newX = MARGIN + BOX_WIDTH / 2
        return QPointF(newX, MARGIN + BOX_HEIGHT / 2)

    def addAlgorithm(self, alg_id, pos=None):
        alg = QgsApplication.processingRegistry().createAlgorithmById(alg_id)
        if not alg:
            return

        dlg = ModelerParametersDialog(alg, self.model())
        if dlg.exec_():
            alg = dlg.createAlgorithm()
            if pos is None or not pos:
                alg.setPosition(self.getPositionForAlgorithmItem())
            else:
                alg.setPosition(pos)

            alg.comment().setPosition(alg.position() + QPointF(
                alg.size().width(),
                -1.5 * alg.size().height()))

            output_offset_x = alg.size().width()
            output_offset_y = 1.5 * alg.size().height()
            for out in alg.modelOutputs():
                alg.modelOutput(out).setPosition(alg.position() + QPointF(output_offset_x, output_offset_y))
                output_offset_y += 1.5 * alg.modelOutput(out).size().height()

            self.beginUndoCommand(self.tr('Add Algorithm'))
            id = self.model().addChildAlgorithm(alg)
            self.repaintModel()
            self.endUndoCommand()

            res, errors = self.model().validateChildAlgorithm(id)
            if not res:
                self.view().scene().showWarning(
                    QCoreApplication.translate('ModelerDialog', 'Algorithm “{}” is invalid').format(alg.description()),
                    self.tr('Algorithm is Invalid'),
                    QCoreApplication.translate('ModelerDialog', "<p>The “{}” algorithm is invalid, because:</p><ul><li>{}</li></ul>").format(alg.description(), '</li><li>'.join(errors)),
                    level=Qgis.Warning
                )
            else:
                self.view().scene().messageBar().clearWidgets()

    def getPositionForAlgorithmItem(self):
        MARGIN = 20
        BOX_WIDTH = 200
        BOX_HEIGHT = 80
        if self.model().childAlgorithms():
            maxX = max([alg.position().x() for alg in list(self.model().childAlgorithms().values())])
            maxY = max([alg.position().y() for alg in list(self.model().childAlgorithms().values())])
            newX = min(MARGIN + BOX_WIDTH + maxX, self.CANVAS_SIZE - BOX_WIDTH)
            newY = min(MARGIN + BOX_HEIGHT + maxY, self.CANVAS_SIZE
                       - BOX_HEIGHT)
        else:
            newX = MARGIN + BOX_WIDTH / 2
            newY = MARGIN * 2 + BOX_HEIGHT + BOX_HEIGHT / 2
        return QPointF(newX, newY)

    def exportAsScriptAlgorithm(self):
        dlg = ScriptEditorDialog(None)

        dlg.editor.setText('\n'.join(self.model().asPythonCode(QgsProcessing.PythonQgsProcessingAlgorithmSubclass, 4)))
        dlg.show()
