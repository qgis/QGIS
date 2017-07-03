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
                       QgsProcessingModelAlgorithm,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterRasterOutput,
                       QgsProcessingParameterFileOutput,
                       QgsProcessingParameterFolderOutput,
                       QgsProcessingOutputDefinition)

from qgis.gui import (QgsMessageBar,
                      QgsScrollArea)

from processing.gui.wrappers import WidgetWrapperFactory
from processing.gui.wrappers import InvalidParameterValue
from processing.gui.MultipleInputPanel import MultipleInputPanel


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
        self.childId = algName
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

        for dest in self._alg.destinationParameterDefinitions():
            if dest.flags() & QgsProcessingParameterDefinition.FlagHidden:
                continue
            if isinstance(dest, (QgsProcessingParameterRasterOutput, QgsProcessingParameterFeatureSink,
                                 QgsProcessingParameterFileOutput, QgsProcessingParameterFolderOutput)):
                label = QLabel(dest.description())
                item = QLineEdit()
                if hasattr(item, 'setPlaceholderText'):
                    item.setPlaceholderText(ModelerParametersDialog.ENTER_NAME)
                self.verticalLayout.addWidget(label)
                self.verticalLayout.addWidget(item)
                self.valueItems[dest.name()] = item

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
        if self.childId is None:
            dependent = []
        else:
            dependent = list(self.model.dependentChildAlgorithms(self.childId))
            dependent.append(self.childId)
        opts = []
        for alg in list(self.model.childAlgorithms().values()):
            if alg.childId() not in dependent:
                opts.append(alg)
        return opts

    def getDependenciesPanel(self):
        return MultipleInputPanel([alg.description() for alg in self.getAvailableDependencies()])  # spellok

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

    def getAvailableValuesOfType(self, paramType, outTypes=[], dataTypes=[]):
        # upgrade paramType to list
        if paramType is None:
            paramType = []
        elif not isinstance(paramType, list):
            paramType = [paramType]
        if outTypes is None:
            outTypes = []
        elif not isinstance(outTypes, list):
            outTypes = [outTypes]

        return self.model.availableSourcesForChild(self.childId, [p.typeName() for p in paramType if issubclass(p, QgsProcessingParameterDefinition)],
                                                   [o.typeName() for o in outTypes if issubclass(o, QgsProcessingOutputDefinition)], dataTypes)

    def resolveValueDescription(self, value):
        if isinstance(value, QgsProcessingModelAlgorithm.ChildParameterSource):
            if value.source() == QgsProcessingModelAlgorithm.ChildParameterSource.StaticValue:
                return value.staticValue()
            elif value.source() == QgsProcessingModelAlgorithm.ChildParameterSource.ModelParameter:
                return self.model.parameterDefinition(value.parameterName()).description()
            elif value.source() == QgsProcessingModelAlgorithm.ChildParameterSource.ChildOutput:
                alg = self.model.childAlgorithm(value.outputChildId())
                return self.tr("'{0}' from algorithm '{1}'").format(
                    alg.algorithm().outputDefinition(value.outputName()).description(), alg.description())

        return value

    def setPreviousValues(self):
        if self.childId is not None:
            alg = self.model.childAlgorithm(self.childId)
            self.descriptionBox.setText(alg.description())
            for param in alg.algorithm().parameterDefinitions():
                if param.isDestination() or param.flags() & QgsProcessingParameterDefinition.FlagHidden:
                    continue
                if param.name() in alg.parameterSources():
                    value = alg.parameterSources()[param.name()]
                    if isinstance(value, list) and len(value) == 1:
                        value = value[0]
                else:
                    value = param.defaultValue()

                if isinstance(value, QgsProcessingModelAlgorithm.ChildParameterSource) and value.source() == QgsProcessingModelAlgorithm.ChildParameterSource.StaticValue:
                    value = value.staticValue()

                self.wrappers[param.name()].setValue(value)
            for name, out in list(alg.modelOutputs().items()):
                if out.childOutputName() in self.valueItems:
                    self.valueItems[out.childOutputName()].setText(out.name())

            selected = []
            dependencies = self.getAvailableDependencies()  # spellok
            for idx, dependency in enumerate(dependencies):
                if dependency.childId() in alg.dependencies():
                    selected.append(idx)

            self.dependenciesPanel.setSelectedItems(selected)

    def createAlgorithm(self):
        alg = QgsProcessingModelAlgorithm.ChildAlgorithm(self._alg.id())
        if not self.childId:
            alg.generateChildId(self.model)
        else:
            alg.setChildId(self.childId)
        alg.setDescription(self.descriptionBox.text())
        for param in self._alg.parameterDefinitions():
            if param.isDestination() or param.flags() & QgsProcessingParameterDefinition.FlagHidden:
                continue
            val = self.wrappers[param.name()].value()
            if (isinstance(val,
                           QgsProcessingModelAlgorithm.ChildParameterSource) and val.source() == QgsProcessingModelAlgorithm.ChildParameterSource.StaticValue and not param.checkValueIsAcceptable(
                    val.staticValue())) \
                    or (val is None and not param.flags() & QgsProcessingParameterDefinition.FlagOptional):
                self.bar.pushMessage("Error", "Wrong or missing value for parameter '%s'" % param.description(),
                                     level=QgsMessageBar.WARNING)
                return None
            if val is None:
                continue
            elif isinstance(val, QgsProcessingModelAlgorithm.ChildParameterSource):
                alg.addParameterSources(param.name(), [val])
            elif isinstance(val, list):
                alg.addParameterSources(param.name(), val)
            else:
                alg.addParameterSources(param.name(), [QgsProcessingModelAlgorithm.ChildParameterSource.fromStaticValue(val)])

        outputs = {}
        for dest in self._alg.destinationParameterDefinitions():
            if not dest.flags() & QgsProcessingParameterDefinition.FlagHidden:
                name = str(self.valueItems[dest.name()].text())
                if name.strip() != '' and name != ModelerParametersDialog.ENTER_NAME:
                    output = QgsProcessingModelAlgorithm.ModelOutput(name, name)
                    output.setChildId(alg.childId())
                    output.setChildOutputName(dest.name())
                    outputs[name] = output
        alg.setModelOutputs(outputs)

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
