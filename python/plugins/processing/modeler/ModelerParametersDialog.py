# -*- coding: utf-8 -*-

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
from builtins import str


__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import webbrowser

from qgis.PyQt.QtCore import Qt, QUrl, QMetaObject
from qgis.PyQt.QtWidgets import (QDialog, QDialogButtonBox, QLabel, QLineEdit,
                                 QFrame, QPushButton, QSizePolicy, QVBoxLayout,
                                 QHBoxLayout, QWidget)

from qgis.core import (QgsProcessingParameterDefinition,
                       QgsProcessingParameterPoint,
                       QgsProcessingParameterExtent,
                       QgsProcessingModelAlgorithm)

from qgis.gui import (QgsMessageBar,
                      QgsScrollArea)

from processing.gui.wrappers import WidgetWrapperFactory
from processing.gui.wrappers import InvalidParameterValue
from processing.gui.MultipleInputPanel import MultipleInputPanel
from processing.core.outputs import (OutputRaster,
                                     OutputVector,
                                     OutputTable,
                                     OutputHTML,
                                     OutputFile,
                                     OutputDirectory)
from processing.core.parameters import ParameterPoint, ParameterExtent


class ModelerParametersDialog(QDialog):

    ENTER_NAME = '[Enter name if this is a final result]'
    NOT_SELECTED = '[Not selected]'
    USE_MIN_COVERING_EXTENT = '[Use min covering extent]'

    def __init__(self, alg, model, algName=None):
        QDialog.__init__(self)
        self.setModal(True)
        # The algorithm to define in this dialog. It is an instance of GeoAlgorithm
        self._alg = alg
        # The resulting algorithm after the user clicks on OK. it is an instance of the container Algorithm class
        self.alg = None
        # The model this algorithm is going to be added to
        self.model = model
        # The name of the algorithm in the model, in case we are editing it and not defining it for the first time
        self._algName = algName
        self.setupUi()
        self.params = None

    def setupUi(self):
        self.labels = {}
        self.widgets = {}
        self.checkBoxes = {}
        self.showAdvanced = False
        self.wrappers = {}
        self.valueItems = {}
        self.dependentItems = {}
        self.resize(650, 450)
        self.buttonBox = QDialogButtonBox()
        self.buttonBox.setOrientation(Qt.Horizontal)
        self.buttonBox.setStandardButtons(QDialogButtonBox.Cancel | QDialogButtonBox.Ok | QDialogButtonBox.Help)
        self.setSizePolicy(QSizePolicy.Expanding,
                           QSizePolicy.Expanding)
        self.verticalLayout = QVBoxLayout()
        self.verticalLayout.setSpacing(5)
        self.verticalLayout.setMargin(20)

        self.bar = QgsMessageBar()
        self.bar.setSizePolicy(QSizePolicy.Minimum, QSizePolicy.Fixed)
        self.verticalLayout.addWidget(self.bar)

        hLayout = QHBoxLayout()
        hLayout.setSpacing(5)
        hLayout.setMargin(0)
        descriptionLabel = QLabel(self.tr("Description"))
        self.descriptionBox = QLineEdit()
        self.descriptionBox.setText(self._alg.displayName())
        hLayout.addWidget(descriptionLabel)
        hLayout.addWidget(self.descriptionBox)
        self.verticalLayout.addLayout(hLayout)
        line = QFrame()
        line.setFrameShape(QFrame.HLine)
        line.setFrameShadow(QFrame.Sunken)
        self.verticalLayout.addWidget(line)

        for param in self._alg.parameterDefinitions():
            if param.flags() & QgsProcessingParameterDefinition.FlagAdvanced:
                self.advancedButton = QPushButton()
                self.advancedButton.setText(self.tr('Show advanced parameters'))
                self.advancedButton.clicked.connect(
                    self.showAdvancedParametersClicked)
                advancedButtonHLayout = QHBoxLayout()
                advancedButtonHLayout.addWidget(self.advancedButton)
                advancedButtonHLayout.addStretch()
                self.verticalLayout.addLayout(advancedButtonHLayout)
                break
        for param in self._alg.parameterDefinitions():
            if param.isDestination() or param.flags() & QgsProcessingParameterDefinition.FlagHidden:
                continue
            desc = param.description()
            if isinstance(param, QgsProcessingParameterExtent):
                desc += self.tr('(xmin, xmax, ymin, ymax)')
            if isinstance(param, QgsProcessingParameterPoint):
                desc += self.tr('(x, y)')
            if param.flags() & QgsProcessingParameterDefinition.FlagOptional:
                desc += self.tr(' [optional]')
            label = QLabel(desc)
            self.labels[param.name()] = label

            wrapper = WidgetWrapperFactory.create_wrapper(param, self)
            self.wrappers[param.name()] = wrapper

            widget = wrapper.widget
            if widget is not None:
                self.valueItems[param.name()] = widget
                tooltip = param.description()
                label.setToolTip(tooltip)
                widget.setToolTip(tooltip)
                if param.flags() & QgsProcessingParameterDefinition.FlagAdvanced:
                    label.setVisible(self.showAdvanced)
                    widget.setVisible(self.showAdvanced)
                    self.widgets[param.name()] = widget

                self.verticalLayout.addWidget(label)
                self.verticalLayout.addWidget(widget)

        #for output in self._alg.outputs:
        #    if output.flags() & QgsProcessingParameterDefinition.FlagHidden:
        #        continue
        #    if isinstance(output, (OutputRaster, OutputVector, OutputTable,
        #                           OutputHTML, OutputFile, OutputDirectory)):
        #        label = QLabel(output.description() + '<' +
        #                       output.__class__.__name__ + '>')
        #        item = QLineEdit()
        #        if hasattr(item, 'setPlaceholderText'):
        #            item.setPlaceholderText(ModelerParametersDialog.ENTER_NAME)
        #        self.verticalLayout.addWidget(label)
        #        self.verticalLayout.addWidget(item)
        #        self.valueItems[output.name] = item

        label = QLabel(' ')
        self.verticalLayout.addWidget(label)
        label = QLabel(self.tr('Parent algorithms'))
        self.dependenciesPanel = self.getDependenciesPanel()
        self.verticalLayout.addWidget(label)
        self.verticalLayout.addWidget(self.dependenciesPanel)
        self.verticalLayout.addStretch(1000)

        self.setPreviousValues()
        self.setWindowTitle(self._alg.displayName())
        self.verticalLayout2 = QVBoxLayout()
        self.verticalLayout2.setSpacing(2)
        self.verticalLayout2.setMargin(0)

        self.paramPanel = QWidget()
        self.paramPanel.setLayout(self.verticalLayout)
        self.scrollArea = QgsScrollArea()
        self.scrollArea.setWidget(self.paramPanel)
        self.scrollArea.setWidgetResizable(True)

        self.verticalLayout2.addWidget(self.scrollArea)
        self.verticalLayout2.addWidget(self.buttonBox)
        self.setLayout(self.verticalLayout2)
        self.buttonBox.accepted.connect(self.okPressed)
        self.buttonBox.rejected.connect(self.cancelPressed)
        self.buttonBox.helpRequested.connect(self.openHelp)
        QMetaObject.connectSlotsByName(self)

        for wrapper in list(self.wrappers.values()):
            wrapper.postInitialize(list(self.wrappers.values()))

    def getAvailableDependencies(self):  # spellok
        if self._algName is None:
            dependent = []
        else:
            dependent = self.model.getDependentAlgorithms(self._algName)
        opts = []
        for alg in list(self.model.childAlgorithms().values()):
            if alg.childId() not in dependent:
                opts.append(alg)
        return opts

    def getDependenciesPanel(self):
        return MultipleInputPanel([alg.description for alg in self.getAvailableDependencies()])  # spellok

    def showAdvancedParametersClicked(self):
        self.showAdvanced = not self.showAdvanced
        if self.showAdvanced:
            self.advancedButton.setText(self.tr('Hide advanced parameters'))
        else:
            self.advancedButton.setText(self.tr('Show advanced parameters'))
        for param in self._alg.parameterDefinitions():
            if param.flags() & QgsProcessingParameterDefinition.FlagAdvanced:
                self.labels[param.name()].setVisible(self.showAdvanced)
                self.widgets[param.name()].setVisible(self.showAdvanced)

    def getAvailableValuesOfType(self, paramType, outTypes=[], dataType=None):
        # upgrade paramType to list
        if paramType is None:
            paramType = []
        elif type(paramType) is not list:
            paramType = [paramType]
        if outTypes is None:
            outTypes = []
        elif type(outTypes) is not list:
            outTypes = [outTypes]

        values = []
        inputs = self.model.parameterComponents()
        for i in list(inputs.values()):
            param = self.model.parameterDefinition(i.parameterName())
            for t in paramType:
                if isinstance(param, t):
                    if dataType is not None:
                        if param.datatype in dataType:
                            values.append(QgsProcessingModelAlgorithm.ChildParameterSource.fromModelParameter(param.name()))
                    else:
                        values.append(QgsProcessingModelAlgorithm.ChildParameterSource.fromModelParameter(param.name()))
                    break
        if not outTypes:
            return values
        if self._algName is None:
            dependent = []
        else:
            dependent = self.model.getDependentAlgorithms(self._algName)
        for alg in list(self.model.childAlgorithms().values()):
            if alg.childId() not in dependent:
                for out in alg.algorithm().outputDefinitions():
                    for t in outTypes:
                        if isinstance(out, t):
                            if dataType is not None and out.datatype in dataType:
                                values.append(QgsProcessingModelAlgorithm.ChildParameterSource.fromChildOutput(alg.childId(), out.name()))
                            else:
                                values.append(QgsProcessingModelAlgorithm.ChildParameterSource.fromChildOutput(alg.childId(), out.name()))

        return values

    def resolveValueDescription(self, value):
        if isinstance(value, QgsProcessingModelAlgorithm.ChildParameterSource):
            if value.source() == QgsProcessingModelAlgorithm.ChildParameterSource.StaticValue:
                return value.staticValue()
            elif value.source() == QgsProcessingModelAlgorithm.ChildParameterSource.ModelParameter:
                return self.model.parameterDefinition(value.parameterName()).description()
            elif value.source() == QgsProcessingModelAlgorithm.ChildParameterSource.ChildOutput:
                alg = self.model.childAlgorithm(value.outputChildId())
                return self.tr("'{0}' from algorithm '{1}'").format(alg.algorithm().outputDefinition(value.outputName()).description(), alg.description())

        return value

    def setPreviousValues(self):
        if self._algName is not None:
            alg = self.model.childAlgorithm(self._algName)
            self.descriptionBox.setText(alg.description())
            for param in alg.algorithm().parameterDefinitions():
                if param.isDestination() or param.flags() & QgsProcessingParameterDefinition.FlagHidden:
                    continue
                if param.name() in alg.parameterSources():
                    value = alg.parameterSources()[param.name()]
                else:
                    value = param.defaultValue()
                self.wrappers[param.name()].setValue(value)
            for name, out in list(alg.modelOutputs().items()):
                self.valueItems[name].setText(out.description())

            selected = []
            dependencies = self.getAvailableDependencies()  # spellok
            for idx, dependency in enumerate(dependencies):
                if dependency.childId() in alg.dependencies():
                    selected.append(idx)

            self.dependenciesPanel.setSelectedItems(selected)

    def createAlgorithm(self):
        alg = QgsProcessingModelAlgorithm.ChildAlgorithm(self._alg.id())
        alg.generateChildId(self.model)
        alg.setDescription(self.descriptionBox.text())
        for param in self._alg.parameterDefinitions():
            if param.isDestination() or param.flags() & QgsProcessingParameterDefinition.FlagHidden:
                continue
            val = self.wrappers[param.name()].value()
            if ( isinstance(val, QgsProcessingModelAlgorithm.ChildParameterSource) and val.source() == QgsProcessingModelAlgorithm.ChildParameterSource.StaticValue and not param.checkValueIsAcceptable(val.staticValue())) \
                    or (not isinstance(val, QgsProcessingModelAlgorithm.ChildParameterSource) and not param.checkValueIsAcceptable(val)):
                self.bar.pushMessage("Error", "Wrong or missing value for parameter '%s'" % param.description(),
                                     level=QgsMessageBar.WARNING)
                return None
            alg.addParameterSource(param.name(), val)

        # outputs = self._alg.outputDefinitions()
        #for output in outputs:
        #    if not output.flags() & QgsProcessingParameterDefinition.FlagHidden:
        #        name = str(self.valueItems[output.name()].text())
        #        if name.strip() != '' and name != ModelerParametersDialog.ENTER_NAME:
         #           alg.outputs[output.name()] = QgsProcessingModelAlgorithm.ModelOutput(name)

        selectedOptions = self.dependenciesPanel.selectedoptions
        availableDependencies = self.getAvailableDependencies()  # spellok
        dep_ids = []
        for selected in selectedOptions:
            dep_ids.append(availableDependencies[selected].childId())  # spellok
        alg.setDependencies(dep_ids)

        try:
            self._alg.processBeforeAddingToModeler(alg, self.model)
        except:
            pass

        return alg

    def okPressed(self):
        self.alg = self.createAlgorithm()
        if self.alg is not None:
            self.close()

    def cancelPressed(self):
        self.alg = None
        self.close()

    def openHelp(self):
        algHelp = self._alg.help()
        if algHelp is not None:
            webbrowser.open(algHelp)
