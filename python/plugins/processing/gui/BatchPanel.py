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

import os
import json
import warnings

from qgis.PyQt import uic
from qgis.PyQt.QtWidgets import (
    QTableWidgetItem,
    QComboBox,
    QHeaderView,
    QFileDialog,
    QMessageBox,
    QToolButton,
    QMenu,
    QAction
)
from qgis.PyQt.QtGui import QPalette
from qgis.PyQt.QtCore import (
    QDir,
    QFileInfo,
    QCoreApplication
)
from qgis.core import (
    Qgis,
    QgsApplication,
    QgsSettings,
    QgsProperty,  # NOQA - must be here for saved file evaluation
    QgsProject,
    QgsProcessingFeatureSourceDefinition,  # NOQA - must be here for saved file evaluation
    QgsCoordinateReferenceSystem,  # NOQA - must be here for saved file evaluation
    QgsProcessingParameterDefinition,
    QgsProcessingModelAlgorithm,
    QgsProcessingParameterFile,
    QgsProcessingParameterMapLayer,
    QgsProcessingParameterRasterLayer,
    QgsProcessingParameterMeshLayer,
    QgsProcessingParameterVectorLayer,
    QgsProcessingParameterFeatureSource,
    QgsProcessingParameterRasterDestination,
    QgsProcessingParameterVectorDestination,
    QgsProcessingParameterFeatureSink,
    QgsProcessingOutputLayerDefinition,
    QgsExpressionContextUtils,
    QgsExpression
)
from qgis.gui import (
    QgsProcessingParameterWidgetContext,
    QgsProcessingContextGenerator,
    QgsFindFilesByPatternDialog,
    QgsExpressionBuilderDialog
)
from qgis.utils import iface

from processing.gui.wrappers import WidgetWrapperFactory, WidgetWrapper
from processing.gui.BatchOutputSelectionPanel import BatchOutputSelectionPanel

from processing.tools import dataobjects
from processing.tools.dataobjects import createContext

pluginPath = os.path.split(os.path.dirname(__file__))[0]

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(
        os.path.join(pluginPath, 'ui', 'widgetBatchPanel.ui'))


