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
from builtins import range


__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.PyQt.QtCore import Qt, QUrl, QMetaObject
from qgis.PyQt.QtWidgets import (QDialog, QDialogButtonBox, QLabel, QLineEdit,
                                 QFrame, QPushButton, QSizePolicy, QVBoxLayout,
                                 QHBoxLayout, QTabWidget, QWidget, QScrollArea,
                                 QComboBox, QTableWidgetItem, QMessageBox,
                                 QTextBrowser, QToolButton, QMenu, QAction)
from qgis.PyQt.QtNetwork import QNetworkRequest, QNetworkReply

from qgis.core import QgsApplication, QgsNetworkAccessManager

from qgis.gui import QgsMessageBar

from processing.gui.wrappers import InvalidParameterValue
from processing.gui.MultipleInputPanel import MultipleInputPanel
from processing.core.outputs import (OutputRaster,
                                     OutputVector,
                                     OutputTable,
                                     OutputHTML,
                                     OutputFile,
                                     OutputDirectory,
                                     OutputNumber,
                                     OutputString,
                                     OutputExtent,
                                     OutputCrs)
from processing.core.parameters import ParameterPoint, ParameterExtent

from processing.modeler.ModelerAlgorithm import (ValueFromInput,
                                                 ValueFromOutput,
                                                 Algorithm,
                                                 ModelerOutput)


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
        self.buttonBox.setStandardButtons(QDialogButtonBox.Cancel
                                          | QDialogButtonBox.Ok)
        tooltips = self._alg.getParameterDescriptions()
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
                self.advancedButton.clicked.connect(
                    self.showAdvancedParametersClicked)
                advancedButtonHLayout = QHBoxLayout()
                advancedButtonHLayout.addWidget(self.advancedButton)
                advancedButtonHLayout.addStretch()
                self.verticalLayout.addLayout(advancedButtonHLayout)
                break
        for param in self._alg.parameters:
            if param.hidden:
                continue
            desc = param.description
            if isinstance(param, ParameterExtent):
                desc += self.tr('(xmin, xmax, ymin, ymax)')
            if isinstance(param, ParameterPoint):
                desc += self.tr('(x, y)')
            if param.optional:
                desc += self.tr(' [optional]')
            label = QLabel(desc)
            self.labels[param.name] = label

            wrapper = param.wrapper(self)
            self.wrappers[param.name] = wrapper

            widget = wrapper.widget
            if widget is not None:
                self.valueItems[param.name] = widget
                if param.name in list(tooltips.keys()):
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

        self.txtHelp = QTextBrowser()

        html = None
        isText, algHelp = self._alg.help()
        if algHelp is not None:
            algHelp = algHelp if isText else QUrl(algHelp)
            try:
                if isText:
                    self.txtHelp.setHtml(algHelp)
                else:
                    html = self.tr('<p>Downloading algorithm help... Please wait.</p>')
                    self.txtHelp.setHtml(html)
                    self.tabWidget.addTab(self.txtHelp, 'Help')
                    self.reply = QgsNetworkAccessManager.instance().get(QNetworkRequest(algHelp))
                    self.reply.finished.connect(self.requestFinished)
            except:
                pass

        self.verticalLayout2.addWidget(self.tabWidget)
        self.verticalLayout2.addWidget(self.buttonBox)
        self.setLayout(self.verticalLayout2)
        self.buttonBox.accepted.connect(self.okPressed)
        self.buttonBox.rejected.connect(self.cancelPressed)
        QMetaObject.connectSlotsByName(self)

        for wrapper in list(self.wrappers.values()):
            wrapper.postInitialize(list(self.wrappers.values()))

    def requestFinished(self):
        """Change the webview HTML content"""
        reply = self.sender()
        if reply.error() != QNetworkReply.NoError:
            html = self.tr('<h2>No help available for this algorithm</h2><p>{}</p>'.format(reply.errorString()))
        else:
            html = str(reply.readAll())
        reply.deleteLater()
        self.txtHelp.setHtml(html)

    def getAvailableDependencies(self):  # spellok
        if self._algName is None:
            dependent = []
        else:
            dependent = self.model.getDependentAlgorithms(self._algName)
        opts = []
        for alg in list(self.model.algs.values()):
            if alg.name not in dependent:
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
        for param in self._alg.parameters:
            if param.isAdvanced:
                self.labels[param.name].setVisible(self.showAdvanced)
                self.widgets[param.name].setVisible(self.showAdvanced)

    def getAvailableValuesOfType(self, paramType, outType=None, dataType=None):
        # upgrade paramType to list
        if type(paramType) is not list:
            paramType = [paramType]

        values = []
        inputs = self.model.inputs
        for i in list(inputs.values()):
            param = i.param
            for t in paramType:
                if isinstance(param, t):
                    if dataType is not None:
                        if param.datatype in dataType:
                            values.append(ValueFromInput(param.name))
                    else:
                        values.append(ValueFromInput(param.name))
                    break
        if outType is None:
            return values
        if self._algName is None:
            dependent = []
        else:
            dependent = self.model.getDependentAlgorithms(self._algName)
        for alg in list(self.model.algs.values()):
            if alg.name not in dependent:
                for out in alg.algorithm.outputs:
                    if isinstance(out, outType):
                        if dataType is not None and out.datatype in dataType:
                            values.append(ValueFromOutput(alg.name, out.name))
                        else:
                            values.append(ValueFromOutput(alg.name, out.name))

        return values

    def resolveValueDescription(self, value):
        if isinstance(value, ValueFromInput):
            return self.model.inputs[value.name].param.description
        else:
            alg = self.model.algs[value.alg]
            return self.tr("'%s' from algorithm '%s'") % (alg.algorithm.getOutputFromName(value.output).description, alg.description)

    def setPreviousValues(self):
        if self._algName is not None:
            alg = self.model.algs[self._algName]
            self.descriptionBox.setText(alg.description)
            for param in alg.algorithm.parameters:
                if param.hidden:
                    continue
                if param.name in alg.params:
                    value = alg.params[param.name]
                else:
                    value = param.default
                self.wrappers[param.name].setValue(value)
            for name, out in list(alg.outputs.items()):
                self.valueItems[name].setText(out.description)

            selected = []
            dependencies = self.getAvailableDependencies()  # spellok
            for idx, dependency in enumerate(dependencies):
                if dependency.name in alg.dependencies:
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
            if not self.setParamValue(alg, param, self.wrappers[param.name]):
                self.bar.pushMessage("Error", "Wrong or missing value for parameter '%s'" % param.description,
                                     level=QgsMessageBar.WARNING)
                return None
        for output in outputs:
            if not output.hidden:
                name = str(self.valueItems[output.name].text())
                if name.strip() != '' and name != ModelerParametersDialog.ENTER_NAME:
                    alg.outputs[output.name] = ModelerOutput(name)

        selectedOptions = self.dependenciesPanel.selectedoptions
        availableDependencies = self.getAvailableDependencies()  # spellok
        for selected in selectedOptions:
            alg.dependencies.append(availableDependencies[selected].name)  # spellok

        self._alg.processBeforeAddingToModeler(alg, self.model)
        return alg

    def setParamValue(self, alg, param, wrapper):
        try:
            if wrapper.widget:
                value = wrapper.value()
                alg.params[param.name] = value
            return True
        except InvalidParameterValue:
            return False

    def okPressed(self):
        self.alg = self.createAlgorithm()
        if self.alg is not None:
            self.close()

    def cancelPressed(self):
        self.alg = None
        self.close()
