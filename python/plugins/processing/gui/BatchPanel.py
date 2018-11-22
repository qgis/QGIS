# -*- coding: utf-8 -*-

"""
***************************************************************************
    BatchPanel.py
    ---------------------
    Date                 : November 2014
    Copyright            : (C) 2014 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'November 2014'
__copyright__ = '(C) 2014, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import json
import warnings

from qgis.PyQt import uic
from qgis.PyQt.QtWidgets import QTableWidgetItem, QComboBox, QHeaderView, QFileDialog, QMessageBox
from qgis.PyQt.QtCore import QDir, QFileInfo
from qgis.core import (Qgis,
                       QgsApplication,
                       QgsSettings,
                       QgsProcessingParameterDefinition)
from qgis.gui import QgsProcessingParameterWidgetContext
from qgis.utils import iface

from processing.gui.wrappers import WidgetWrapperFactory, WidgetWrapper
from processing.gui.BatchOutputSelectionPanel import BatchOutputSelectionPanel

from processing.tools import dataobjects

pluginPath = os.path.split(os.path.dirname(__file__))[0]

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(
        os.path.join(pluginPath, 'ui', 'widgetBatchPanel.ui'))


class BatchPanel(BASE, WIDGET):

    PARAMETERS = "PARAMETERS"
    OUTPUTS = "OUTPUTS"

    def __init__(self, parent, alg):
        super(BatchPanel, self).__init__(None)
        self.setupUi(self)

        self.wrappers = []

        self.btnAdvanced.hide()

        # Set icons
        self.btnAdd.setIcon(QgsApplication.getThemeIcon('/symbologyAdd.svg'))
        self.btnRemove.setIcon(QgsApplication.getThemeIcon('/symbologyRemove.svg'))
        self.btnOpen.setIcon(QgsApplication.getThemeIcon('/mActionFileOpen.svg'))
        self.btnSave.setIcon(QgsApplication.getThemeIcon('/mActionFileSave.svg'))
        self.btnAdvanced.setIcon(QgsApplication.getThemeIcon("/processingAlgorithm.svg"))

        self.alg = alg
        self.parent = parent

        self.btnAdd.clicked.connect(self.addRow)
        self.btnRemove.clicked.connect(self.removeRows)
        self.btnOpen.clicked.connect(self.load)
        self.btnSave.clicked.connect(self.save)
        self.btnAdvanced.toggled.connect(self.toggleAdvancedMode)
        self.tblParameters.horizontalHeader().sectionDoubleClicked.connect(
            self.fillParameterValues)

        self.tblParameters.horizontalHeader().resizeSections(QHeaderView.ResizeToContents)
        self.tblParameters.horizontalHeader().setDefaultSectionSize(250)
        self.tblParameters.horizontalHeader().setMinimumSectionSize(150)

        self.initWidgets()

    def layerRegistryChanged(self):
        pass

    def initWidgets(self):
        # If there are advanced parameters — show corresponding button
        for param in self.alg.parameterDefinitions():
            if param.flags() & QgsProcessingParameterDefinition.FlagAdvanced:
                self.btnAdvanced.show()
                break

        # Determine column count
        nOutputs = len(self.alg.destinationParameterDefinitions()) + 1
        if nOutputs == 1:
            nOutputs = 0

        self.tblParameters.setColumnCount(
            self.alg.countVisibleParameters())

        # Table headers
        column = 0
        for param in self.alg.parameterDefinitions():
            if param.isDestination():
                continue
            self.tblParameters.setHorizontalHeaderItem(
                column, QTableWidgetItem(param.description()))
            if param.flags() & QgsProcessingParameterDefinition.FlagAdvanced:
                self.tblParameters.setColumnHidden(column, True)
            column += 1

        for out in self.alg.destinationParameterDefinitions():
            if not out.flags() & QgsProcessingParameterDefinition.FlagHidden:
                self.tblParameters.setHorizontalHeaderItem(
                    column, QTableWidgetItem(out.description()))
                column += 1

        # Last column for indicating if output will be added to canvas
        if len(self.alg.destinationParameterDefinitions()) > 0:
            self.tblParameters.setHorizontalHeaderItem(
                column, QTableWidgetItem(self.tr('Load in QGIS')))

        # Add an empty row to begin
        self.addRow()

        self.tblParameters.horizontalHeader().resizeSections(QHeaderView.ResizeToContents)
        self.tblParameters.verticalHeader().setSectionResizeMode(QHeaderView.ResizeToContents)
        self.tblParameters.horizontalHeader().setStretchLastSection(True)

    def load(self):
        context = dataobjects.createContext()
        settings = QgsSettings()
        last_path = settings.value("/Processing/LastBatchPath", QDir.homePath())
        filename, selected_filter = QFileDialog.getOpenFileName(self,
                                                                self.tr('Open Batch'), last_path,
                                                                self.tr('JSON files (*.json)'))
        if filename:
            last_path = QFileInfo(filename).path()
            settings.setValue('/Processing/LastBatchPath', last_path)
            with open(filename) as f:
                values = json.load(f)
        else:
            # If the user clicked on the cancel button.
            return

        self.tblParameters.setRowCount(0)
        try:
            for row, alg in enumerate(values):
                self.addRow()
                params = alg[self.PARAMETERS]
                outputs = alg[self.OUTPUTS]
                column = 0
                for param in self.alg.parameterDefinitions():
                    if param.flags() & QgsProcessingParameterDefinition.FlagHidden:
                        continue
                    if param.isDestination():
                        continue
                    if param.name() in params:
                        value = eval(params[param.name()])
                        wrapper = self.wrappers[row][column]
                        wrapper.setParameterValue(value, context)
                    column += 1

                for out in self.alg.destinationParameterDefinitions():
                    if out.flags() & QgsProcessingParameterDefinition.FlagHidden:
                        continue
                    if out.name() in outputs:
                        value = outputs[out.name()].strip("'")
                        widget = self.tblParameters.cellWidget(row, column)
                        widget.setValue(value)
                    column += 1
        except TypeError:
            QMessageBox.critical(
                self,
                self.tr('Error'),
                self.tr('An error occurred while reading your file.'))

    def save(self):
        toSave = []
        context = dataobjects.createContext()
        for row in range(self.tblParameters.rowCount()):
            algParams = {}
            algOutputs = {}
            col = 0
            alg = self.alg
            for param in alg.parameterDefinitions():
                if param.flags() & QgsProcessingParameterDefinition.FlagHidden:
                    continue
                if param.isDestination():
                    continue
                wrapper = self.wrappers[row][col]

                # For compatibility with 3.x API, we need to check whether the wrapper is
                # the deprecated WidgetWrapper class. If not, it's the newer
                # QgsAbstractProcessingParameterWidgetWrapper class
                # TODO QGIS 4.0 - remove
                if issubclass(wrapper.__class__, WidgetWrapper):
                    widget = wrapper.widget
                else:
                    widget = wrapper.wrappedWidget()

                value = wrapper.parameterValue()

                if not param.checkValueIsAcceptable(value, context):
                    self.parent.messageBar().pushMessage("", self.tr('Wrong or missing parameter value: {0} (row {1})').format(
                        param.description(), row + 1),
                        level=Qgis.Warning, duration=5)
                    return
                algParams[param.name()] = param.valueAsPythonString(value, context)
                col += 1
            for out in alg.destinationParameterDefinitions():
                if out.flags() & QgsProcessingParameterDefinition.FlagHidden:
                    continue
                widget = self.tblParameters.cellWidget(row, col)
                text = widget.getValue()
                if text.strip() != '':
                    algOutputs[out.name()] = text.strip()
                    col += 1
                else:
                    self.parent.messageBar().pushMessage("", self.tr('Wrong or missing output value: {0} (row {1})').format(
                        out.description(), row + 1),
                        level=Qgis.Warning, duration=5)
                    return
            toSave.append({self.PARAMETERS: algParams, self.OUTPUTS: algOutputs})

        settings = QgsSettings()
        last_path = settings.value("/Processing/LastBatchPath", QDir.homePath())
        filename, __ = QFileDialog.getSaveFileName(self,
                                                   self.tr('Save Batch'),
                                                   last_path,
                                                   self.tr('JSON files (*.json)'))
        if filename:
            if not filename.endswith('.json'):
                filename += '.json'
            last_path = QFileInfo(filename).path()
            settings.setValue('/Processing/LastBatchPath', last_path)
            with open(filename, 'w') as f:
                json.dump(toSave, f)

    def setCellWrapper(self, row, column, wrapper, context):
        self.wrappers[row][column] = wrapper

        # For compatibility with 3.x API, we need to check whether the wrapper is
        # the deprecated WidgetWrapper class. If not, it's the newer
        # QgsAbstractProcessingParameterWidgetWrapper class
        # TODO QGIS 4.0 - remove
        is_cpp_wrapper = not issubclass(wrapper.__class__, WidgetWrapper)
        if is_cpp_wrapper:
            widget_context = QgsProcessingParameterWidgetContext()
            if iface is not None:
                widget_context.setMapCanvas(iface.mapCanvas())
            wrapper.setWidgetContext(widget_context)
            widget = wrapper.createWrappedWidget(context)
        else:
            widget = wrapper.widget

        self.tblParameters.setCellWidget(row, column, widget)

    def addRow(self):
        self.wrappers.append([None] * self.tblParameters.columnCount())
        self.tblParameters.setRowCount(self.tblParameters.rowCount() + 1)

        context = dataobjects.createContext()

        wrappers = {}
        row = self.tblParameters.rowCount() - 1
        column = 0
        for param in self.alg.parameterDefinitions():
            if param.flags() & QgsProcessingParameterDefinition.FlagHidden or param.isDestination():
                continue

            wrapper = WidgetWrapperFactory.create_wrapper(param, self.parent, row, column)
            wrappers[param.name()] = wrapper
            self.setCellWrapper(row, column, wrapper, context)
            column += 1

        for out in self.alg.destinationParameterDefinitions():
            if out.flags() & QgsProcessingParameterDefinition.FlagHidden:
                continue

            self.tblParameters.setCellWidget(
                row, column, BatchOutputSelectionPanel(
                    out, self.alg, row, column, self))
            column += 1

        if len(self.alg.destinationParameterDefinitions()) > 0:
            item = QComboBox()
            item.addItem(self.tr('Yes'))
            item.addItem(self.tr('No'))
            item.setCurrentIndex(0)
            self.tblParameters.setCellWidget(row, column, item)

        for wrapper in list(wrappers.values()):
            wrapper.postInitialize(list(wrappers.values()))

    def removeRows(self):
        if self.tblParameters.rowCount() > 1:
            self.wrappers.pop()
            self.tblParameters.setRowCount(self.tblParameters.rowCount() - 1)

    def fillParameterValues(self, column):
        context = dataobjects.createContext()

        wrapper = self.wrappers[0][column]
        if wrapper is None:
            # e.g. double clicking on a destination header
            return

        for row in range(1, self.tblParameters.rowCount()):
            self.wrappers[row][column].setParameterValue(wrapper.parameterValue(), context)

    def toggleAdvancedMode(self, checked):
        for column, param in enumerate(self.alg.parameterDefinitions()):
            if param.flags() & QgsProcessingParameterDefinition.FlagAdvanced:
                self.tblParameters.setColumnHidden(column, not checked)
