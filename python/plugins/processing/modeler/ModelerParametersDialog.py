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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from PyQt4.QtCore import Qt, QUrl, QMetaObject
from PyQt4.QtGui import QDialog, QDialogButtonBox, QLabel, QLineEdit, QFrame, QPushButton, QSizePolicy, QVBoxLayout, QHBoxLayout, QTabWidget, QWidget, QScrollArea, QComboBox, QTableWidgetItem, QMessageBox
from PyQt4.QtWebKit import QWebView

from processing.modeler.ModelerAlgorithm import ValueFromInput, \
    ValueFromOutput, Algorithm, ModelerOutput
from processing.gui.CrsSelectionPanel import CrsSelectionPanel
from processing.gui.MultipleInputPanel import MultipleInputPanel
from processing.gui.FixedTablePanel import FixedTablePanel
from processing.gui.RangePanel import RangePanel
from processing.modeler.MultilineTextPanel import MultilineTextPanel
from processing.core.parameters import ParameterExtent, ParameterRaster, ParameterVector, ParameterBoolean, ParameterTable, ParameterFixedTable, ParameterMultipleInput, ParameterSelection, ParameterRange, ParameterNumber, ParameterString, ParameterCrs, ParameterTableField, ParameterFile
from processing.core.outputs import OutputRaster, OutputVector, OutputTable, OutputHTML, OutputFile, OutputDirectory, OutputNumber, OutputString, OutputExtent


