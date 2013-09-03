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
from processing.outputs.OutputExtent import OutputExtent

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtCore, QtGui, QtWebKit
from processing.modeler.MultilineTextPanel import MultilineTextPanel
from processing.gui.CrsSelectionPanel import CrsSelectionPanel
from processing.parameters.ParameterCrs import ParameterCrs
from processing.outputs.OutputString import OutputString
from processing.parameters.ParameterRaster import ParameterRaster
from processing.parameters.ParameterVector import ParameterVector
from processing.parameters.ParameterBoolean import ParameterBoolean
from processing.parameters.ParameterSelection import ParameterSelection
from processing.parameters.ParameterMultipleInput import ParameterMultipleInput
from processing.gui.MultipleInputPanel import MultipleInputPanel
from processing.parameters.ParameterFixedTable import ParameterFixedTable
from processing.gui.FixedTablePanel import FixedTablePanel
from processing.parameters.ParameterNumber import ParameterNumber
from processing.parameters.ParameterTableField import ParameterTableField
from processing.parameters.ParameterTable import ParameterTable
from processing.parameters.ParameterString import ParameterString
from processing.outputs.OutputRaster import OutputRaster
from processing.outputs.OutputVector import OutputVector
from processing.outputs.OutputTable import OutputTable
from processing.modeler.ModelerAlgorithm import AlgorithmAndParameter
from processing.parameters.ParameterRange import ParameterRange
from processing.gui.RangePanel import RangePanel
from processing.outputs.OutputNumber import OutputNumber
from processing.outputs.OutputHTML import OutputHTML
from processing.parameters.ParameterFile import ParameterFile
from processing.outputs.OutputFile import OutputFile
from processing.core.WrongHelpFileException import WrongHelpFileException
from processing.parameters.ParameterExtent import ParameterExtent

