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

from qgis.PyQt import uic
from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtWidgets import QTableWidgetItem, QComboBox, QLineEdit, QHeaderView, QFileDialog, QMessageBox

from qgis.core import QgsApplication
from qgis.gui import QgsMessageBar

from processing.gui.BatchOutputSelectionPanel import BatchOutputSelectionPanel
from processing.gui.GeometryPredicateSelectionPanel import GeometryPredicateSelectionPanel

from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterTable
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterExtent
from processing.core.parameters import ParameterCrs
from processing.core.parameters import ParameterPoint
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterFixedTable
from processing.core.parameters import ParameterMultipleInput
from processing.core.parameters import ParameterGeometryPredicate

pluginPath = os.path.split(os.path.dirname(__file__))[0]
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
        self.btnAdvanced.setIcon(QIcon(os.path.join(pluginPath, 'images', 'alg.png')))

        self.alg = alg
        self.parent = parent

        self.btnAdd.clicked.connect(self.addRow)
        self.btnRemove.clicked.connect(self.removeRows)
        self.btnOpen.clicked.connect(self.load)
        self.btnSave.clicked.connect(self.save)
        self.btnAdvanced.toggled.connect(self.toggleAdvancedMode)
        self.tblParameters.horizontalHeader().sectionDoubleClicked.connect(
            self.fillParameterValues)

        self.initWidgets()

    def layerRegistryChanged(self):
        pass

    def initWidgets(self):
        # If there are advanced parameters — show corresponding button
        for param in self.alg.parameters:
            if param.isAdvanced:
                self.btnAdvanced.show()
                break

        # Determine column count
        nOutputs = self.alg.getVisibleOutputsCount() + 1
        if nOutputs == 1:
            nOutputs = 0

        self.tblParameters.setColumnCount(
            self.alg.getVisibleParametersCount() + nOutputs)

        # Table headers
        column = 0
        for param in self.alg.parameters:
            self.tblParameters.setHorizontalHeaderItem(
                column, QTableWidgetItem(param.description))
            if param.isAdvanced:
                self.tblParameters.setColumnHidden(column, True)
            column += 1

        for out in self.alg.outputs:
            if not out.hidden:
                self.tblParameters.setHorizontalHeaderItem(
                    column, QTableWidgetItem(out.description))
                column += 1

        # Last column for indicating if output will be added to canvas
        if self.alg.getVisibleOutputsCount():
            self.tblParameters.setHorizontalHeaderItem(
                column, QTableWidgetItem(self.tr('Load in QGIS')))

        # Add three empty rows by default
        for i in xrange(3):
            self.addRow()

        self.tblParameters.horizontalHeader().setResizeMode(QHeaderView.Interactive)
        self.tblParameters.horizontalHeader().setDefaultSectionSize(250)
        self.tblParameters.horizontalHeader().setMinimumSectionSize(150)
        self.tblParameters.horizontalHeader().setResizeMode(QHeaderView.ResizeToContents)
        self.tblParameters.verticalHeader().setResizeMode(QHeaderView.ResizeToContents)
        self.tblParameters.horizontalHeader().setStretchLastSection(True)

    def load(self):
        filename = unicode(QFileDialog.getOpenFileName(self,
                                                       self.tr('Open batch'), None,
                                                       self.tr('JSON files (*.json)')))
        if filename:
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
                for param in self.alg.parameters:
                    if param.hidden:
                        continue
                    if param.name in params:
                        value = params[param.name]
                        wrapper = self.wrappers[row][column]
                        wrapper.setValue(value)
                    column += 1

                for out in self.alg.outputs:
                    if out.hidden:
                        continue
                    if out.name in outputs:
                        value = outputs[out.name]
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
        for row in range(self.tblParameters.rowCount()):
            algParams = {}
            algOutputs = {}
            col = 0
            alg = self.alg.getCopy()
            for param in alg.parameters:
                if param.hidden:
                    continue
                wrapper = self.wrappers[row][col]
                if not self.setParamValue(param, wrapper, alg):
                    self.parent.bar.pushMessage("", self.tr('Wrong or missing parameter value: %s (row %d)')
                                                % (param.description, row + 1),
                                                level=QgsMessageBar.WARNING, duration=5)
                    return
                algParams[param.name] = param.getValueAsCommandLineParameter()
                col += 1
            for out in alg.outputs:
                if out.hidden:
                    continue
                widget = self.tblParameters.cellWidget(row, col)
                text = widget.getValue()
                if text.strip() != '':
                    algOutputs[out.name] = text.strip()
                    col += 1
                else:
                    self.parent.bar.pushMessage("", self.tr('Wrong or missing output value: %s (row %d)')
                                                % (out.description, row + 1),
                                                level=QgsMessageBar.WARNING, duration=5)
                    return
            toSave.append({self.PARAMETERS: algParams, self.OUTPUTS: algOutputs})

        filename = unicode(QFileDialog.getSaveFileName(self,
                                                       self.tr('Save batch'),
                                                       None,
                                                       self.tr('JSON files (*.json)')))
        if filename:
            if not filename.endswith('.json'):
                filename += '.json'
            with open(filename, 'w') as f:
                json.dump(toSave, f)

    def setParamValue(self, param, wrapper, alg=None):
        return param.setValue(wrapper.value())

    def setCellWrapper(self, row, column, wrapper):
        self.wrappers[row][column] = wrapper
        self.tblParameters.setCellWidget(row, column, wrapper.widget)

    def addRow(self):
        self.wrappers.append([None] * self.tblParameters.columnCount())
        self.tblParameters.setRowCount(self.tblParameters.rowCount() + 1)

        wrappers = {}
        row = self.tblParameters.rowCount() - 1
        column = 0
        for param in self.alg.parameters:
            if param.hidden:
                continue

            wrapper = param.wrapper(self.parent, row, column)
            wrappers[param.name] = wrapper
            self.setCellWrapper(row, column, wrapper)
            column += 1

        for out in self.alg.outputs:
            if out.hidden:
                continue

            self.tblParameters.setCellWidget(
                row, column, BatchOutputSelectionPanel(
                    out, self.alg, row, column, self))
            column += 1

        if self.alg.getVisibleOutputsCount():
            item = QComboBox()
            item.addItem(self.tr('Yes'))
            item.addItem(self.tr('No'))
            item.setCurrentIndex(0)
            self.tblParameters.setCellWidget(row, column, item)

        for wrapper in wrappers.values():
            wrapper.postInitialize(wrappers.values())

    def removeRows(self):
        if self.tblParameters.rowCount() > 2:
            self.wrappers.pop()
            self.tblParameters.setRowCount(self.tblParameters.rowCount() - 1)

    def fillParameterValues(self, column):
        wrapper = self.wrappers[0][column]
        for row in range(1, self.tblParameters.rowCount()):
            self.wrappers[row][column].setValue(wrapper.value())

    def toggleAdvancedMode(self, checked):
        for column, param in enumerate(self.alg.parameters):
            if param.isAdvanced:
                self.tblParameters.setColumnHidden(column, not checked)
