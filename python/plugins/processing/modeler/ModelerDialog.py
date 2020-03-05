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

import sys
import os

from qgis.PyQt.QtCore import (
    Qt,
    QCoreApplication,
    QDir,
    QRectF,
    QMimeData,
    QPoint,
    QPointF,
    pyqtSignal,
    QUrl)
from qgis.PyQt.QtWidgets import (QTreeWidget,
                                 QMessageBox,
                                 QFileDialog,
                                 QTreeWidgetItem,
                                 QSizePolicy,
                                 QMainWindow,
                                 QLabel,
                                 QDockWidget,
                                 QWidget,
                                 QVBoxLayout,
                                 QGridLayout,
                                 QFrame,
                                 QLineEdit,
                                 QToolButton,
                                 QAction)
from qgis.PyQt.QtGui import QIcon
from qgis.core import (Qgis,
                       QgsApplication,
                       QgsProcessing,
                       QgsProject,
                       QgsSettings,
                       QgsMessageLog,
                       QgsProcessingModelAlgorithm,
                       QgsProcessingModelParameter,
                       QgsProcessingParameterType,
                       QgsExpressionContextScope,
                       QgsExpressionContext
                       )
from qgis.gui import (QgsDockWidget,
                      QgsScrollArea,
                      QgsFilterLineEdit,
                      QgsProcessingToolboxTreeView,
                      QgsProcessingToolboxProxyModel,
                      QgsProcessingParameterDefinitionDialog,
                      QgsVariableEditorWidget,
                      QgsProcessingParameterWidgetContext,
                      QgsModelGraphicsScene,
                      QgsModelDesignerDialog)
from processing.gui.HelpEditionDialog import HelpEditionDialog
from processing.gui.AlgorithmDialog import AlgorithmDialog
from processing.modeler.ModelerParameterDefinitionDialog import ModelerParameterDefinitionDialog
from processing.modeler.ModelerParametersDialog import ModelerParametersDialog
from processing.modeler.ModelerUtils import ModelerUtils
from processing.modeler.ModelerScene import ModelerScene
from processing.modeler.ProjectProvider import PROJECT_PROVIDER_ID
from processing.script.ScriptEditorDialog import ScriptEditorDialog
from processing.core.ProcessingConfig import ProcessingConfig
from processing.tools.dataobjects import createContext
from qgis.utils import iface

pluginPath = os.path.split(os.path.dirname(__file__))[0]


class ModelerToolboxModel(QgsProcessingToolboxProxyModel):

    def __init__(self, parent=None, registry=None, recentLog=None):
        super().__init__(parent, registry, recentLog)

    def flags(self, index):
        f = super().flags(index)
        source_index = self.mapToSource(index)
        if self.toolboxModel().isAlgorithm(source_index):
            f = f | Qt.ItemIsDragEnabled
        return f

    def supportedDragActions(self):
        return Qt.CopyAction