class BatchPanelFillWidget(QToolButton):

    def __init__(self, parameterDefinition, column, panel, parent=None):
        super().__init__(parent)

        self.setBackgroundRole(QPalette.Button)
        self.setAutoFillBackground(True)

        self.parameterDefinition = parameterDefinition
        self.column = column
        self.panel = panel

        self.setText(QCoreApplication.translate('BatchPanel', 'Autofill…'))
        f = self.font()
        f.setItalic(True)
        self.setFont(f)
        self.setPopupMode(QToolButton.InstantPopup)
        self.setAutoRaise(True)

        self.menu = QMenu()
        self.menu.aboutToShow.connect(self.createMenu)
        self.setMenu(self.menu)

    def createMenu(self):
        self.menu.clear()
        self.menu.setMinimumWidth(self.width())

        fill_down_action = QAction(self.tr('Fill Down'), self.menu)
        fill_down_action.triggered.connect(self.fillDown)
        fill_down_action.setToolTip(self.tr('Copy the first value down to all other rows'))
        self.menu.addAction(fill_down_action)

        calculate_by_expression = QAction(QCoreApplication.translate('BatchPanel', 'Calculate by Expression…'),
                                          self.menu)
        calculate_by_expression.setIcon(QgsApplication.getThemeIcon('/mActionCalculateField.svg'))
        calculate_by_expression.triggered.connect(self.calculateByExpression)
        calculate_by_expression.setToolTip(self.tr('Calculates parameter values by evaluating an expression'))
        self.menu.addAction(calculate_by_expression)

        add_by_expression = QAction(QCoreApplication.translate('BatchPanel', 'Add Values by Expression…'),
                                    self.menu)
        add_by_expression.triggered.connect(self.addByExpression)
        add_by_expression.setToolTip(self.tr('Adds new parameter values by evaluating an expression'))
        self.menu.addAction(add_by_expression)

        if isinstance(self.parameterDefinition, (QgsProcessingParameterFile,
                                                 QgsProcessingParameterMapLayer,
                                                 QgsProcessingParameterRasterLayer,
                                                 QgsProcessingParameterMeshLayer,
                                                 QgsProcessingParameterVectorLayer,
                                                 QgsProcessingParameterFeatureSource)):
            self.menu.addSeparator()
            find_by_pattern_action = QAction(QCoreApplication.translate('BatchPanel', 'Add Files by Pattern…'),
                                             self.menu)
            find_by_pattern_action.triggered.connect(self.addFilesByPattern)
            find_by_pattern_action.setToolTip(self.tr('Adds files by a file pattern match'))
            self.menu.addAction(find_by_pattern_action)

    def fillDown(self):
        """
        Copy the top value down
        """
        context = dataobjects.createContext()

        wrapper = self.panel.wrappers[0][self.column]
        if wrapper is None:
            # e.g. double clicking on a destination header
            widget = self.panel.tblParameters.cellWidget(1, self.column)
            value = widget.getValue()
        else:
            value = wrapper.parameterValue()

        for row in range(1, self.panel.batchRowCount()):
            self.setRowValue(row, value, context)

    def setRowValue(self, row, value, context):
        """
        Sets the value for a row, in the current column
        """
        if self.panel.batchRowCount() <= row:
            self.panel.addRow()

        wrapper = self.panel.wrappers[row][self.column]
        if wrapper is None:
            # e.g. destination header
            self.panel.tblParameters.cellWidget(row + 1, self.column).setValue(str(value))
        else:
            wrapper.setParameterValue(value, context)

    def addFilesByPattern(self):
        """
        Populates the dialog using a file pattern match
        """
        dlg = QgsFindFilesByPatternDialog()
        dlg.setWindowTitle(self.tr("Add Files by Pattern"))
        if dlg.exec_():
            files = dlg.files()
            context = dataobjects.createContext()

            first_row = self.panel.batchRowCount() if self.panel.batchRowCount() > 1 else 0
            for row, file in enumerate(files):
                self.setRowValue(first_row + row, file, context)

    def calculateByExpression(self):
        """
        Calculates parameter values by evaluating expressions.
        """
        self.populateByExpression(adding=False)

    def addByExpression(self):
        """
        Adds new parameter values by evaluating an expression
        """
        self.populateByExpression(adding=True)

    def populateByExpression(self, adding=False):
        """
        Populates the panel using an expression
        """
        context = dataobjects.createContext()
        expression_context = context.expressionContext()

        # use the first row parameter values as a preview during expression creation
        params = self.panel.parametersForRow(0, warnOnInvalid=False)
        alg_scope = QgsExpressionContextUtils.processingAlgorithmScope(self.panel.alg, params, context)

        # create explicit variables corresponding to every parameter
        for k, v in params.items():
            alg_scope.setVariable(k, v, True)

        # add batchCount in the alg scope to be used in the expressions. 0 is only an example value
        alg_scope.setVariable('row_number', 0, False)

        expression_context.appendScope(alg_scope)

        # mark the parameter variables as highlighted for discoverability
        highlighted_vars = expression_context.highlightedVariables()
        highlighted_vars.extend(list(params.keys()))
        highlighted_vars.append('row_number')
        expression_context.setHighlightedVariables(highlighted_vars)

        dlg = QgsExpressionBuilderDialog(layer=None, context=context.expressionContext())
        if adding:
            dlg.setExpectedOutputFormat(self.tr('An array of values corresponding to each new row to add'))

        if not dlg.exec_():
            return

        if adding:
            exp = QgsExpression(dlg.expressionText())
            res = exp.evaluate(expression_context)

            if type(res) is not list:
                res = [res]

            first_row = self.panel.batchRowCount() if self.panel.batchRowCount() > 1 else 0
            for row, value in enumerate(res):
                self.setRowValue(row + first_row, value, context)
        else:
            for row in range(self.panel.batchRowCount()):
                params = self.panel.parametersForRow(row, warnOnInvalid=False)

                # remove previous algorithm scope -- we need to rebuild this completely, using the
                # other parameter values from the current row
                expression_context.popScope()
                alg_scope = QgsExpressionContextUtils.processingAlgorithmScope(self.panel.alg, params, context)

                for k, v in params.items():
                    alg_scope.setVariable(k, v, True)

                # add batch row number as evaluable variable in algorithm scope
                alg_scope.setVariable('row_number', row, False)

                expression_context.appendScope(alg_scope)

                # rebuild a new expression every time -- we don't want the expression compiler to replace
                # variables with precompiled values
                exp = QgsExpression(dlg.expressionText())
                value = exp.evaluate(expression_context)
                self.setRowValue(row, value, context)


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

        self.tblParameters.horizontalHeader().resizeSections(QHeaderView.ResizeToContents)
        self.tblParameters.horizontalHeader().setDefaultSectionSize(250)
        self.tblParameters.horizontalHeader().setMinimumSectionSize(150)

        self.processing_context = createContext()

        class ContextGenerator(QgsProcessingContextGenerator):

            def __init__(self, context):
                super().__init__()
                self.processing_context = context

            def processingContext(self):
                return self.processing_context

        self.context_generator = ContextGenerator(self.processing_context)

        self.column_to_parameter_definition = {}
        self.parameter_to_column = {}

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

            self.column_to_parameter_definition[column] = param.name()
            self.parameter_to_column[param.name()] = column
            column += 1

        for out in self.alg.destinationParameterDefinitions():
            if not out.flags() & QgsProcessingParameterDefinition.FlagHidden:
                self.tblParameters.setHorizontalHeaderItem(
                    column, QTableWidgetItem(out.description()))
                self.column_to_parameter_definition[column] = out.name()
                self.parameter_to_column[out.name()] = column
                column += 1

        self.addFillRow()

        # Add an empty row to begin
        self.addRow()

        self.tblParameters.horizontalHeader().resizeSections(QHeaderView.ResizeToContents)
        self.tblParameters.verticalHeader().setSectionResizeMode(QHeaderView.ResizeToContents)
        self.tblParameters.horizontalHeader().setStretchLastSection(True)

    def batchRowCount(self):
        """
        Returns the number of rows corresponding to execution iterations
        """
        return len(self.wrappers)

    def clear(self):
        self.tblParameters.setRowCount(1)
        self.wrappers = []
        self.column_to_parameter_definition = {}
        self.parameter_to_column = {}

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

        self.clear()
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
                        widget = self.tblParameters.cellWidget(row + 1, column)
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
        for row in range(self.batchRowCount()):
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
                    self.parent.messageBar().pushMessage("", self.tr(
                        'Wrong or missing parameter value: {0} (row {1})').format(
                        param.description(), row + 1),
                        level=Qgis.Warning, duration=5)
                    return
                algParams[param.name()] = param.valueAsPythonString(value, context)
                col += 1
            for out in alg.destinationParameterDefinitions():
                if out.flags() & QgsProcessingParameterDefinition.FlagHidden:
                    continue
                widget = self.tblParameters.cellWidget(row + 1, col)
                text = widget.getValue()
                if text.strip() != '':
                    algOutputs[out.name()] = text.strip()
                    col += 1
                else:
                    self.parent.messageBar().pushMessage("",
                                                         self.tr('Wrong or missing output value: {0} (row {1})').format(
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
        self.wrappers[row - 1][column] = wrapper

        widget_context = QgsProcessingParameterWidgetContext()
        widget_context.setProject(QgsProject.instance())
        if iface is not None:
            widget_context.setMapCanvas(iface.mapCanvas())
        if isinstance(self.alg, QgsProcessingModelAlgorithm):
            widget_context.setModel(self.alg)
        wrapper.setWidgetContext(widget_context)
        wrapper.registerProcessingContextGenerator(self.context_generator)

        # For compatibility with 3.x API, we need to check whether the wrapper is
        # the deprecated WidgetWrapper class. If not, it's the newer
        # QgsAbstractProcessingParameterWidgetWrapper class
        # TODO QGIS 4.0 - remove
        is_cpp_wrapper = not issubclass(wrapper.__class__, WidgetWrapper)
        if is_cpp_wrapper:
            widget = wrapper.createWrappedWidget(context)
        else:
            widget = wrapper.widget

        self.tblParameters.setCellWidget(row, column, widget)

    def addFillRow(self):
        self.tblParameters.setRowCount(1)
        for col, name in self.column_to_parameter_definition.items():
            param_definition = self.alg.parameterDefinition(self.column_to_parameter_definition[col])
            self.tblParameters.setCellWidget(0, col, BatchPanelFillWidget(param_definition, col, self))

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
        rows = set()
        for index in self.tblParameters.selectedIndexes():
            if index.row() == 0:
                continue
            rows.add(index.row())

        for row in sorted(rows, reverse=True):
            if self.tblParameters.rowCount() <= 2:
                break

            del self.wrappers[row - 1]
            self.tblParameters.removeRow(row)

    def toggleAdvancedMode(self, checked):
        for param in self.alg.parameterDefinitions():
            if param.flags() & QgsProcessingParameterDefinition.FlagAdvanced:
                self.tblParameters.setColumnHidden(self.parameter_to_column[param.name()], not checked)

    def valueForParameter(self, row, parameter_name):
        """
        Returns the current value for a parameter in a row
        """
        wrapper = self.wrappers[row][self.parameter_to_column[parameter_name]]
        return wrapper.parameterValue()

    def parametersForRow(self, row, destinationProject=None, warnOnInvalid=True):
        """
        Returns the parameters dictionary corresponding to a row in the batch table
        """
        col = 0
        parameters = {}
        for param in self.alg.parameterDefinitions():
            if param.flags() & QgsProcessingParameterDefinition.FlagHidden or param.isDestination():
                continue
            wrapper = self.wrappers[row][col]
            parameters[param.name()] = wrapper.parameterValue()
            if warnOnInvalid and not param.checkValueIsAcceptable(wrapper.parameterValue()):
                self.parent.messageBar().pushMessage("",
                                                     self.tr('Wrong or missing parameter value: {0} (row {1})').format(
                                                         param.description(), row + 1),
                                                     level=Qgis.Warning, duration=5)
                return {}
            col += 1
        count_visible_outputs = 0
        for out in self.alg.destinationParameterDefinitions():
            if out.flags() & QgsProcessingParameterDefinition.FlagHidden:
                continue

            count_visible_outputs += 1
            widget = self.tblParameters.cellWidget(row + 1, col)
            text = widget.getValue()
            if not warnOnInvalid or out.checkValueIsAcceptable(text):
                if isinstance(out, (QgsProcessingParameterRasterDestination,
                                    QgsProcessingParameterVectorDestination,
                                    QgsProcessingParameterFeatureSink)):
                    # load rasters and sinks on completion
                    parameters[out.name()] = QgsProcessingOutputLayerDefinition(text, destinationProject)
                else:
                    parameters[out.name()] = text
                col += 1
            else:
                self.parent.messageBar().pushMessage("", self.tr('Wrong or missing output value: {0} (row {1})').format(
                    out.description(), row + 1),
                    level=Qgis.Warning, duration=5)
                return {}
        return parameters