class ModelerParametersDialog(QtGui.QDialog):

    ENTER_NAME = "[Enter name if this is a final result]"
    NOT_SELECTED = "[Not selected]"
    USE_MIN_COVERING_EXTENT = "[Use min covering extent]"

    def __init__(self, alg, model, algIndex = None):
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.alg = alg
        self.model = model
        self.algIndex = algIndex
        self.setupUi()
        self.params = None


    def setupUi(self):
        self.labels = {}
        self.widgets = {}
        self.checkBoxes = {}
        self.showAdvanced = False
        self.valueItems = {}
        self.dependentItems = {}
        self.resize(650, 450)
        self.buttonBox = QtGui.QDialogButtonBox()
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        tooltips = self.alg.getParameterDescriptions()
        self.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        self.verticalLayout = QtGui.QVBoxLayout()
        self.verticalLayout.setSpacing(5)
        self.verticalLayout.setMargin(20)
        for param in self.alg.parameters:
            if param.isAdvanced:
                self.advancedButton = QtGui.QPushButton()
                self.advancedButton.setText("Show advanced parameters")
                self.advancedButton.setMaximumWidth(150)
                QtCore.QObject.connect(self.advancedButton, QtCore.SIGNAL("clicked()"), self.showAdvancedParametersClicked)
                self.verticalLayout.addWidget(self.advancedButton)
                break
        for param in self.alg.parameters:
            if param.hidden:
                continue
            desc = param.description
            if isinstance(param, ParameterExtent):
                desc += "(xmin, xmax, ymin, ymax)"
            label = QtGui.QLabel(desc)
            self.labels[param.name] = label
            widget = self.getWidgetFromParameter(param)
            self.valueItems[param.name] = widget
            if param.name in tooltips.keys():
                tooltip = tooltips[param.name]
            else:
                tooltip = param.description
            label.setToolTip(tooltip)
            widget.setToolTip(tooltip)
            if param.isAdvanced:
                label.setVisible(self.showAdvanced)
                widget.setVisible(self.showAdvanced)
                self.widgets[param.name] = widget
            self.verticalLayout.addWidget(label)
            self.verticalLayout.addWidget(widget)

        for output in self.alg.outputs:
            if output.hidden:
                continue
            if isinstance(output, (OutputRaster, OutputVector, OutputTable, OutputHTML, OutputFile)):
                label = QtGui.QLabel(output.description + "<" + output.__module__.split(".")[-1] + ">")
                item = QLineEdit()
                if hasattr(item, 'setPlaceholderText'):
                    item.setPlaceholderText(ModelerParametersDialog.ENTER_NAME)
                self.verticalLayout.addWidget(label)
                self.verticalLayout.addWidget(item)
                self.valueItems[output.name] = item

        label = QtGui.QLabel(" ")
        self.verticalLayout.addWidget(label)
        label = QtGui.QLabel("Parent algorithms")
        self.dependenciesPanel = self.getDependenciesPanel()
        self.verticalLayout.addWidget(label)
        self.verticalLayout.addWidget(self.dependenciesPanel)

        self.verticalLayout.addStretch(1000)
        self.setLayout(self.verticalLayout)

        self.setPreviousValues()
        self.setWindowTitle(self.alg.name)
        self.verticalLayout2 = QtGui.QVBoxLayout()
        self.verticalLayout2.setSpacing(2)
        self.verticalLayout2.setMargin(0)
        self.tabWidget = QtGui.QTabWidget()
        self.tabWidget.setMinimumWidth(300)
        self.paramPanel = QtGui.QWidget()
        self.paramPanel.setLayout(self.verticalLayout)
        self.scrollArea = QtGui.QScrollArea()
        self.scrollArea.setWidget(self.paramPanel)
        self.scrollArea.setWidgetResizable(True)
        self.tabWidget.addTab(self.scrollArea, "Parameters")
        self.webView = QtWebKit.QWebView()
        html = None
        try:
            if self.alg.helpFile():
                helpFile = self.alg.helpFile()
            else:
                html = "<h2>Sorry, no help is available for this algorithm.</h2>"
        except WrongHelpFileException, e:
            html = e.msg
            self.webView.setHtml("<h2>Could not open help file :-( </h2>")
        try:
            if html:
                self.webView.setHtml(html)
            else:
                url = QtCore.QUrl(helpFile)
                self.webView.load(url)
        except:
            self.webView.setHtml("<h2>Could not open help file :-( </h2>")
        self.tabWidget.addTab(self.webView, "Help")
        self.verticalLayout2.addWidget(self.tabWidget)
        self.verticalLayout2.addWidget(self.buttonBox)
        self.setLayout(self.verticalLayout2)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL("accepted()"), self.okPressed)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL("rejected()"), self.cancelPressed)
        QtCore.QMetaObject.connectSlotsByName(self)


    def getAvailableDependencies(self):
        if self.algIndex is None:
            dependent = []
        else:
            dependent = self.model.getDependentAlgorithms(self.algIndex)
        opts = []
        i=0
        for alg in self.model.algs:
            if i not in dependent:
                opts.append(str(i+1) + ":" + alg.name)
            i+=1
        return opts

    def getDependenciesPanel(self):
        return MultipleInputPanel(self.getAvailableDependencies())


    def showAdvancedParametersClicked(self):
        self.showAdvanced = not self.showAdvanced
        if self.showAdvanced:
            self.advancedButton.setText("Hide advanced parameters")
        else:
            self.advancedButton.setText("Show advanced parameters")
        for param in self.alg.parameters:
            if param.isAdvanced:
                self.labels[param.name].setVisible(self.showAdvanced)
                self.widgets[param.name].setVisible(self.showAdvanced)

    def getRasterLayers(self):
        layers = []
        params = self.model.parameters
        for param in params:
            if isinstance(param, ParameterRaster):
                layers.append(AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, param.name, "", param.description))

        if self.algIndex is None:
            dependent = []
        else:
            dependent = self.model.getDependentAlgorithms(self.algIndex)
            #dependent.append(self.algIndex)

        i=0
        for alg in self.model.algs:
            if i not in dependent:
                for out in alg.outputs:
                    if isinstance(out, OutputRaster):
                        layers.append(AlgorithmAndParameter(i, out.name, alg.name, out.description))
            i+=1

        return layers

    def getVectorLayers(self):
        layers = []
        params = self.model.parameters
        for param in params:
            if isinstance(param, ParameterVector):
                layers.append(AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, param.name, "", param.description))

        if self.algIndex is None:
            dependent = []
        else:
            dependent = self.model.getDependentAlgorithms(self.algIndex)
            #dependent.append(self.algIndex)

        i=0
        for alg in self.model.algs:
            if i not in dependent:
                for out in alg.outputs:
                    if isinstance(out, OutputVector):
                        layers.append(AlgorithmAndParameter(i, out.name, alg.name, out.description))
            i+=1

        return layers

    def getTables(self):
        tables = []
        params = self.model.parameters
        for param in params:
            if isinstance(param, ParameterTable):
                tables.append(AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, param.name, "", param.description))

        if self.algIndex is None:
            dependent = []
        else:
            dependent = self.model.getDependentAlgorithms(self.algIndex)
            #dependent.append(self.algIndex)

        i=0
        for alg in self.model.algs:
            if i not in dependent:
                for out in alg.outputs:
                    if isinstance(out, OutputTable):
                        tables.append(AlgorithmAndParameter(i, out.name, alg.name, out.description))
            i+=1

        return tables

    def getExtents(self):
        extents = []
        params = self.model.parameters
        for param in params:
            if isinstance(param, ParameterExtent):
                extents.append(AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, param.name, "", param.description))


        if self.algIndex is None:
            dependent = []
        else:
            dependent = self.model.getDependentAlgorithms(self.algIndex)
            #dependent.append(self.algIndex)

        i=0
        for alg in self.model.algs:
            if i not in dependent:
                for out in alg.outputs:
                    if isinstance(out, OutputExtent):
                        extents.append(AlgorithmAndParameter(i, out.name, alg.name, out.description))
            i+=1

        return extents

    def getNumbers(self):
        numbers = []
        params = self.model.parameters
        for param in params:
            if isinstance(param, ParameterNumber):
                numbers.append(AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, param.name, "", param.description))

        if self.algIndex is None:
            dependent = []
        else:
            dependent = self.model.getDependentAlgorithms(self.algIndex)
            #dependent.append(self.algIndex)

        i=0
        for alg in self.model.algs:
            if i not in dependent:
                for out in alg.outputs:
                    if isinstance(out, OutputNumber):
                        numbers.append(AlgorithmAndParameter(i, out.name, alg.name, out.description))
            i+=1
        return numbers

    def getFiles(self):
        files = []
        params = self.model.parameters
        for param in params:
            if isinstance(param, ParameterFile):
                files.append(AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, param.name, "", param.description))

        if self.algIndex is None:
            dependent = []
        else:
            dependent = self.model.getDependentAlgorithms(self.algIndex)
            #dependent.append(self.algIndex)

        i=0
        for alg in self.model.algs:
            if i not in dependent:
                for out in alg.outputs:
                    if isinstance(out, OutputFile):
                        files.append(AlgorithmAndParameter(i, out.name, alg.name, out.description))
            i+=1
        return files

    def getBooleans(self):
        booleans = []
        params = self.model.parameters
        for param in params:
            if isinstance(param, ParameterBoolean):
                booleans.append(AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, param.name, "", param.description))
        return booleans

    def getStrings(self):
        strings = []
        params = self.model.parameters
        for param in params:
            if isinstance(param, ParameterString):
                strings.append(AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, param.name, "", param.description))

        if self.algIndex is None:
            dependent = []
        else:
            dependent = self.model.getDependentAlgorithms(self.algIndex)
            #dependent.append(self.algIndex)

        i=0
        for alg in self.model.algs:
            if i not in dependent:
                for out in alg.outputs:
                    if isinstance(out, OutputString):
                        strings.append(AlgorithmAndParameter(i, out.name, alg.name, out.description))
            i+=1
        return strings

    def getTableFields(self):
        strings = []
        params = self.model.parameters
        for param in params:
            if isinstance(param, ParameterTableField):
                strings.append(AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, param.name, "", param.description))
        return strings

    def getWidgetFromParameter(self, param):
        if isinstance(param, ParameterRaster):
            item = QtGui.QComboBox()
            #item.setEditable(True)
            layers = self.getRasterLayers()
            if (param.optional):
                item.addItem(self.NOT_SELECTED, None)
            for layer in layers:
                item.addItem(layer.name(), layer)
        elif isinstance(param, ParameterVector):
            item = QtGui.QComboBox()
            #item.setEditable(True)
            layers = self.getVectorLayers()
            if (param.optional):
                item.addItem(self.NOT_SELECTED, None)
            for layer in layers:
                item.addItem(layer.name(), layer)
        elif isinstance(param, ParameterTable):
            item = QtGui.QComboBox()
            item.setEditable(True)
            layers = self.getTables()
            if (param.optional):
                item.addItem(self.NOT_SELECTED, None)
            for layer in layers:
                item.addItem(layer.name(), layer)
        elif isinstance(param, ParameterBoolean):
            item = QtGui.QComboBox()
            item.addItem("Yes")
            item.addItem("No")
            bools = self.getBooleans()
            for b in bools:
                item.addItem(b.name(), b)
        elif isinstance(param, ParameterSelection):
            item = QtGui.QComboBox()
            item.addItems(param.options)
        elif isinstance(param, ParameterFixedTable):
            item = FixedTablePanel(param)
        elif isinstance(param, ParameterRange):
            item = RangePanel(param)
        elif isinstance(param, ParameterMultipleInput):
            if param.datatype == ParameterMultipleInput.TYPE_VECTOR_ANY:
                options = self.getVectorLayers()
            else:
                options = self.getRasterLayers()
            opts = []
            for opt in options:
                opts.append(opt.name())
            item = MultipleInputPanel(opts)
        elif isinstance(param, ParameterString):
            strings = self.getStrings()
            if param.multiline:
                item = MultilineTextPanel(strings,self.model)
                item.setText(str(param.default))
            else:
                item = QtGui.QComboBox()
                item.setEditable(True)
                for s in strings:
                    item.addItem(s.name(), s)
                item.setEditText(str(param.default))
        elif isinstance(param, ParameterTableField):
            item = QtGui.QComboBox()
            item.setEditable(True)
            fields = self.getTableFields()
            for f in fields:
                item.addItem(f.name(), f)
        elif isinstance(param, ParameterNumber):
            item = QtGui.QComboBox()
            item.setEditable(True)
            numbers = self.getNumbers()
            for n in numbers:
                item.addItem(n.name(), n)
            item.setEditText(str(param.default))
        elif isinstance(param, ParameterCrs):
            item = CrsSelectionPanel(param.default)
        elif isinstance(param, ParameterExtent):
            item = QtGui.QComboBox()
            item.setEditable(True)
            extents = self.getExtents()
            if self.canUseAutoExtent():
                item.addItem(self.USE_MIN_COVERING_EXTENT, None)
            for ex in extents:
                item.addItem(ex.name(), ex)
            if not self.canUseAutoExtent():
                item.setEditText(str(param.default))
        elif isinstance(param, ParameterFile):
            item = QtGui.QComboBox()
            item.setEditable(True)
            files = self.getFiles()
            for f in files:
                item.addItem(f.name(), f)
        else:
            item = QtGui.QLineEdit()
            try:
                item.setText(str(param.default))
            except:
                pass
        return item

    def canUseAutoExtent(self):
        for param in self.alg.parameters:
            if isinstance(param, (ParameterRaster, ParameterVector)):
                return True
            if isinstance(param, ParameterMultipleInput):
                return True

    def setTableContent(self):
        params = self.alg.parameters
        outputs = self.alg.outputs
        numParams = 0
        for param in params:
            if not param.hidden:
                numParams += 1
        numOutputs = 0
        for output in outputs:
            if not output.hidden:
                numOutputs += 1
        self.tableWidget.setRowCount(numParams + numOutputs)

        i=0
        for param in params:
            if not param.hidden:
                item = QtGui.QTableWidgetItem(param.description)
                item.setFlags(QtCore.Qt.ItemIsEnabled)
                self.tableWidget.setItem(i,0, item)
                item = self.getWidgetFromParameter(param)
                self.valueItems[param.name] = item
                self.tableWidget.setCellWidget(i,1, item)
                self.tableWidget.setRowHeight(i,22)
                i+=1

        for output in outputs:
            if not output.hidden:
                item = QtGui.QTableWidgetItem(output.description + "<" + output.__module__.split(".")[-1] + ">")
                item.setFlags(QtCore.Qt.ItemIsEnabled)
                self.tableWidget.setItem(i,0, item)
                item = QLineEdit()
                if hasattr(item, 'setPlaceholderText'):
                    item.setPlaceholderText(ModelerParametersDialog.ENTER_NAME)
                self.valueItems[output.name] = item
                self.tableWidget.setCellWidget(i,1, item)
                self.tableWidget.setRowHeight(i,22)
                i+=1


    def setComboBoxValue(self, combo, value, param):
        items = [combo.itemData(i) for i in range(combo.count())]
        idx = 0
        for item in items:
            if item and value:
                if item.alg == value.alg and item.param == value.param:
                    combo.setCurrentIndex(idx)
                    return
            idx += 1
        if combo.isEditable():
            value = self.model.getValueFromAlgorithmAndParameter(value)
            if value:
                combo.setEditText(str(value))
        elif isinstance(param, ParameterSelection):
            value = self.model.getValueFromAlgorithmAndParameter(value)
            combo.setCurrentIndex(int(value))
        elif isinstance(param, ParameterBoolean):
            value = self.model.getValueFromAlgorithmAndParameter(value) == str(True)
            if value:
                combo.setCurrentIndex(0)
            else:
                combo.setCurrentIndex(1)


    def setPreviousValues(self):
        if self.algIndex is not None:
            for name, value in self.model.algParameters[self.algIndex].items():
                widget = self.valueItems[name]
                param = self.alg.getParameterFromName(name)
                if isinstance(param, (ParameterRaster, ParameterVector,
                                      ParameterTable, ParameterTableField,
                                      ParameterSelection, ParameterNumber,
                                      ParameterBoolean, ParameterExtent)):
                    self.setComboBoxValue(widget, value, param)
                elif isinstance(param, ParameterString):
                    if param.multiline:
                        widget.setValue(value)
                    else:
                        self.setComboBoxValue(widget, value, param)
                elif isinstance(param, ParameterCrs):
                    value = self.model.getValueFromAlgorithmAndParameter(value)
                    widget.setAuthid(value)
                elif isinstance(param, ParameterFixedTable):
                    pass
                elif isinstance(param, ParameterMultipleInput):
                    value = self.model.getValueFromAlgorithmAndParameter(value)
                    values = value.split(";")
                    selectedoptions = []
                    if param.datatype == ParameterMultipleInput.TYPE_VECTOR_ANY:
                        options = self.getVectorLayers()
                    else:
                        options = self.getRasterLayers()
                    for i in range(len(options)):
                        option = options[i]
                        for aap in (values):
                            if str(option) == aap:
                                selectedoptions.append(i)
                    widget.setSelectedItems(selectedoptions)
                else:
                    pass

            for out in self.alg.outputs:
                if not out.hidden:
                    value = self.model.algOutputs[self.algIndex][out.name]
                    if value is not None:
                        widget = self.valueItems[out.name].setText(unicode(value))

            selected = []
            dependencies = self.getAvailableDependencies()
            index = -1
            for dependency in dependencies:
                index += 1
                n = int(dependency[:dependency.find(":")]) - 1
                if n in self.model.dependencies[self.algIndex]:
                    selected.append(index)

            self.dependenciesPanel.setSelectedItems(selected)


    def setParamValues(self):
        self.params = {}
        self.values = {}
        self.outputs = {}

        params = self.alg.parameters
        outputs = self.alg.outputs

        for param in params:
            if param.hidden:
                continue
            if not self.setParamValue(param, self.valueItems[param.name]):
                return False
        for output in outputs:
            if output.hidden:
                self.outputs[output.name] = None
            else:
                name= unicode(self.valueItems[output.name].text())
                if name.strip()!="" and name != ModelerParametersDialog.ENTER_NAME:
                    self.outputs[output.name]=name
                else:
                    self.outputs[output.name] = None

        selectedOptions = self.dependenciesPanel.selectedoptions
        #this index are based on the list of available dependencies.
        #we translate them into indices based on the whole set of algorithm in the model
        #We just take the values in the beginning of the string representing the algorithm
        availableDependencies = self.getAvailableDependencies()
        self.dependencies = []
        for selected in selectedOptions:
            s = availableDependencies[selected]
            n = int(s[:s.find(":")]) - 1
            self.dependencies.append(n);

        return True


    def setParamValueLayerOrTable(self, param, widget):
        idx = widget.currentIndex()
        if idx < 0:
            return False
        else:
            value = widget.itemData(widget.currentIndex())
            self.params[param.name] = value
            return True

    def setParamBooleanValue(self, param, widget):
        if widget.currentIndex() < 2:
            name =  self.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            self.values[name] = str(widget.currentIndex() == 0)
        else:
            value = widget.itemData(widget.currentIndex())
            self.params[param.name] = value
        return True

    def setParamTableFieldValue(self, param, widget):
        idx = widget.findText(widget.currentText())
        if idx < 0:
            name =  self.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            s = str(widget.currentText()).strip()
            if s == "":
                if param.optional:
                    self.params[param.name] = None
                    return True
                else:
                    return False
            else:
                self.params[param.name] = value
                self.values[name] = str(widget.currentText())
                return True
        else:
            value = widget.itemData(widget.currentIndex())
            self.params[param.name] = value
        return True

    def setParamStringValue(self, param, widget):
        if param.multiline:
            name =  self.getSafeNameForHarcodedParameter(param)
            paramValue = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            value = widget.getValue()
            option = widget.getOption()
            if option == MultilineTextPanel.USE_TEXT:
                if value == "":
                    if param.optional:
                        self.params[param.name] = None
                        return True
                    else:
                        return False
                else:
                    self.values[name] = value

                    self.params[param.name] = paramValue
            else:
                self.params[param.name] = value
        else:
            if widget.currentText() == "":
                return False
            idx = widget.findText(widget.currentText())
            if idx < 0:
                name =  self.getSafeNameForHarcodedParameter(param)
                value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
                self.params[param.name] = value
                value = str(widget.currentText()).strip()
                if value == "":
                    if param.optional:
                        self.values[name] = None
                        return True
                    else:
                        return False
                else:
                    self.values[name] = str(widget.currentText())
            else:
                value = widget.itemData(widget.currentIndex())
                self.params[param.name] = value
        return True

    def setParamFileValue(self, param, widget):
        idx = widget.findText(widget.currentText())
        if idx < 0:
            name = self.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            s = str(widget.currentText())
            self.values[name] = s
        else:
            value = widget.itemData(widget.currentIndex())
            self.params[param.name] = value
        return True

    def setParamNumberValue(self, param, widget):
        idx = widget.findText(widget.currentText())
        if idx < 0:
            name =  self.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            s = str(widget.currentText())
            try:
                float(s)
                self.values[name] = s
            except:
                return False
        else:
            value = widget.itemData(widget.currentIndex())
            self.params[param.name] = value
        return True

    def setParamExtentValue(self, param, widget):
        idx = widget.findText(widget.currentText())
        if idx < 0:
            name =  self.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            s = str(widget.currentText())
            try:
                tokens = s.split(",")
                if len(tokens) != 4:
                    return False
                for token in tokens:
                    float(token)
            except:
                return False
            self.values[name] = s
        else:
            value = widget.itemData(widget.currentIndex())
            self.params[param.name] = value
        return True

    def setParamValue(self, param, widget):
        if isinstance(param, (ParameterRaster, ParameterVector, ParameterTable)):
            return self.setParamValueLayerOrTable(param, widget)
        elif isinstance(param, ParameterBoolean):
            return self.setParamBooleanValue(param, widget)
        elif isinstance(param, ParameterString):
            return self.setParamStringValue(param, widget)
        elif isinstance(param, ParameterNumber):
            return self.setParamNumberValue(param, widget)
        elif isinstance(param, ParameterExtent):
            return self.setParamExtentValue(param, widget)
        elif isinstance(param, ParameterFile):
            return self.setParamFileValue(param, widget)
        elif isinstance(param, ParameterSelection):
            name =  self.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            self.values[name] = str(widget.currentIndex())
            return True
        elif isinstance(param, ParameterRange):
            name =  self.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            self.values[name] = str(widget.getValue())
            return True
        elif isinstance(param, ParameterCrs):
            authid = widget.getValue()
            if authid is None:
                self.params[param.name] = None
            else:
                name =  self.getSafeNameForHarcodedParameter(param)
                value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
                self.params[param.name] = value
                self.values[name] = authid
            return True
        elif isinstance(param, ParameterFixedTable):
            name =  self.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            self.values[name] = ParameterFixedTable.tableToString(widget.table)
            return True
        elif isinstance(param, ParameterTableField):
            return self.setParamTableFieldValue(param, widget)
        elif isinstance(param, ParameterMultipleInput):
            if param.datatype == ParameterMultipleInput.TYPE_VECTOR_ANY:
                options = self.getVectorLayers()
            else:
                options = self.getRasterLayers()
            values = []
            for index in widget.selectedoptions:
                values.append(options[index].serialize())
            if len(values) == 0 and not param.optional:
                return False
            name =  self.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            self.values[name] = ";".join(values)
            return True
        else:
            name =  self.getSafeNameForHarcodedParameter(param)
            value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
            self.params[param.name] = value
            self.values[name] = unicode(widget.text())
            return True

    def getSafeNameForHarcodedParameter(self, param):
        if self.algIndex is None:
            return "HARDCODEDPARAMVALUE_" + param.name + "_" + str(len(self.model.algs))
        else:
            return "HARDCODEDPARAMVALUE_" + param.name + "_" + str(self.algIndex)


    def okPressed(self):
        if self.setParamValues():
            self.close()
        else:
            QMessageBox.critical(self, "Unable to add algorithm", "Wrong or missing parameter values")
            self.params = None


    def cancelPressed(self):
        self.params = None
        self.close()












