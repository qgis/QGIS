# -*- coding: utf-8 -*-

"""
***************************************************************************
    postgis.py - Postgis widget wrappers
    ---------------------
    Date                 : December 2016
    Copyright            : (C) 2016 by Arnaud Morvan
    Email                : arnaud dot morvan at camptocamp dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""


from qgis.core import (QgsSettings,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterFile,
                       QgsProcessingParameterField,
                       QgsProcessingParameterExpression,
                       QgsProcessingOutputString,
                       QgsProcessingParameterString)

from qgis.PyQt.QtWidgets import QComboBox

from processing.gui.wrappers import (
    WidgetWrapper,
    DIALOG_MODELER,
)
from processing.tools.postgis import GeoDB


class ConnectionWidgetWrapper(WidgetWrapper):
    """
    WidgetWrapper for ParameterString that create and manage a combobox widget
    with existing postgis connections.
    """

    def createWidget(self):
        self._combo = QComboBox()
        for group in self.items():
            self._combo.addItem(*group)
        self._combo.currentIndexChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
        return self._combo

    def items(self):
        settings = QgsSettings()
        settings.beginGroup('/PostgreSQL/connections/')
        items = [(group, group) for group in settings.childGroups()]

        if self.dialogType == DIALOG_MODELER:
            strings = self.dialog.getAvailableValuesOfType(
                [QgsProcessingParameterString, QgsProcessingParameterNumber, QgsProcessingParameterFile,
                 QgsProcessingParameterField, QgsProcessingParameterExpression], QgsProcessingOutputString)
            items = items + [(self.dialog.resolveValueDescription(s), s) for s in strings]

        return items

    def setValue(self, value):
        self.setComboValue(value, self._combo)

    def value(self):
        return self.comboValue(combobox=self._combo)


class SchemaWidgetWrapper(WidgetWrapper):
    """
    WidgetWrapper for ParameterString that create and manage a combobox widget
    with existing schemas from a parent connection parameter.
    """

    def createWidget(self, connection_param=None):
        self._connection_param = connection_param
        self._connection = None
        self._database = None

        self._combo = QComboBox()
        self._combo.setEditable(True)
        self.refreshItems()
        self._combo.currentIndexChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
        self._combo.lineEdit().editingFinished.connect(lambda: self.widgetValueHasChanged.emit(self))

        return self._combo

    def postInitialize(self, wrappers):
        for wrapper in wrappers:
            if wrapper.parameterDefinition().name() == self._connection_param:
                self.connection_wrapper = wrapper
                self.setConnection(wrapper.parameterValue())
                wrapper.widgetValueHasChanged.connect(self.connectionChanged)
                break

    def connectionChanged(self, wrapper):
        connection = wrapper.parameterValue()
        if connection == self._connection:
            return
        self.setConnection(connection)

    def setConnection(self, connection):
        self._connection = connection
        # when there is NO connection (yet), this get's called with a ''-connection
        if isinstance(connection, str) and connection != '':
            self._database = GeoDB.from_name(connection)
        else:
            self._database = None
        self.refreshItems()
        self.widgetValueHasChanged.emit(self)

    def refreshItems(self):
        value = self.comboValue(combobox=self._combo)

        self._combo.clear()

        if self._database is not None:
            for schema in self._database.list_schemas():
                self._combo.addItem(schema[1], schema[1])

        if self.dialogType == DIALOG_MODELER:
            strings = self.dialog.getAvailableValuesOfType(
                [QgsProcessingParameterString, QgsProcessingParameterNumber, QgsProcessingParameterFile,
                 QgsProcessingParameterField, QgsProcessingParameterExpression], QgsProcessingOutputString)
            for text, data in [(self.dialog.resolveValueDescription(s), s) for s in strings]:
                self._combo.addItem(text, data)

        self.setComboValue(value, self._combo)

    def setValue(self, value):
        self.setComboValue(value, self._combo)
        self.widgetValueHasChanged.emit(self)

    def value(self):
        return self.comboValue(combobox=self._combo)

    def database(self):
        return self._database


class TableWidgetWrapper(WidgetWrapper):
    """
    WidgetWrapper for ParameterString that create and manage a combobox widget
    with existing tables from a parent schema parameter.
    """

    def createWidget(self, schema_param=None):
        self._schema_param = schema_param
        self._database = None
        self._schema = None

        self._combo = QComboBox()
        self._combo.setEditable(True)
        self.refreshItems()
        self._combo.currentIndexChanged.connect(lambda: self.widgetValueHasChanged.emit(self))
        self._combo.lineEdit().editingFinished.connect(lambda: self.widgetValueHasChanged.emit(self))

        return self._combo

    def postInitialize(self, wrappers):
        for wrapper in wrappers:
            if wrapper.parameterDefinition().name() == self._schema_param:
                self.schema_wrapper = wrapper
                self.setSchema(wrapper.database(), wrapper.parameterValue())
                wrapper.widgetValueHasChanged.connect(self.schemaChanged)
                break

    def schemaChanged(self, wrapper):
        database = wrapper.database()
        schema = wrapper.parameterValue()
        if database == self._database and schema == self._schema:
            return
        self.setSchema(database, schema)

    def setSchema(self, database, schema):
        self._database = database
        self._schema = schema
        self.refreshItems()
        self.widgetValueHasChanged.emit(self)

    def refreshItems(self):
        value = self.comboValue(combobox=self._combo)

        self._combo.clear()

        if (self._database is not None and isinstance(self._schema, str)):
            for table in self._database.list_geotables(self._schema):
                self._combo.addItem(table[0], table[0])

        if self.dialogType == DIALOG_MODELER:
            strings = self.dialog.getAvailableValuesOfType(
                [QgsProcessingParameterString, QgsProcessingParameterNumber, QgsProcessingParameterFile,
                 QgsProcessingParameterField, QgsProcessingParameterExpression], QgsProcessingOutputString)
            for text, data in [(self.dialog.resolveValueDescription(s), s) for s in strings]:
                self._combo.addItem(text, data)

        self.setComboValue(value, self._combo)

    def setValue(self, value):
        self.setComboValue(value, self._combo)
        self.widgetValueHasChanged.emit(self)

    def value(self):
        return self.comboValue(combobox=self._combo)
