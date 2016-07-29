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

from processing.gui.FileSelectionPanel import FileSelectionPanel
from processing.gui.CrsSelectionPanel import CrsSelectionPanel
from processing.gui.ExtentSelectionPanel import ExtentSelectionPanel
from processing.gui.FixedTablePanel import FixedTablePanel
from processing.gui.PointSelectionPanel import PointSelectionPanel
from processing.gui.BatchInputSelectionPanel import BatchInputSelectionPanel
from processing.gui.BatchOutputSelectionPanel import BatchOutputSelectionPanel
from processing.gui.GeometryPredicateSelectionPanel import GeometryPredicateSelectionPanel

from processing.core.parameters import ParameterFile
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterTable
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterExtent
from processing.core.parameters import ParameterCrs
from processing.core.parameters import ParameterPoint
from processing.core.parameters import ParameterBoolean
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

    def getWidgetFromParameter(self, param, row, col):
        if isinstance(param, (ParameterRaster, ParameterVector, ParameterTable,
                              ParameterMultipleInput)):
            item = BatchInputSelectionPanel(param, row, col, self)
        elif isinstance(param, ParameterBoolean):
            item = QComboBox()
            item.addItem(self.tr('Yes'))
            item.addItem(self.tr('No'))
            if param.default:
                item.setCurrentIndex(0)
            else:
                item.setCurrentIndex(1)
        elif isinstance(param, ParameterSelection):
            item = QComboBox()
            item.addItems(param.options)
        elif isinstance(param, ParameterFixedTable):
            item = FixedTablePanel(param)
        elif isinstance(param, ParameterExtent):
            item = ExtentSelectionPanel(self.parent, self.alg, param.default)
        elif isinstance(param, ParameterPoint):
            item = PointSelectionPanel(self.parent, param.default)
        elif isinstance(param, ParameterCrs):
            item = CrsSelectionPanel(param.default)
        elif isinstance(param, ParameterFile):
            item = FileSelectionPanel(param.isFolder)
        elif isinstance(param, ParameterGeometryPredicate):
            item = GeometryPredicateSelectionPanel(param.enabledPredicates, rows=1)
            width = max(self.tblParameters.columnWidth(col),
                        item.sizeHint().width())
            self.tblParameters.setColumnWidth(col, width)
        else:
            item = QLineEdit()
            try:
                item.setText(unicode(param.default))
            except:
                pass

        return item

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
                    widget = self.tblParameters.cellWidget(row, column)
                    if param.name in params:
                        value = params[param.name]
                        self.setValueInWidget(widget, value)
                    column += 1

                for out in self.alg.outputs:
                    if out.hidden:
                        continue
                    widget = self.tblParameters.cellWidget(row, column)
                    if out.name in outputs:
                        value = outputs[out.name]
                        self.setValueInWidget(widget, value)
                    column += 1
        except TypeError:
            QMessageBox.critical(
                self,
                self.tr('Error'),
                self.tr('An error occurred while reading your file.'))

    def setValueInWidget(self, widget, value):
        if isinstance(widget, (BatchInputSelectionPanel, QLineEdit, FileSelectionPanel)):
            widget.setText(unicode(value))
        elif isinstance(widget, (BatchOutputSelectionPanel, GeometryPredicateSelectionPanel)):
            widget.setValue(unicode(value))

        elif isinstance(widget, QComboBox):
            idx = widget.findText(unicode(value))
            if idx != -1:
                widget.setCurrentIndex(idx)
        elif isinstance(widget, ExtentSelectionPanel):
            if value is not None:
                widget.setExtentFromString(value)
            else:
                widget.setExtentFromString('')
        elif isinstance(widget, CrsSelectionPanel):
            widget.setAuthId(value)

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
                if isinstance(param, ParameterExtent):
                    col += 1
                    continue
                widget = self.tblParameters.cellWidget(row, col)
                if not self.setParamValue(param, widget, alg):
                    self.parent.lblProgress.setText(
                        self.tr('<b>Missing parameter value: %s (row %d)</b>') % (param.description, row + 1))
                    return
                algParams[param.name] = param.getValueAsCommandLineParameter()
                col += 1
            col = 0
            for param in alg.parameters:
                if param.hidden:
                    continue
                if isinstance(param, ParameterExtent):
                    widget = self.tblParameters.cellWidget(row, col)
                    if not self.setParamValue(param, widget, alg):
                        self.parent.lblProgress.setText(
                            self.tr('<b>Missing parameter value: %s (row %d)</b>') % (param.description, row + 1))
                        return
                    algParams[param.name] = unicode(param.value())
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
                    self.parent.lblProgress.setText(
                        self.tr('<b>Wrong or missing parameter value: %s (row %d)</b>') % (out.description, row + 1))
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

    def setParamValue(self, param, widget, alg=None):
        if isinstance(param, (ParameterRaster, ParameterVector, ParameterTable,
                              ParameterMultipleInput)):
            value = widget.getText()
            if unicode(value).strip() == '':
                value = None
            return param.setValue(value)
        elif isinstance(param, ParameterBoolean):
            return param.setValue(widget.currentIndex() == 0)
        elif isinstance(param, ParameterSelection):
            return param.setValue(widget.currentIndex())
        elif isinstance(param, ParameterFixedTable):
            return param.setValue(widget.table)
        elif isinstance(param, ParameterExtent):
            if alg is not None:
                widget.useNewAlg(alg)
            return param.setValue(widget.getValue())
        elif isinstance(param, (ParameterCrs, ParameterFile)):
            return param.setValue(widget.getValue())
        elif isinstance(param, ParameterGeometryPredicate):
            return param.setValue(widget.value())
        else:
            return param.setValue(widget.text())

    def addRow(self):
        self.tblParameters.setRowCount(self.tblParameters.rowCount() + 1)

        row = self.tblParameters.rowCount() - 1
        column = 0
        for param in self.alg.parameters:
            if param.hidden:
                continue

            self.tblParameters.setCellWidget(
                row, column, self.getWidgetFromParameter(param, row, column))
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

    def removeRows(self):
        #~ self.tblParameters.setUpdatesEnabled(False)
        #~ indexes = self.tblParameters.selectionModel().selectedIndexes()
        #~ indexes.sort()
        #~ for i in reversed(indexes):
            #~ self.tblParameters.model().removeRow(i.row())
        #~ self.tblParameters.setUpdatesEnabled(True)
        if self.tblParameters.rowCount() > 2:
            self.tblParameters.setRowCount(self.tblParameters.rowCount() - 1)

    def fillParameterValues(self, column):
        widget = self.tblParameters.cellWidget(0, column)

        if isinstance(widget, QComboBox):
            widgetValue = widget.currentIndex()
            for row in range(1, self.tblParameters.rowCount()):
                self.tblParameters.cellWidget(row, column).setCurrentIndex(widgetValue)
        elif isinstance(widget, ExtentSelectionPanel):
            widgetValue = widget.getValue()
            for row in range(1, self.tblParameters.rowCount()):
                if widgetValue is not None:
                    self.tblParameters.cellWidget(row, column).setExtentFromString(widgetValue)
                else:
                    self.tblParameters.cellWidget(row, column).setExtentFromString('')
        elif isinstance(widget, CrsSelectionPanel):
            widgetValue = widget.getValue()
            for row in range(1, self.tblParameters.rowCount()):
                self.tblParameters.cellWidget(row, column).setAuthId(widgetValue)
        elif isinstance(widget, FileSelectionPanel):
            widgetValue = widget.getValue()
            for row in range(1, self.tblParameters.rowCount()):
                self.tblParameters.cellWidget(row, column).setText(widgetValue)
        elif isinstance(widget, QLineEdit):
            widgetValue = widget.text()
            for row in range(1, self.tblParameters.rowCount()):
                self.tblParameters.cellWidget(row, column).setText(widgetValue)
        elif isinstance(widget, BatchInputSelectionPanel):
            widgetValue = widget.getText()
            for row in range(1, self.tblParameters.rowCount()):
                self.tblParameters.cellWidget(row, column).setText(widgetValue)
        elif isinstance(widget, GeometryPredicateSelectionPanel):
            widgetValue = widget.value()
            for row in range(1, self.tblParameters.rowCount()):
                self.tblParameters.cellWidget(row, column).setValue(widgetValue)
        else:
            pass

    def toggleAdvancedMode(self, checked):
        for column, param in enumerate(self.alg.parameters):
            if param.isAdvanced:
                self.tblParameters.setColumnHidden(column, not checked)