class ModelerParametersDialog(QDialog):

    ENTER_NAME = '[Enter name if this is a final result]'
    NOT_SELECTED = '[Not selected]'
    USE_MIN_COVERING_EXTENT = '[Use min covering extent]'

    def __init__(self, alg, model, algName=None):
        QDialog.__init__(self)
        self.setModal(True)
        #The algorithm to define in this dialog. It is an instance of GeoAlgorithm
        self._alg = alg
        #The resulting algorithm after the user clicks on OK. it is an instance of the container Algorithm class
        self.alg = None
        #The model this algorithm is going to be added to
        self.model = model
        #The name of the algorithm in the model, in case we are editing it and not defining it for the first time
        self._algName = algName
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
        self.buttonBox = QDialogButtonBox()
        self.buttonBox.setOrientation(Qt.Horizontal)
        self.buttonBox.setStandardButtons(QDialogButtonBox.Cancel
                | QDialogButtonBox.Ok)
        tooltips = self._alg.getParameterDescriptions()
        self.setSizePolicy(QSizePolicy.Expanding,
                           QSizePolicy.Expanding)
        self.verticalLayout = QVBoxLayout()
        self.verticalLayout.setSpacing(5)
        self.verticalLayout.setMargin(20)

        hLayout = QHBoxLayout()
        hLayout.setSpacing(5)
        hLayout.setMargin(0)
        descriptionLabel = QLabel(self.tr("Description"))
        self.descriptionBox = QLineEdit()
        self.descriptionBox.setText(self._alg.name)
        hLayout.addWidget(descriptionLabel)
        hLayout.addWidget(self.descriptionBox)
        self.verticalLayout.addLayout(hLayout)
        line = QFrame()
        line.setFrameShape(QFrame.HLine)
        line.setFrameShadow(QFrame.Sunken)
        self.verticalLayout.addWidget(line)

        for param in self._alg.parameters:
            if param.isAdvanced:
                self.advancedButton = QPushButton()
                self.advancedButton.setText(self.tr('Show advanced parameters'))
                self.advancedButton.setMaximumWidth(150)
                self.advancedButton.clicked.connect(
                    self.showAdvancedParametersClicked)
                self.verticalLayout.addWidget(self.advancedButton)
                break
        for param in self._alg.parameters:
            if param.hidden:
                continue
            desc = param.description
            if isinstance(param, ParameterExtent):
                desc += '(xmin, xmax, ymin, ymax)'
            label = QLabel(desc)
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

        for output in self._alg.outputs:
            if output.hidden:
                continue
            if isinstance(output, (OutputRaster, OutputVector, OutputTable,
                          OutputHTML, OutputFile, OutputDirectory)):
                label = QLabel(output.description + '<'
                               + output.__class__.__name__ + '>')
                item = QLineEdit()
                if hasattr(item, 'setPlaceholderText'):
                    item.setPlaceholderText(ModelerParametersDialog.ENTER_NAME)
                self.verticalLayout.addWidget(label)
                self.verticalLayout.addWidget(item)
                self.valueItems[output.name] = item

        label = QLabel(' ')
        self.verticalLayout.addWidget(label)
        label = QLabel(self.tr('Parent algorithms'))
        self.dependenciesPanel = self.getDependenciesPanel()
        self.verticalLayout.addWidget(label)
        self.verticalLayout.addWidget(self.dependenciesPanel)

        self.verticalLayout.addStretch(1000)
        self.setLayout(self.verticalLayout)

        self.setPreviousValues()
        self.setWindowTitle(self._alg.name)
        self.verticalLayout2 = QVBoxLayout()
        self.verticalLayout2.setSpacing(2)
        self.verticalLayout2.setMargin(0)
        self.tabWidget = QTabWidget()
        self.tabWidget.setMinimumWidth(300)
        self.paramPanel = QWidget()
        self.paramPanel.setLayout(self.verticalLayout)
        self.scrollArea = QScrollArea()
        self.scrollArea.setWidget(self.paramPanel)
        self.scrollArea.setWidgetResizable(True)
        self.tabWidget.addTab(self.scrollArea, self.tr('Parameters'))
        self.webView = QWebView()

        html = None
        url = None
        isText, help = self._alg.help()
        if help is not None:
            if isText:
                html = help
            else:
                url = QUrl(help)
        else:
            html = self.tr('<h2>Sorry, no help is available for this '
                           'algorithm.</h2>')
        try:
            if html:
                self.webView.setHtml(html)
            elif url:
                self.webView.load(url)
        except:
            self.webView.setHtml(self.tr('<h2>Could not open help file :-( </h2>'))
        self.tabWidget.addTab(self.webView, 'Help')
        self.verticalLayout2.addWidget(self.tabWidget)
        self.verticalLayout2.addWidget(self.buttonBox)
        self.setLayout(self.verticalLayout2)
        self.buttonBox.accepted.connect(self.okPressed)
        self.buttonBox.rejected.connect(self.cancelPressed)
        QMetaObject.connectSlotsByName(self)

    def getAvailableDependencies(self):
        if self._algName is None:
            dependent = []
        else:
            dependent = self.model.getDependentAlgorithms(self._algName)
        opts = []
        for alg in self.model.algs.values():
            if alg.name not in dependent:
                opts.append(alg)
        return opts

    def getDependenciesPanel(self):
        return MultipleInputPanel([alg.algorithm.name for alg in self.getAvailableDependencies()])

    def showAdvancedParametersClicked(self):
        self.showAdvanced = not self.showAdvanced
        if self.showAdvanced:
            self.advancedButton.setText(self.tr('Hide advanced parameters'))
        else:
            self.advancedButton.setText(self.tr('Show advanced parameters'))
        for param in self._alg.parameters:
            if param.isAdvanced:
                self.labels[param.name].setVisible(self.showAdvanced)
                self.widgets[param.name].setVisible(self.showAdvanced)

    def getAvailableValuesOfType(self, paramType, outType=None):
        values = []
        inputs = self.model.inputs
        for i in inputs.values():
            param = i.param
            if isinstance(param, paramType):
                values.append(ValueFromInput(param.name))
        if outType is None:
            return values
        if self._algName is None:
            dependent = []
        else:
            dependent = self.model.getDependentAlgorithms(self._algName)
        for alg in self.model.algs.values():
            if alg.name not in dependent:
                for out in alg.algorithm.outputs:
                    if isinstance(out, outType):
                        values.append(ValueFromOutput(alg.name, out.name))

        return values

    def resolveValueDescription(self, value):
        if isinstance(value, ValueFromInput):
            return self.model.inputs[value.name].param.description
        else:
            alg = self.model.algs[value.alg]
            return self.tr("'%s' from algorithm '%s'") % (alg.algorithm.getOutputFromName(value.output).description, alg.description)


    def getWidgetFromParameter(self, param):
        if isinstance(param, ParameterRaster):
            item = QComboBox()
            layers = self.getAvailableValuesOfType(ParameterRaster, OutputRaster)
            if param.optional:
                item.addItem(self.NOT_SELECTED, None)
            for layer in layers:
                item.addItem(self.resolveValueDescription(layer), layer)
        elif isinstance(param, ParameterVector):
            item = QComboBox()
            layers = self.getAvailableValuesOfType(ParameterVector, OutputVector)
            if param.optional:
                item.addItem(self.NOT_SELECTED, None)
            for layer in layers:
                item.addItem(self.resolveValueDescription(layer), layer)
        elif isinstance(param, ParameterTable):
            item = QComboBox()
            tables = self.getAvailableValuesOfType(ParameterTable, OutputTable)
            layers = self.getAvailableValuesOfType(ParameterVector, OutputVector)
            if param.optional:
                item.addItem(self.NOT_SELECTED, None)
            for table in tables:
                item.addItem(self.resolveValueDescription(table), table)
            for layer in layers:
                item.addItem(self.resolveValueDescription(layer), layer)
        elif isinstance(param, ParameterBoolean):
            item = QComboBox()
            item.addItem('Yes')
            item.addItem('No')
            bools = self.getAvailableValuesOfType(ParameterBoolean, None)
            for b in bools:
                item.addItem(self.resolveValueDescription(b), b)
        elif isinstance(param, ParameterSelection):
            item = QComboBox()
            item.addItems(param.options)
        elif isinstance(param, ParameterFixedTable):
            item = FixedTablePanel(param)
        elif isinstance(param, ParameterRange):
            item = RangePanel(param)
        elif isinstance(param, ParameterMultipleInput):
            if param.datatype == ParameterMultipleInput.TYPE_VECTOR_ANY:
                options = self.getAvailableValuesOfType(ParameterVector, OutputVector)
            else:
                options = self.getAvailableValuesOfType(ParameterRaster, OutputRaster)
            opts = []
            for opt in options:
                opts.append(self.resolveValueDescription(opt))
            item = MultipleInputPanel(opts)
        elif isinstance(param, ParameterString):
            strings = self.getAvailableValuesOfType(ParameterString, OutputString)
            options = [(self.resolveValueDescription(s), s) for s in strings]
            if param.multiline:
                item = MultilineTextPanel(options)
                item.setText(unicode(param.default))
            else:
                item = QComboBox()
                item.setEditable(True)
                for desc, val in options:
                    item.addItem(desc, val)
                item.setEditText(unicode(param.default))
        elif isinstance(param, ParameterTableField):
            item = QComboBox()
            item.setEditable(True)
            fields = self.getAvailableValuesOfType(ParameterTableField, None)
            for f in fields:
                item.addItem(self.resolveValueDescription(f), f)
        elif isinstance(param, ParameterNumber):
            item = QComboBox()
            item.setEditable(True)
            numbers = self.getAvailableValuesOfType(ParameterNumber, OutputNumber)
            for n in numbers:
                item.addItem(self.resolveValueDescription(n), n)
            item.setEditText(str(param.default))
        elif isinstance(param, ParameterCrs):
            item = CrsSelectionPanel(param.default)
        elif isinstance(param, ParameterExtent):
            item = QComboBox()
            item.setEditable(True)
            extents = self.getAvailableValuesOfType(ParameterExtent, OutputExtent)
            if self.canUseAutoExtent():
                item.addItem(self.USE_MIN_COVERING_EXTENT, None)
            for ex in extents:
                item.addItem(self.resolveValueDescription(ex), ex)
            if not self.canUseAutoExtent():
                item.setEditText(str(param.default))
        elif isinstance(param, ParameterFile):
            item = QComboBox()
            item.setEditable(True)
            files = self.getAvailableValuesOfType(ParameterFile, OutputFile)
            for f in files:
                item.addItem(self.resolveValueDescription(f), f)
        else:
            item = QLineEdit()
            try:
                item.setText(str(param.default))
            except:
                pass
        return item

    def canUseAutoExtent(self):
        for param in self._alg.parameters:
            if isinstance(param, (ParameterRaster, ParameterVector, ParameterMultipleInput)):
                return True
        return False

    def setTableContent(self):
        params = self._alg.parameters
        outputs = self._alg.outputs
        visibleParams = [p for p in params if not p.hidden]
        visibleOutputs = [p for o in outputs if not o.hidden]
        self.tableWidget.setRowCount(len(visibleParams) + len(visibleOutputs))

        for i, param in visibleParams:
            item = QTableWidgetItem(param.description)
            item.setFlags(Qt.ItemIsEnabled)
            self.tableWidget.setItem(i, 0, item)
            item = self.getWidgetFromParameter(param)
            self.valueItems[param.name] = item
            self.tableWidget.setCellWidget(i, 1, item)
            self.tableWidget.setRowHeight(i, 22)

        for i, output in visibleOutputs:
            item = QTableWidgetItem(output.description + '<'
                    + output.__module__.split('.')[-1] + '>')
            item.setFlags(Qt.ItemIsEnabled)
            self.tableWidget.setItem(i, 0, item)
            item = QLineEdit()
            if hasattr(item, 'setPlaceholderText'):
                item.setPlaceholderText(ModelerParametersDialog.ENTER_NAME)
            self.valueItems[output.name] = item
            self.tableWidget.setCellWidget(i, 1, item)
            self.tableWidget.setRowHeight(i, 22)

    def setComboBoxValue(self, combo, value, param):
        if isinstance(value, list):
            value = value[0]
        items = [combo.itemData(i) for i in range(combo.count())]
        try:
            idx = items.index(value)
            combo.setCurrentIndex(idx)
            return
        except ValueError:
            pass
        if combo.isEditable():
            if value is not None:
                combo.setEditText(unicode(value))
        elif isinstance(param, ParameterSelection):
            combo.setCurrentIndex(int(value))
        elif isinstance(param, ParameterBoolean):
            if value:
                combo.setCurrentIndex(0)
            else:
                combo.setCurrentIndex(1)

    def setPreviousValues(self):
        if self._algName is not None:
            alg = self.model.algs[self._algName]
            self.descriptionBox.setText(alg.description)
            for param in alg.algorithm.parameters:
                if param.hidden:
                    continue
                widget = self.valueItems[param.name]
                value = alg.params[param.name]
                if isinstance(param, (
                        ParameterRaster,
                        ParameterVector,
                        ParameterTable,
                        ParameterTableField,
                        ParameterSelection,
                        ParameterNumber,
                        ParameterBoolean,
                        ParameterExtent,
                        ParameterFile
                )):
                    self.setComboBoxValue(widget, value, param)
                elif isinstance(param, ParameterString):
                    if param.multiline:
                        widget.setValue(value)
                    else:
                        self.setComboBoxValue(widget, value, param)
                elif isinstance(param, ParameterCrs):
                    widget.setAuthId(value)
                elif isinstance(param, ParameterFixedTable):
                    pass  # TODO!
                elif isinstance(param, ParameterMultipleInput):
                    if param.datatype == ParameterMultipleInput.TYPE_VECTOR_ANY:
                        options = self.getAvailableValuesOfType(ParameterVector, OutputVector)
                    else:
                        options = self.getAvailableValuesOfType(ParameterRaster, OutputRaster)
                    selected = []
                    for i, opt in enumerate(options):
                        if opt in value:
                            selected.append(i)
                    widget.setSelectedItems(selected)

            for name, out in alg.outputs.iteritems():
                widget = self.valueItems[name].setText(out.description)

            selected = []
            dependencies = self.getAvailableDependencies()
            for idx, dependency in enumerate(dependencies):
                if dependency in alg.dependencies:
                    selected.append(idx)

            self.dependenciesPanel.setSelectedItems(selected)

    def createAlgorithm(self):
        alg = Algorithm(self._alg.commandLineName())
        alg.setName(self.model)
        alg.description = self.descriptionBox.text()
        params = self._alg.parameters
        outputs = self._alg.outputs
        for param in params:
            if param.hidden:
                continue
            if not self.setParamValue(alg, param, self.valueItems[param.name]):
                return None
        for output in outputs:
            if not output.hidden:
                name = unicode(self.valueItems[output.name].text())
                if name.strip() != '' and name != ModelerParametersDialog.ENTER_NAME:
                    alg.outputs[output.name] = ModelerOutput(name)

        selectedOptions = self.dependenciesPanel.selectedoptions
        availableDependencies = self.getAvailableDependencies()
        for selected in selectedOptions:
            alg.dependencies.append(availableDependencies[selected].name)

        return alg

    def setParamValueLayerOrTable(self, alg, param, widget):
        idx = widget.currentIndex()
        if idx < 0:
            return False
        else:
            value = widget.itemData(widget.currentIndex())
            alg.params[param.name] = value
            return True


    def setParamTableFieldValue(self, alg, param, widget):
        idx = widget.findText(widget.currentText())
        if idx < 0:
            s = str(widget.currentText()).strip()
            if s == '':
                if param.optional:
                    alg.params[param.name] = None
                    return True
                else:
                    return False
            else:
                alg.params[param.name] = s
                return True
        else:
            alg.params[param.name] = widget.itemData(widget.currentIndex())
        return True

    def setParamStringValue(self, alg, param, widget):
        if param.multiline:
            value = widget.getValue()
            option = widget.getOption()
            if option == MultilineTextPanel.USE_TEXT:
                if value == '':
                    if param.optional:
                        alg.params[param.name] = None
                        return True
                    else:
                        return False
                else:
                    alg.params[param.name] = value
            else:
                alg.params[param.name] = value
        else:
            idx = widget.findText(widget.currentText())
            if idx < 0:
                value = widget.currentText().strip()
                if value == '':
                    if param.optional:
                        alg.params[param.name] = None
                        return True
                    else:
                        return False
                else:
                    alg.params[param.name] = value
            else:
                alg.params[param.name] = widget.itemData(widget.currentIndex())
        return True

    def setParamFileValue(self, alg, param, widget):
        idx = widget.findText(widget.currentText())
        if idx < 0:
            value = widget.currentText()
        else:
            value = widget.itemData(widget.currentIndex())
        alg.params[param.name] = value
        return True

    def setParamNumberValue(self, alg, param, widget):
        idx = widget.findText(widget.currentText())
        if idx < 0:
            s = widget.currentText()
            try:
                value = float(s)
            except:
                return False
        else:
            value = widget.itemData(widget.currentIndex())
        alg.params[param.name] = value
        return True

    def setParamExtentValue(self, alg, param, widget):
        idx = widget.findText(widget.currentText())
        if idx < 0:
            s = str(widget.currentText())
            try:
                tokens = s.split(',')
                if len(tokens) != 4:
                    return False
                for token in tokens:
                    float(token)
            except:
                return False
            alg.params[param.name] = [s]
        else:
            value = widget.itemData(widget.currentIndex())
            alg.params[param.name] = value
        return True

    def setParamValue(self, alg, param, widget):
        if isinstance(param, (ParameterRaster, ParameterVector,
                      ParameterTable)):
            return self.setParamValueLayerOrTable(alg, param, widget)
        elif isinstance(param, ParameterBoolean):
            if widget.currentIndex() < 2:
                value = widget.currentIndex() == 0
            else:
                value = widget.itemData(widget.currentIndex())
            alg.params[param.name] = value
            return True
        elif isinstance(param, ParameterString):
            return self.setParamStringValue(alg, param, widget)
        elif isinstance(param, ParameterNumber):
            return self.setParamNumberValue(alg, param, widget)
        elif isinstance(param, ParameterExtent):
            return self.setParamExtentValue(alg, param, widget)
        elif isinstance(param, ParameterFile):
            return self.setParamFileValue(alg, param, widget)
        elif isinstance(param, ParameterSelection):
            alg.params[param.name] = widget.currentIndex()
            return True
        elif isinstance(param, ParameterRange):
            alg.params[param.name] = widget.getValue()
            return True
        elif isinstance(param, ParameterCrs):
            authid = widget.getValue()
            if authid is None:
                alg.params[param.name] = None
            else:
                alg.params[param.name] = authid
            return True
        elif isinstance(param, ParameterFixedTable):
            alg.params[param.name] = ParameterFixedTable.tableToString(widget.table)
            return True
        elif isinstance(param, ParameterTableField):
            return self.setParamTableFieldValue(alg, param, widget)
        elif isinstance(param, ParameterMultipleInput):
            if param.datatype == ParameterMultipleInput.TYPE_VECTOR_ANY:
                options = self.getAvailableValuesOfType(ParameterVector, OutputVector)
            else:
                options = self.getAvailableValuesOfType(ParameterRaster, OutputRaster)
            values = [options[i] for i in widget.selectedoptions]
            if len(values) == 0 and not param.optional:
                return False
            alg.params[param.name] = values
            return True
        else:
            alg.params[param.name] = unicode(widget.text())
            return True

    def okPressed(self):
        self.alg = self.createAlgorithm()
        if self.alg is not None:
            self.close()
        else:
            QMessageBox.warning(self, self.tr('Unable to add algorithm'),
                                self.tr('Wrong or missing parameter values'))

    def cancelPressed(self):
        self.alg = None
        self.close()