class ModelerDialog(QgsModelDesignerDialog):
    ALG_ITEM = 'ALG_ITEM'
    PROVIDER_ITEM = 'PROVIDER_ITEM'
    GROUP_ITEM = 'GROUP_ITEM'

    NAME_ROLE = Qt.UserRole
    TAG_ROLE = Qt.UserRole + 1
    TYPE_ROLE = Qt.UserRole + 2

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
        self._variables_scope = None
        self._model = None

        # LOTS of bug reports when we include the dock creation in the UI file
        # see e.g. #16428, #19068
        # So just roll it all by hand......!
        self.propertiesDock = QgsDockWidget(self)
        self.propertiesDock.setFeatures(
            QDockWidget.DockWidgetFloatable | QDockWidget.DockWidgetMovable)
        self.propertiesDock.setObjectName("propertiesDock")
        propertiesDockContents = QWidget()
        self.verticalDockLayout_1 = QVBoxLayout(propertiesDockContents)
        self.verticalDockLayout_1.setContentsMargins(0, 0, 0, 0)
        self.verticalDockLayout_1.setSpacing(0)
        self.scrollArea_1 = QgsScrollArea(propertiesDockContents)
        sizePolicy = QSizePolicy(QSizePolicy.MinimumExpanding,
                                 QSizePolicy.MinimumExpanding)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.scrollArea_1.sizePolicy().hasHeightForWidth())
        self.scrollArea_1.setSizePolicy(sizePolicy)
        self.scrollArea_1.setFocusPolicy(Qt.WheelFocus)
        self.scrollArea_1.setFrameShape(QFrame.NoFrame)
        self.scrollArea_1.setFrameShadow(QFrame.Plain)
        self.scrollArea_1.setWidgetResizable(True)
        self.scrollAreaWidgetContents_1 = QWidget()
        self.gridLayout = QGridLayout(self.scrollAreaWidgetContents_1)
        self.gridLayout.setContentsMargins(6, 6, 6, 6)
        self.gridLayout.setSpacing(4)
        self.label_1 = QLabel(self.scrollAreaWidgetContents_1)
        self.gridLayout.addWidget(self.label_1, 0, 0, 1, 1)
        self.textName = QLineEdit(self.scrollAreaWidgetContents_1)
        self.gridLayout.addWidget(self.textName, 0, 1, 1, 1)
        self.label_2 = QLabel(self.scrollAreaWidgetContents_1)
        self.gridLayout.addWidget(self.label_2, 1, 0, 1, 1)
        self.textGroup = QLineEdit(self.scrollAreaWidgetContents_1)
        self.gridLayout.addWidget(self.textGroup, 1, 1, 1, 1)
        self.label_1.setText(self.tr("Name"))
        self.textName.setToolTip(self.tr("Enter model name here"))
        self.label_2.setText(self.tr("Group"))
        self.textGroup.setToolTip(self.tr("Enter group name here"))
        self.scrollArea_1.setWidget(self.scrollAreaWidgetContents_1)
        self.verticalDockLayout_1.addWidget(self.scrollArea_1)
        self.propertiesDock.setWidget(propertiesDockContents)
        self.propertiesDock.setWindowTitle(self.tr("Model Properties"))

        self.inputsDock = QgsDockWidget(self)
        self.inputsDock.setFeatures(QDockWidget.DockWidgetFloatable | QDockWidget.DockWidgetMovable)
        self.inputsDock.setObjectName("inputsDock")
        self.inputsDockContents = QWidget()
        self.verticalLayout_3 = QVBoxLayout(self.inputsDockContents)
        self.verticalLayout_3.setContentsMargins(0, 0, 0, 0)
        self.scrollArea_2 = QgsScrollArea(self.inputsDockContents)
        sizePolicy.setHeightForWidth(self.scrollArea_2.sizePolicy().hasHeightForWidth())
        self.scrollArea_2.setSizePolicy(sizePolicy)
        self.scrollArea_2.setFocusPolicy(Qt.WheelFocus)
        self.scrollArea_2.setFrameShape(QFrame.NoFrame)
        self.scrollArea_2.setFrameShadow(QFrame.Plain)
        self.scrollArea_2.setWidgetResizable(True)
        self.scrollAreaWidgetContents_2 = QWidget()
        self.verticalLayout = QVBoxLayout(self.scrollAreaWidgetContents_2)
        self.verticalLayout.setContentsMargins(0, 0, 0, 0)
        self.verticalLayout.setSpacing(0)
        self.inputsTree = QTreeWidget(self.scrollAreaWidgetContents_2)
        self.inputsTree.setAlternatingRowColors(True)
        self.inputsTree.header().setVisible(False)
        self.verticalLayout.addWidget(self.inputsTree)
        self.scrollArea_2.setWidget(self.scrollAreaWidgetContents_2)
        self.verticalLayout_3.addWidget(self.scrollArea_2)
        self.inputsDock.setWidget(self.inputsDockContents)
        self.addDockWidget(Qt.DockWidgetArea(1), self.inputsDock)
        self.inputsDock.setWindowTitle(self.tr("Inputs"))

        self.algorithmsDock = QgsDockWidget(self)
        self.algorithmsDock.setFeatures(QDockWidget.DockWidgetFloatable | QDockWidget.DockWidgetMovable)
        self.algorithmsDock.setObjectName("algorithmsDock")
        self.algorithmsDockContents = QWidget()
        self.verticalLayout_4 = QVBoxLayout(self.algorithmsDockContents)
        self.verticalLayout_4.setContentsMargins(0, 0, 0, 0)
        self.scrollArea_3 = QgsScrollArea(self.algorithmsDockContents)
        sizePolicy.setHeightForWidth(self.scrollArea_3.sizePolicy().hasHeightForWidth())
        self.scrollArea_3.setSizePolicy(sizePolicy)
        self.scrollArea_3.setFocusPolicy(Qt.WheelFocus)
        self.scrollArea_3.setFrameShape(QFrame.NoFrame)
        self.scrollArea_3.setFrameShadow(QFrame.Plain)
        self.scrollArea_3.setWidgetResizable(True)
        self.scrollAreaWidgetContents_3 = QWidget()
        self.verticalLayout_2 = QVBoxLayout(self.scrollAreaWidgetContents_3)
        self.verticalLayout_2.setContentsMargins(0, 0, 0, 0)
        self.verticalLayout_2.setSpacing(4)
        self.searchBox = QgsFilterLineEdit(self.scrollAreaWidgetContents_3)
        self.verticalLayout_2.addWidget(self.searchBox)
        self.algorithmTree = QgsProcessingToolboxTreeView(None,
                                                          QgsApplication.processingRegistry())
        self.algorithmTree.setAlternatingRowColors(True)
        self.algorithmTree.header().setVisible(False)
        self.verticalLayout_2.addWidget(self.algorithmTree)
        self.scrollArea_3.setWidget(self.scrollAreaWidgetContents_3)
        self.verticalLayout_4.addWidget(self.scrollArea_3)
        self.algorithmsDock.setWidget(self.algorithmsDockContents)
        self.addDockWidget(Qt.DockWidgetArea(1), self.algorithmsDock)
        self.algorithmsDock.setWindowTitle(self.tr("Algorithms"))
        self.searchBox.setToolTip(self.tr("Enter algorithm name to filter list"))
        self.searchBox.setShowSearchIcon(True)

        self.variables_dock = QgsDockWidget(self)
        self.variables_dock.setFeatures(QDockWidget.DockWidgetFloatable | QDockWidget.DockWidgetMovable)
        self.variables_dock.setObjectName("variablesDock")
        self.variables_dock_contents = QWidget()
        vl_v = QVBoxLayout()
        vl_v.setContentsMargins(0, 0, 0, 0)
        self.variables_editor = QgsVariableEditorWidget()
        vl_v.addWidget(self.variables_editor)
        self.variables_dock_contents.setLayout(vl_v)
        self.variables_dock.setWidget(self.variables_dock_contents)
        self.addDockWidget(Qt.DockWidgetArea(1), self.variables_dock)
        self.variables_dock.setWindowTitle(self.tr("Variables"))
        self.addDockWidget(Qt.DockWidgetArea(1), self.propertiesDock)
        self.tabifyDockWidget(self.propertiesDock, self.variables_dock)
        self.variables_editor.scopeChanged.connect(self.variables_changed)

        try:
            self.setDockOptions(self.dockOptions() | QMainWindow.GroupedDragging)
        except:
            pass

        if iface is not None:
            self.toolbar().setIconSize(iface.iconSize())
            self.setStyleSheet(iface.mainWindow().styleSheet())

        self.toolbutton_export_to_script = QToolButton()
        self.toolbutton_export_to_script.setPopupMode(QToolButton.InstantPopup)
        self.export_to_script_algorithm_action = QAction(
            QCoreApplication.translate('ModelerDialog', 'Export as Script Algorithm…'))
        self.toolbutton_export_to_script.addActions([self.export_to_script_algorithm_action])
        self.toolbutton_export_to_script.setDefaultAction(self.export_to_script_algorithm_action)
        self.toolbar().insertWidget(self.actionExportImage(), self.toolbutton_export_to_script)
        self.export_to_script_algorithm_action.triggered.connect(self.export_as_script_algorithm)

        self.addDockWidget(Qt.LeftDockWidgetArea, self.propertiesDock)
        self.addDockWidget(Qt.LeftDockWidgetArea, self.inputsDock)
        self.addDockWidget(Qt.LeftDockWidgetArea, self.algorithmsDock)
        self.tabifyDockWidget(self.inputsDock, self.algorithmsDock)
        self.inputsDock.raise_()

        self.scene = ModelerScene(self)
        self.scene.setSceneRect(QRectF(0, 0, self.CANVAS_SIZE, self.CANVAS_SIZE))
        self.scene.rebuildRequired.connect(self.repaintModel)
        self.scene.componentChanged.connect(self.componentChanged)

        self.view().setScene(self.scene)
        self.view().ensureVisible(0, 0, 10, 10)
        self.view().scale(QgsApplication.desktop().logicalDpiX() / 96, QgsApplication.desktop().logicalDpiX() / 96)

        def _mimeDataInput(items):
            mimeData = QMimeData()
            text = items[0].data(0, Qt.UserRole)
            mimeData.setText(text)
            return mimeData

        self.inputsTree.mimeData = _mimeDataInput

        self.inputsTree.setDragDropMode(QTreeWidget.DragOnly)
        self.inputsTree.setDropIndicatorShown(True)

        self.algorithms_model = ModelerToolboxModel(self, QgsApplication.processingRegistry())
        self.algorithmTree.setToolboxProxyModel(self.algorithms_model)
        self.algorithmTree.setDragDropMode(QTreeWidget.DragOnly)
        self.algorithmTree.setDropIndicatorShown(True)

        filters = QgsProcessingToolboxProxyModel.Filters(QgsProcessingToolboxProxyModel.FilterModeler)
        if ProcessingConfig.getSetting(ProcessingConfig.SHOW_ALGORITHMS_KNOWN_ISSUES):
            filters |= QgsProcessingToolboxProxyModel.FilterShowKnownIssues
        self.algorithmTree.setFilters(filters)

        if hasattr(self.searchBox, 'setPlaceholderText'):
            self.searchBox.setPlaceholderText(QCoreApplication.translate('ModelerDialog', 'Search…'))
        if hasattr(self.textName, 'setPlaceholderText'):
            self.textName.setPlaceholderText(self.tr('Enter model name here'))
        if hasattr(self.textGroup, 'setPlaceholderText'):
            self.textGroup.setPlaceholderText(self.tr('Enter group name here'))

        # Connect signals and slots
        self.inputsTree.doubleClicked.connect(self._addInput)
        self.searchBox.textChanged.connect(self.algorithmTree.setFilterString)
        self.algorithmTree.doubleClicked.connect(self._addAlgorithm)

        self.actionOpen().triggered.connect(self.openModel)
        self.actionSave().triggered.connect(self.save)
        self.actionSaveAs().triggered.connect(self.saveAs)
        self.actionSaveInProject().triggered.connect(self.saveInProject)
        self.actionEditHelp().triggered.connect(self.editHelp)
        self.actionRun().triggered.connect(self.runModel)

        #        self.mActionShowComments.toggled.connect(self.showComments)
        # self.mActionShowComments.setChecked(settings.value("/Processing/stateModeler", self.saveState())))

        if model is not None:
            self._model = model.create()
            self._model.setSourceFilePath(model.sourceFilePath())
            self.textGroup.setText(self._model.group())
            self.textName.setText(self._model.displayName())
            self.repaintModel()

        else:
            self._model = QgsProcessingModelAlgorithm()
            self._model.setProvider(QgsApplication.processingRegistry().providerById('model'))
        self.update_variables_gui()

        self.fillInputsTree()

        self.view().centerOn(0, 0)
        self.help = None

        self.hasChanged = False

    def closeEvent(self, evt):
        if self.hasChanged:
            ret = QMessageBox.question(
                self, self.tr('Save Model?'),
                self.tr('There are unsaved changes in this model. Do you want to keep those?'),
                QMessageBox.Save | QMessageBox.Cancel | QMessageBox.Discard, QMessageBox.Cancel)

            if ret == QMessageBox.Save:
                self.saveModel(False)
                evt.accept()
            elif ret == QMessageBox.Discard:
                evt.accept()
            else:
                evt.ignore()
        else:
            evt.accept()

    def model(self):
        return self._model

    def editHelp(self):
        alg = self.model()
        dlg = HelpEditionDialog(alg)
        dlg.exec_()
        if dlg.descriptions:
            self.model().setHelpContent(dlg.descriptions)
            self.hasChanged = True

    def update_variables_gui(self):
        variables_scope = QgsExpressionContextScope(self.tr('Model Variables'))
        for k, v in self.model().variables().items():
            variables_scope.setVariable(k, v)
        variables_context = QgsExpressionContext()
        variables_context.appendScope(variables_scope)
        self.variables_editor.setContext(variables_context)
        self.variables_editor.setEditableScopeIndex(0)

    def variables_changed(self):
        self.model().setVariables(self.variables_editor.variablesInActiveScope())

    def runModel(self):
        if len(self.model().childAlgorithms()) == 0:
            self.messageBar().pushMessage("", self.tr(
                "Model doesn't contain any algorithm and/or parameter and can't be executed"), level=Qgis.Warning,
                duration=5)
            return

        dlg = AlgorithmDialog(self.model().create(), parent=self)
        dlg.setParameters(self.model().designerParameterValues())
        dlg.exec_()

        if dlg.wasExecuted():
            self.model().setDesignerParameterValues(dlg.getParameterValues())

    def save(self):
        self.saveModel(False)

    def saveAs(self):
        self.saveModel(True)

    def saveInProject(self):
        if not self.can_save():
            return

        self.model().setName(str(self.textName.text()))
        self.model().setGroup(str(self.textGroup.text()))
        self.model().setSourceFilePath(None)

        project_provider = QgsApplication.processingRegistry().providerById(PROJECT_PROVIDER_ID)
        project_provider.add_model(self.model())

        self.update_model.emit()
        self.messageBar().pushMessage("", self.tr("Model was saved inside current project"), level=Qgis.Success,
                                      duration=5)

        self.hasChanged = False
        QgsProject.instance().setDirty(True)

    def can_save(self):
        """
        Tests whether a model can be saved, or if it is not yet valid
        :return: bool
        """
        if str(self.textName.text()).strip() == '':
            self.messageBar().pushWarning(
                "", self.tr('Please a enter model name before saving')
            )
            return False

        return True

    def saveModel(self, saveAs):
        if not self.can_save():
            return
        self.model().setName(str(self.textName.text()))
        self.model().setGroup(str(self.textGroup.text()))
        if self.model().sourceFilePath() and not saveAs:
            filename = self.model().sourceFilePath()
        else:
            filename, filter = QFileDialog.getSaveFileName(self,
                                                           self.tr('Save Model'),
                                                           ModelerUtils.modelsFolders()[0],
                                                           self.tr('Processing models (*.model3 *.MODEL3)'))
            if filename:
                if not filename.endswith('.model3'):
                    filename += '.model3'
                self.model().setSourceFilePath(filename)
        if filename:
            if not self.model().toFile(filename):
                if saveAs:
                    QMessageBox.warning(self, self.tr('I/O error'),
                                        self.tr('Unable to save edits. Reason:\n {0}').format(str(sys.exc_info()[1])))
                else:
                    QMessageBox.warning(self, self.tr("Can't save model"),
                                        QCoreApplication.translate('QgsPluginInstallerInstallingDialog', (
                                            "This model can't be saved in its original location (probably you do not "
                                            "have permission to do it). Please, use the 'Save as…' option."))
                                        )
                return
            self.update_model.emit()
            if saveAs:
                self.messageBar().pushMessage("", self.tr("Model was correctly saved to <a href=\"{}\">{}</a>").format(
                    QUrl.fromLocalFile(filename).toString(), QDir.toNativeSeparators(filename)), level=Qgis.Success,
                    duration=5)
            else:
                self.messageBar().pushMessage("", self.tr("Model was correctly saved"), level=Qgis.Success, duration=5)

            self.hasChanged = False

    def openModel(self):
        filename, selected_filter = QFileDialog.getOpenFileName(self,
                                                                self.tr('Open Model'),
                                                                ModelerUtils.modelsFolders()[0],
                                                                self.tr('Processing models (*.model3 *.MODEL3)'))
        if filename:
            self.loadModel(filename)

    def loadModel(self, filename):
        alg = QgsProcessingModelAlgorithm()
        if alg.fromFile(filename):
            self._model = alg
            self._model.setProvider(QgsApplication.processingRegistry().providerById('model'))
            self.textGroup.setText(alg.group())
            self.textName.setText(alg.name())
            self.repaintModel()

            self.update_variables_gui()

            self.view().centerOn(0, 0)
            self.hasChanged = False
        else:
            QgsMessageLog.logMessage(self.tr('Could not load model {0}').format(filename),
                                     self.tr('Processing'),
                                     Qgis.Critical)
            QMessageBox.critical(self, self.tr('Open Model'),
                                 self.tr('The selected model could not be loaded.\n'
                                         'See the log for more information.'))

    def repaintModel(self, showControls=True):
        self.scene = ModelerScene(self)
        self.scene.setSceneRect(QRectF(0, 0, self.CANVAS_SIZE,
                                       self.CANVAS_SIZE))

        if not showControls:
            self.scene.setFlag(QgsModelGraphicsScene.FlagHideControls)

        context = createContext()
        self.scene.createItems(self.model(), context)
        self.view().setScene(self.scene)

        self.scene.rebuildRequired.connect(self.repaintModel)
        self.scene.componentChanged.connect(self.componentChanged)

    def componentChanged(self):
        self.hasChanged = True

    def _addInput(self):
        item = self.inputsTree.currentItem()
        param = item.data(0, Qt.UserRole)
        self.addInput(param)

    def create_widget_context(self):
        """
        Returns a new widget context for use in the model editor
        """
        widget_context = QgsProcessingParameterWidgetContext()
        widget_context.setProject(QgsProject.instance())
        if iface is not None:
            widget_context.setMapCanvas(iface.mapCanvas())
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
        parameter.setName(safeName)

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
            if dlg.exec_():
                new_param = dlg.createParameter()
                self.autogenerate_parameter_name(new_param)
                comment = dlg.comments()

        if new_param is not None:
            if pos is None:
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

            self.model().addModelParameter(new_param, component)
            self.repaintModel()
            # self.view().ensureVisible(self.scene.getLastParameterItem())
            self.hasChanged = True

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

    def fillInputsTree(self):
        icon = QIcon(os.path.join(pluginPath, 'images', 'input.svg'))
        parametersItem = QTreeWidgetItem()
        parametersItem.setText(0, self.tr('Parameters'))
        sortedParams = sorted(QgsApplication.instance().processingRegistry().parameterTypes(), key=lambda pt: pt.name())
        for param in sortedParams:
            if param.flags() & QgsProcessingParameterType.ExposeToModeler:
                paramItem = QTreeWidgetItem()
                paramItem.setText(0, param.name())
                paramItem.setData(0, Qt.UserRole, param.id())
                paramItem.setIcon(0, icon)
                paramItem.setFlags(Qt.ItemIsEnabled | Qt.ItemIsSelectable | Qt.ItemIsDragEnabled)
                paramItem.setToolTip(0, param.description())
                parametersItem.addChild(paramItem)
        self.inputsTree.addTopLevelItem(parametersItem)
        parametersItem.setExpanded(True)

    def _addAlgorithm(self):
        self.addAlgorithm(self.algorithmTree.selectedAlgorithm())

    def addAlgorithm(self, alg_id, pos=None):
        alg = QgsApplication.processingRegistry().createAlgorithmById(alg_id)
        if not alg:
            return

        dlg = ModelerParametersDialog(alg, self.model())
        if dlg.exec_():
            alg = dlg.createAlgorithm()
            if pos is None:
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

            self.model().addChildAlgorithm(alg)
            self.repaintModel()
            self.hasChanged = True

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

    def export_as_script_algorithm(self):
        dlg = ScriptEditorDialog(None)

        dlg.editor.setText('\n'.join(self.model().asPythonCode(QgsProcessing.PythonQgsProcessingAlgorithmSubclass, 4)))
        dlg.show()
