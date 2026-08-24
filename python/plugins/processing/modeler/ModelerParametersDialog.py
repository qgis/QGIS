"""
***************************************************************************
    ModelerParametersDialog.py
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

import webbrowser
from typing import Optional

from qgis.core import (
    Qgis,
    QgsProcessingAlgorithm,
    QgsProcessingModelAlgorithm,
    QgsProcessingModelChildAlgorithm,
    QgsProcessingModelChildParameterSource,
    QgsProcessingModelOutput,
    QgsProcessingParameterDefinition,
    QgsProject,
)
from qgis.gui import (
    QgsCollapsibleGroupBox,
    QgsColorButton,
    QgsGui,
    QgsHelp,
    QgsMessageBar,
    QgsModelChildDependenciesWidget,
    QgsPanelWidget,
    QgsPanelWidgetStack,
    QgsProcessingContextGenerator,
    QgsProcessingModelConfigWidget,
    QgsProcessingParameterWidgetContext,
    QgsScrollArea,
)
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtWidgets import (
    QDialog,
    QDialogButtonBox,
    QFrame,
    QHBoxLayout,
    QLabel,
    QLineEdit,
    QSizePolicy,
    QTabWidget,
    QTextEdit,
    QVBoxLayout,
    QWidget,
)
from qgis.utils import iface

from processing.tools.dataobjects import createContext


class ModelerParametersDialog(QDialog):
    def __init__(
        self,
        alg: QgsProcessingAlgorithm,
        model: QgsProcessingModelAlgorithm,
        algName: Optional[str] = None,
        configuration: Optional[dict[str, object]] = None,
    ):
        super().__init__()
        self.setObjectName("ModelerParametersDialog")

        if iface is not None:
            self.setStyleSheet(iface.mainWindow().styleSheet())

        # dammit this is SUCH as mess... stupid stable API
        self._alg = alg  # The algorithm to define in this dialog. It is an instance of QgsProcessingAlgorithm
        self.model = model  # The model this algorithm is going to be added to. It is an instance of QgsProcessingModelAlgorithm
        self.childId = algName  # The name of the algorithm in the model, in case we are editing it and not defining it for the first time
        self.configuration = configuration
        self.context = createContext()

        self.setWindowTitle(
            " - ".join([self._alg.group(), self._alg.displayName()])
            if self._alg.group()
            else self._alg.displayName()
        )

        self.widget = ModelerParametersWidget(
            alg, model, algName, configuration, context=self.context, dialog=self
        )
        QgsGui.enableAutoGeometryRestore(self)

        self.buttonBox = QDialogButtonBox()
        self.buttonBox.setOrientation(Qt.Orientation.Horizontal)
        self.buttonBox.setStandardButtons(
            QDialogButtonBox.StandardButton.Cancel
            | QDialogButtonBox.StandardButton.Ok
            | QDialogButtonBox.StandardButton.Help
        )

        self.buttonBox.accepted.connect(self.okPressed)
        self.buttonBox.rejected.connect(self.reject)
        self.buttonBox.helpRequested.connect(self.openHelp)

        mainLayout = QVBoxLayout()
        mainLayout.addWidget(self.widget, 1)
        mainLayout.addWidget(self.buttonBox)
        self.setLayout(mainLayout)

    def algorithm(self):
        return self._alg

    def setComments(self, text):
        self.widget.setComments(text)

    def comments(self):
        return self.widget.comments()

    def setCommentColor(self, color):
        self.widget.setCommentColor(color)

    def commentColor(self):
        return self.widget.commentColor()

    def switchToCommentTab(self):
        self.widget.switchToCommentTab()

    def setPreviousValues(self):
        self.widget.setPreviousValues()

    def createAlgorithm(self):
        return self.widget.createAlgorithm()

    def okPressed(self):
        if self.createAlgorithm() is not None:
            self.accept()

    def openHelp(self):
        algHelp = self.widget.algorithm().helpUrl()
        if not algHelp:
            algHelp = QgsHelp.helpUrl(
                "processing_algs/{}/{}.html#{}".format(
                    self.widget.algorithm().provider().helpId(),
                    self.algorithm().groupId(),
                    f"{self.algorithm().provider().helpId()}{self.algorithm().name().replace('_', '-')}",
                )
            ).toString()

        if algHelp not in [None, ""]:
            webbrowser.open(algHelp)


class ModelerParametersPanelWidget(QgsPanelWidget):
    def __init__(
        self, alg, model, algName=None, configuration=None, dialog=None, context=None
    ):
        super().__init__()
        self._alg = alg  # The algorithm to define in this dialog. It is an instance of QgsProcessingAlgorithm
        self.model = model  # The model this algorithm is going to be added to. It is an instance of QgsProcessingModelAlgorithm
        self.childId = algName  # The name of the algorithm in the model, in case we are editing it and not defining it for the first time
        self.configuration = configuration
        self.context = context
        self.dialog = dialog
        self.previous_output_definitions = {}
        self.block_changes_signal = 0

        class ContextGenerator(QgsProcessingContextGenerator):
            def __init__(self, context):
                super().__init__()
                self.processing_context = context

            def processingContext(self):
                return self.processing_context

        self.context_generator = ContextGenerator(self.context)

        self.setupUi()
        self.params = None

    def algorithm(self):
        return self._alg

    def setupUi(self):
        self.showAdvanced = False
        self.wrappers = {}
        self.algorithmItem = None

        self.mainLayout = QVBoxLayout()
        self.mainLayout.setContentsMargins(0, 0, 0, 0)

        self.verticalLayout = QVBoxLayout()

        self.bar = QgsMessageBar()
        self.bar.setSizePolicy(QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Fixed)
        self.verticalLayout.addWidget(self.bar)

        hLayout = QHBoxLayout()
        hLayout.setContentsMargins(0, 0, 0, 0)
        descriptionLabel = QLabel(self.tr("Description"))
        self.descriptionBox = QLineEdit()
        self.descriptionBox.setText(self._alg.displayName())
        hLayout.addWidget(descriptionLabel)
        hLayout.addWidget(self.descriptionBox)
        self.descriptionBox.textChanged.connect(self.emit_changed_signal)

        self.verticalLayout.addLayout(hLayout)
        line = QFrame()
        line.setFrameShape(QFrame.Shape.HLine)
        line.setFrameShadow(QFrame.Shadow.Sunken)
        self.verticalLayout.addWidget(line)

        widget_context = QgsProcessingParameterWidgetContext()
        widget_context.setProject(QgsProject.instance())
        if iface is not None:
            widget_context.setMapCanvas(iface.mapCanvas())
            widget_context.setActiveLayer(iface.activeLayer())

        widget_context.setModel(self.model)
        widget_context.setModelChildAlgorithmId(self.childId)

        self.algorithmItem = (
            QgsGui.instance()
            .processingGuiRegistry()
            .algorithmConfigurationWidget(self._alg)
        )
        if self.algorithmItem:
            self.algorithmItem.setWidgetContext(widget_context)
            self.algorithmItem.registerProcessingContextGenerator(
                self.context_generator
            )
            if self.configuration:
                self.algorithmItem.setConfiguration(self.configuration)
            self.verticalLayout.addWidget(self.algorithmItem)

        self.grpAdvanced = QgsCollapsibleGroupBox(self.tr("Advanced Parameters"))
        self.grpAdvancedVLayout = QVBoxLayout()
        self.grpAdvanced.setLayout(self.grpAdvancedVLayout)
        self.grpAdvanced.hide()

        self.verticalLayout.addWidget(self.grpAdvanced)

        for param in self._alg.parameterDefinitions():
            if param.flags() & QgsProcessingParameterDefinition.Flag.FlagAdvanced:
                self.grpAdvanced.show()
                break
        for param in self._alg.parameterDefinitions():
            if (
                param.isDestination()
                or param.flags() & QgsProcessingParameterDefinition.Flag.FlagHidden
            ):
                continue

            widget = QgsGui.processingGuiRegistry().createModelerParameterWidget(
                self.model, self.childId, param, self.context
            )
            widget.setDialog(self.dialog)
            widget.setWidgetContext(widget_context)
            widget.registerProcessingContextGenerator(self.context_generator)
            widget.changed.connect(self.emit_changed_signal)
            self.wrappers[param.name()] = widget
            label = widget.createLabel()

            if param.flags() & QgsProcessingParameterDefinition.Flag.FlagAdvanced:
                self.grpAdvancedVLayout.addWidget(label)
                self.grpAdvancedVLayout.addWidget(widget)
            else:
                # Regular parameters
                self.verticalLayout.insertWidget(self.verticalLayout.count() - 1, label)
                self.verticalLayout.insertWidget(
                    self.verticalLayout.count() - 1, widget
                )

        for output in self._alg.destinationParameterDefinitions():
            if output.flags() & QgsProcessingParameterDefinition.Flag.FlagHidden:
                continue

            widget = QgsGui.processingGuiRegistry().createModelerParameterWidget(
                self.model, self.childId, output, self.context
            )
            widget.setDialog(self.dialog)
            widget.setWidgetContext(widget_context)
            widget.registerProcessingContextGenerator(self.context_generator)
            widget.changed.connect(self.emit_changed_signal)

            self.wrappers[output.name()] = widget

            label = widget.createLabel()
            if label is not None:
                self.verticalLayout.addWidget(label)

            self.verticalLayout.addWidget(widget)

        label = QLabel(" ")
        self.verticalLayout.addWidget(label)
        label = QLabel(self.tr("Dependencies"))
        self.dependencies_panel = QgsModelChildDependenciesWidget(
            self, self.model, self.childId
        )
        self.verticalLayout.addWidget(label)
        self.verticalLayout.addWidget(self.dependencies_panel)
        self.verticalLayout.addStretch(1000)

        self.setPreviousValues()
        self.verticalLayout2 = QVBoxLayout()
        self.verticalLayout2.setSpacing(2)
        self.verticalLayout2.setMargin(0)

        self.paramPanel = QWidget()
        self.paramPanel.setLayout(self.verticalLayout)
        self.scrollArea = QgsScrollArea()
        self.scrollArea.setWidget(self.paramPanel)
        self.scrollArea.setWidgetResizable(True)
        self.scrollArea.setFrameStyle(QFrame.Shape.NoFrame)

        self.verticalLayout2.addWidget(self.scrollArea)

        w = QWidget()
        w.setLayout(self.verticalLayout2)
        self.mainLayout.addWidget(w)
        self.setLayout(self.mainLayout)

    def emit_changed_signal(self):
        if not self.block_changes_signal:
            self.widgetChanged.emit()

    def setPreviousValues(self):
        self.block_changes_signal += 1
        if self.childId is not None:
            alg = self.model.childAlgorithm(self.childId)

            self.descriptionBox.setText(alg.description())
            for param in alg.algorithm().parameterDefinitions():
                if (
                    param.isDestination()
                    or param.flags() & QgsProcessingParameterDefinition.Flag.FlagHidden
                ):
                    continue
                value = None
                if param.name() in alg.parameterSources():
                    value = alg.parameterSources()[param.name()]
                    if isinstance(value, list) and len(value) == 1:
                        value = value[0]
                    elif isinstance(value, list) and len(value) == 0:
                        value = None

                wrapper = self.wrappers[param.name()]
                if value is None:
                    value = QgsProcessingModelChildParameterSource.fromStaticValue(
                        param.defaultValue()
                    )

                wrapper.setWidgetValue(value)

            for output in self.algorithm().destinationParameterDefinitions():
                if output.flags() & QgsProcessingParameterDefinition.Flag.FlagHidden:
                    continue

                model_output_name = None
                for name, out in alg.modelOutputs().items():
                    if (
                        out.childId() == self.childId
                        and out.childOutputName() == output.name()
                    ):
                        # this destination parameter is linked to a model output
                        model_output_name = out.name()
                        self.previous_output_definitions[output.name()] = out
                        break

                value = None
                if (
                    model_output_name is None
                    and output.name() in alg.parameterSources()
                ):
                    value = alg.parameterSources()[output.name()]
                    if isinstance(value, list) and len(value) == 1:
                        value = value[0]
                    elif isinstance(value, list) and len(value) == 0:
                        value = None

                wrapper = self.wrappers[output.name()]

                if model_output_name is not None:
                    wrapper.setToModelOutput(model_output_name)
                elif value is not None or output.defaultValue() is not None:
                    if value is None:
                        value = QgsProcessingModelChildParameterSource.fromStaticValue(
                            output.defaultValue()
                        )

                    wrapper.setWidgetValue(value)

            self.dependencies_panel.setValue(alg.dependencies())
        self.block_changes_signal -= 1

    def createAlgorithm(self):
        alg = QgsProcessingModelChildAlgorithm(self._alg.id())
        if not self.childId:
            alg.generateChildId(self.model)
        else:
            alg.setChildId(self.childId)
        alg.setDescription(self.descriptionBox.text())
        if self.algorithmItem:
            alg.setConfiguration(self.algorithmItem.configuration())
            self._alg = alg.algorithm().create(self.algorithmItem.configuration())
        for param in self._alg.parameterDefinitions():
            if (
                param.isDestination()
                or param.flags() & QgsProcessingParameterDefinition.Flag.FlagHidden
            ):
                continue

            wrapper = self.wrappers[param.name()]
            val = wrapper.value()

            if isinstance(val, QgsProcessingModelChildParameterSource):
                val = [val]
            elif not (
                isinstance(val, list)
                and all(
                    [
                        isinstance(subval, QgsProcessingModelChildParameterSource)
                        for subval in val
                    ]
                )
            ):
                val = [QgsProcessingModelChildParameterSource.fromStaticValue(val)]

            valid = True
            for subval in val:
                if (
                    isinstance(subval, QgsProcessingModelChildParameterSource)
                    and subval.source()
                    == Qgis.ProcessingModelChildParameterSource.StaticValue
                    and not param.checkValueIsAcceptable(subval.staticValue())
                ) or (
                    subval is None
                    and not param.flags()
                    & QgsProcessingParameterDefinition.Flag.FlagOptional
                ):
                    valid = False
                    break

            if valid:
                alg.addParameterSources(param.name(), val)

        outputs = {}
        for output in self._alg.destinationParameterDefinitions():
            if not output.flags() & QgsProcessingParameterDefinition.Flag.FlagHidden:
                wrapper = self.wrappers[output.name()]

                if wrapper.isModelOutput():
                    name = wrapper.modelOutputName()
                    if name:
                        # if there was a previous output definition already for this output, we start with it,
                        # otherwise we'll lose any existing output comments, coloring, position, etc
                        model_output = self.previous_output_definitions.get(
                            output.name(), QgsProcessingModelOutput(name, name)
                        )
                        model_output.setDescription(name)
                        model_output.setChildId(alg.childId())
                        model_output.setChildOutputName(output.name())
                        outputs[name] = model_output
                else:
                    val = wrapper.value()

                    if isinstance(val, QgsProcessingModelChildParameterSource):
                        val = [val]

                    alg.addParameterSources(output.name(), val)

            if output.flags() & QgsProcessingParameterDefinition.Flag.FlagIsModelOutput:
                if output.name() not in outputs:
                    model_output = QgsProcessingModelOutput(
                        output.name(), output.name()
                    )
                    model_output.setChildId(alg.childId())
                    model_output.setChildOutputName(output.name())
                    outputs[output.name()] = model_output

        alg.setModelOutputs(outputs)
        alg.setDependencies(self.dependencies_panel.value())

        return alg


class ModelerParametersWidget(QgsProcessingModelConfigWidget):
    def __init__(
        self, alg, model, algName=None, configuration=None, dialog=None, context=None
    ):
        super().__init__()
        self._alg = alg  # The algorithm to define in this dialog. It is an instance of QgsProcessingAlgorithm
        self.model = model  # The model this algorithm is going to be added to. It is an instance of QgsProcessingModelAlgorithm
        self.childId = algName  # The name of the algorithm in the model, in case we are editing it and not defining it for the first time
        self.configuration = configuration
        self.context = context
        self.dialog = dialog

        self.widget = ModelerParametersPanelWidget(
            alg, model, algName, configuration, dialog, context
        )
        self.widget.widgetChanged.connect(self.widgetChanged)

        self.setupUi()
        self.params = None

    def algorithm(self):
        return self._alg

    def switchToCommentTab(self):
        self.tab.setCurrentIndex(1)
        self.commentEdit.setFocus()
        self.commentEdit.selectAll()

    def setupUi(self):
        self.mainLayout = QVBoxLayout()
        self.mainLayout.setContentsMargins(0, 0, 0, 0)
        self.tab = QTabWidget()
        self.mainLayout.addWidget(self.tab)

        self.param_widget = QgsPanelWidgetStack()
        self.widget.setDockMode(True)
        self.param_widget.setMainPanel(self.widget)

        self.tab.addTab(self.param_widget, self.tr("Properties"))

        self.commentLayout = QVBoxLayout()
        self.commentEdit = QTextEdit()
        self.commentEdit.setAcceptRichText(False)
        self.commentLayout.addWidget(self.commentEdit, 1)

        hl = QHBoxLayout()
        hl.setContentsMargins(0, 0, 0, 0)
        hl.addWidget(QLabel(self.tr("Color")))
        self.comment_color_button = QgsColorButton()
        self.comment_color_button.setAllowOpacity(True)
        self.comment_color_button.setWindowTitle(self.tr("Comment Color"))
        self.comment_color_button.setShowNull(True, self.tr("Default"))
        hl.addWidget(self.comment_color_button)
        self.commentLayout.addLayout(hl)

        w2 = QWidget()
        w2.setLayout(self.commentLayout)
        self.tab.addTab(w2, self.tr("Comments"))

        self.setLayout(self.mainLayout)

    def setComments(self, text):
        self.commentEdit.setPlainText(text)

    def comments(self):
        return self.commentEdit.toPlainText()

    def setCommentColor(self, color):
        if color.isValid():
            self.comment_color_button.setColor(color)
        else:
            self.comment_color_button.setToNull()

    def commentColor(self):
        return (
            self.comment_color_button.color()
            if not self.comment_color_button.isNull()
            else QColor()
        )

    def setPreviousValues(self):
        self.widget.setPreviousValues()

    def createAlgorithm(self):
        alg = self.widget.createAlgorithm()
        if alg:
            alg.comment().setDescription(self.comments())
            alg.comment().setColor(self.commentColor())
        return alg
