"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QGIS (Oracle)
Date                 : Aug 27, 2014
copyright            : (C) 2014 by Médéric RIBREUX
email                : mederic.ribreux@gmail.com

The content of this file is based on
- PG_Manager by Martin Dobias <wonder.sk@gmail.com> (GPLv2 license)
- DB Manager by Giuseppe Sucameli <brush.tyler@gmail.com> (GPLv2 license)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""

# this will disable the dbplugin if the connector raise an ImportError
from typing import (
    Optional,
    Union
)

from .connector import OracleDBConnector

from qgis.PyQt.QtCore import Qt, QCoreApplication
from qgis.PyQt.QtGui import QIcon, QKeySequence
from qgis.PyQt.QtWidgets import QAction, QApplication, QMessageBox

from qgis.core import QgsApplication, QgsVectorLayer, NULL, QgsSettings

from ..plugin import ConnectionError, InvalidDataException, DBPlugin, \
    Database, Schema, Table, VectorTable, TableField, TableConstraint, \
    TableIndex, TableTrigger

from qgis.core import QgsCredentials


def classFactory():
    return OracleDBPlugin


class OracleDBPlugin(DBPlugin):

    @classmethod
    def icon(self):
        return QgsApplication.getThemeIcon("/mIconOracle.svg")

    @classmethod
    def typeName(self):
        return 'oracle'

    @classmethod
    def typeNameString(self):
        return QCoreApplication.translate('db_manager', 'Oracle Spatial')

    @classmethod
    def providerName(self):
        return 'oracle'

    @classmethod
    def connectionSettingsKey(self):
        return '/Oracle/connections'

    def connectToUri(self, uri):
        self.db = self.databasesFactory(self, uri)
        if self.db:
            return True
        return False

    def databasesFactory(self, connection, uri):
        return ORDatabase(connection, uri)

    def connect(self, parent=None):
        conn_name = self.connectionName()
        settings = QgsSettings()
        settings.beginGroup("/{}/{}".format(
            self.connectionSettingsKey(), conn_name))

        if not settings.contains("database"):  # non-existent entry?
            raise InvalidDataException(
                self.tr('There is no defined database connection "{}".'.format(
                    conn_name)))

        from qgis.core import QgsDataSourceUri
        uri = QgsDataSourceUri()

        settingsList = ["host", "port", "database", "username", "password"]
        host, port, database, username, password = (
            settings.value(x, "", type=str) for x in settingsList)

        # get all of the connection options

        useEstimatedMetadata = settings.value(
            "estimatedMetadata", False, type=bool)
        uri.setParam('userTablesOnly', str(
            settings.value("userTablesOnly", False, type=bool)))
        uri.setParam('geometryColumnsOnly', str(
            settings.value("geometryColumnsOnly", False, type=bool)))
        uri.setParam('allowGeometrylessTables', str(
            settings.value("allowGeometrylessTables", False, type=bool)))
        uri.setParam('onlyExistingTypes', str(
            settings.value("onlyExistingTypes", False, type=bool)))
        uri.setParam('includeGeoAttributes', str(
            settings.value("includeGeoAttributes", False, type=bool)))

        settings.endGroup()

        uri.setConnection(host, port, database, username, password)

        uri.setUseEstimatedMetadata(useEstimatedMetadata)

        err = ""
        try:
            return self.connectToUri(uri)
        except ConnectionError as e:
            err = str(e)

        # ask for valid credentials
        max_attempts = 3
        for i in range(max_attempts):
            (ok, username, password) = QgsCredentials.instance().get(
                uri.connectionInfo(False), username, password, err)

            if not ok:
                return False

            uri.setConnection(host, port, database, username, password)

            try:
                self.connectToUri(uri)
            except ConnectionError as e:
                if i == max_attempts - 1:  # failed the last attempt
                    raise e
                err = str(e)
                continue

            QgsCredentials.instance().put(
                uri.connectionInfo(False), username, password)

            return True

        return False


class ORDatabase(Database):

    def __init__(self, connection, uri):
        self.connName = connection.connectionName()
        Database.__init__(self, connection, uri)

    def connectorsFactory(self, uri):
        return OracleDBConnector(uri, self.connName)

    def dataTablesFactory(self, row, db, schema=None):
        return ORTable(row, db, schema)

    def vectorTablesFactory(self, row, db, schema=None):
        return ORVectorTable(row, db, schema)

    def info(self):
        from .info_model import ORDatabaseInfo
        return ORDatabaseInfo(self)

    def schemasFactory(self, row, db):
        return ORSchema(row, db)

    def columnUniqueValuesModel(self, col, table, limit=10):
        l = ""
        if limit:
            l = "WHERE ROWNUM < {:d}".format(limit)
        con = self.database().connector
        # Prevent geometry column show
        tableName = table.replace('"', "").split(".")
        if len(tableName) == 0:
            tableName = [None, tableName[0]]
        colName = col.replace('"', "").split(".")[-1]

        if con.isGeometryColumn(tableName, colName):
            return None

        query = "SELECT DISTINCT {} FROM {} {}".format(col, table, l)
        return self.sqlResultModel(query, self)

    def sqlResultModel(self, sql, parent):
        from .data_model import ORSqlResultModel
        return ORSqlResultModel(self, sql, parent)

    def sqlResultModelAsync(self, sql, parent):
        from .data_model import ORSqlResultModelAsync

        return ORSqlResultModelAsync(self, sql, parent)

    def toSqlLayer(self, sql, geomCol, uniqueCol,
                   layerName="QueryLayer", layerType=None,
                   avoidSelectById=False, filter=""):

        uri = self.uri()
        con = self.database().connector

        if uniqueCol is not None:
            uniqueCol = uniqueCol.strip('"').replace('""', '"')

        uri.setDataSource("", "({}\n)".format(
            sql), geomCol, filter, uniqueCol)

        if avoidSelectById:
            uri.disableSelectAtId(True)
        provider = self.dbplugin().providerName()
        vlayer = QgsVectorLayer(uri.uri(False), layerName, provider)

        # handling undetermined geometry type
        if not vlayer.isValid():

            wkbType, srid = con.getTableMainGeomType(
                "({}\n)".format(sql), geomCol)
            uri.setWkbType(wkbType)
            if srid:
                uri.setSrid(str(srid))
            vlayer = QgsVectorLayer(uri.uri(False), layerName, provider)

        return vlayer

    def registerDatabaseActions(self, mainWindow):
        action = QAction(QApplication.translate(
            "DBManagerPlugin", "&Re-connect"), self)
        mainWindow.registerAction(action, QApplication.translate(
            "DBManagerPlugin", "&Database"), self.reconnectActionSlot)

        if self.schemas():
            action = QAction(QApplication.translate(
                "DBManagerPlugin", "&Create Schema…"), self)
            mainWindow.registerAction(action, QApplication.translate(
                "DBManagerPlugin", "&Schema"), self.createSchemaActionSlot)
            action = QAction(QApplication.translate(
                "DBManagerPlugin", "&Delete (Empty) Schema…"), self)
            mainWindow.registerAction(action, QApplication.translate(
                "DBManagerPlugin", "&Schema"), self.deleteSchemaActionSlot)

        action = QAction(QApplication.translate(
            "DBManagerPlugin", "Delete Selected Item"), self)
        mainWindow.registerAction(action, None, self.deleteActionSlot)
        action.setShortcuts(QKeySequence.Delete)

        action = QAction(QgsApplication.getThemeIcon("/mActionCreateTable.svg"),
                         QApplication.translate(
                             "DBManagerPlugin", "&Create Table…"), self)
        mainWindow.registerAction(action, QApplication.translate(
            "DBManagerPlugin", "&Table"), self.createTableActionSlot)
        action = QAction(QgsApplication.getThemeIcon("/mActionEditTable.svg"),
                         QApplication.translate(
                             "DBManagerPlugin", "&Edit Table…"), self)
        mainWindow.registerAction(action, QApplication.translate(
            "DBManagerPlugin", "&Table"), self.editTableActionSlot)
        action = QAction(QgsApplication.getThemeIcon("/mActionDeleteTable.svg"),
                         QApplication.translate(
                             "DBManagerPlugin", "&Delete Table/View…"), self)
        mainWindow.registerAction(action, QApplication.translate(
            "DBManagerPlugin", "&Table"), self.deleteTableActionSlot)
        action = QAction(QApplication.translate(
            "DBManagerPlugin", "&Empty Table…"), self)
        mainWindow.registerAction(action, QApplication.translate(
            "DBManagerPlugin", "&Table"), self.emptyTableActionSlot)

    def supportsComment(self):
        return False


class ORSchema(Schema):

    def __init__(self, row, db):
        Schema.__init__(self, db)
        # self.oid, self.name, self.owner, self.perms, self.comment = row
        self.name = row[0]


class ORTable(Table):

    def __init__(self, row, db, schema=None):
        Table.__init__(self, db, schema)
        self.name, self.owner, isView = row

        self.estimatedRowCount = None
        self.objectType: Optional[Union[str, bool]] = None
        self.isView = False
        self.isMaterializedView = False
        if isView == 1:
            self.isView = True
        self.creationDate = None
        self.modificationDate = None

    def getDates(self):
        """Grab the creation/modification dates of the table"""
        self.creationDate, self.modificationDate = (
            self.database().connector.getTableDates((self.schemaName(),
                                                     self.name)))

    def refreshRowEstimation(self):
        """Use ALL_ALL_TABLE to get an estimation of rows"""
        if self.isView:
            self.estimatedRowCount = 0

        self.estimatedRowCount = (
            self.database().connector.getTableRowEstimation(
                (self.schemaName(), self.name)))

    def getType(self):
        """Grab the type of object for the table"""
        self.objectType = self.database().connector.getTableType(
            (self.schemaName(), self.name))

    def getComment(self):
        """Grab the general comment of the table/view"""
        self.comment = self.database().connector.getTableComment(
            (self.schemaName(), self.name), self.objectType)

    def getDefinition(self):
        return self.database().connector.getDefinition(
            (self.schemaName(), self.name), self.objectType)

    def getMViewInfo(self):
        if self.objectType == "MATERIALIZED VIEW":
            return self.database().connector.getMViewInfo(
                (self.schemaName(), self.name))
        else:
            return None

    def runAction(self, action):
        action = str(action)

        if action.startswith("rows/"):
            if action == "rows/recount":
                self.refreshRowCount()
                return True
        elif action.startswith("index/"):
            parts = action.split('/')
            index_name = parts[1]
            index_action = parts[2]

            msg = QApplication.translate(
                "DBManagerPlugin",
                "Do you want to {} index {}?".format(
                    index_action, index_name))
            QApplication.restoreOverrideCursor()
            try:
                if QMessageBox.question(
                        None,
                        QApplication.translate(
                            "DBManagerPlugin", "Table Index"),
                        msg,
                        QMessageBox.Yes | QMessageBox.No) == QMessageBox.No:
                    return False
            finally:
                QApplication.setOverrideCursor(Qt.WaitCursor)

            if index_action == "rebuild":
                self.aboutToChange.emit()
                self.database().connector.rebuildTableIndex(
                    (self.schemaName(), self.name), index_name)
                self.refreshIndexes()
                return True
        elif action.startswith("mview/"):
            if action == "mview/refresh":
                self.aboutToChange.emit()
                self.database().connector.refreshMView(
                    (self.schemaName(), self.name))
                return True

        return Table.runAction(self, action)

    def tableFieldsFactory(self, row, table):
        return ORTableField(row, table)

    def tableConstraintsFactory(self, row, table):
        return ORTableConstraint(row, table)

    def tableIndexesFactory(self, row, table):
        return ORTableIndex(row, table)

    def tableTriggersFactory(self, row, table):
        return ORTableTrigger(row, table)

    def info(self):
        from .info_model import ORTableInfo
        return ORTableInfo(self)

    def tableDataModel(self, parent):
        from .data_model import ORTableDataModel
        return ORTableDataModel(self, parent)

    def getValidQgisUniqueFields(self, onlyOne=False):
        """ list of fields valid to load the table as layer in QGIS canvas.
        QGIS automatically search for a valid unique field, so it's
        needed only for queries and views.
        """

        ret = []

        # add the pk
        pkcols = [x for x in self.fields() if x.primaryKey]
        if len(pkcols) == 1:
            ret.append(pkcols[0])

        # then add integer fields with an unique index
        indexes = self.indexes()
        if indexes is not None:
            for idx in indexes:
                if idx.isUnique and len(idx.columns) == 1:
                    fld = idx.fields()[idx.columns[0]]
                    if (fld.dataType == "NUMBER" and not fld.modifier and fld.notNull and fld not in ret):
                        ret.append(fld)

        # and finally append the other suitable fields
        for fld in self.fields():
            if (fld.dataType == "NUMBER" and not fld.modifier and fld.notNull and fld not in ret):
                ret.append(fld)

        if onlyOne:
            return ret[0] if len(ret) > 0 else None
        return ret

    def uri(self):
        uri = self.database().uri()
        schema = self.schemaName() if self.schemaName() else ''
        geomCol = self.geomColumn if self.type in [
            Table.VectorType, Table.RasterType] else ""
        uniqueCol = self.getValidQgisUniqueFields(
            True) if self.isView else None
        uri.setDataSource(schema, self.name, geomCol if geomCol else None,
                          None, uniqueCol.name if uniqueCol else "")

        # Handle geographic table
        if geomCol:
            uri.setWkbType(self.wkbType)
            uri.setSrid(str(self.srid))

        return uri


class ORVectorTable(ORTable, VectorTable):

    def __init__(self, row, db, schema=None):
        ORTable.__init__(self, row[0:3], db, schema)
        VectorTable.__init__(self, db, schema)
        self.geomColumn, self.geomType, self.wkbType, self.geomDim, \
            self.srid = row[-7:-2]

    def info(self):
        from .info_model import ORVectorTableInfo
        return ORVectorTableInfo(self)

    def runAction(self, action):
        if action.startswith("extent/"):
            if action == "extent/update":
                self.aboutToChange.emit()
                self.updateExtent()
                return True

        if ORTable.runAction(self, action):
            return True

        return VectorTable.runAction(self, action)

    def canUpdateMetadata(self):
        return self.database().connector.canUpdateMetadata((self.schemaName(),
                                                            self.name))

    def updateExtent(self):
        self.database().connector.updateMetadata(
            (self.schemaName(), self.name),
            self.geomColumn, extent=self.extent)
        self.refreshTableEstimatedExtent()
        self.refresh()

    def hasSpatialIndex(self, geom_column=None):
        geom_column = geom_column if geom_column else self.geomColumn

        for idx in self.indexes():
            if geom_column == idx.column:
                return True
        return False


class ORTableField(TableField):

    def __init__(self, row, table):
        """ build fields information from query and find primary key """
        TableField.__init__(self, table)
        self.num, self.name, self.dataType, self.charMaxLen, \
            self.modifier, self.notNull, self.hasDefault, \
            self.default, typeStr, self.comment = row

        self.primaryKey = False
        self.num = int(self.num)
        if self.charMaxLen == NULL:
            self.charMaxLen = None
        else:
            self.charMaxLen = int(self.charMaxLen)

        if self.modifier == NULL:
            self.modifier = None
        else:
            self.modifier = int(self.modifier)

        if self.notNull.upper() == "Y":
            self.notNull = False
        else:
            self.notNull = True

        if self.comment == NULL:
            self.comment = ""

        # find out whether fields are part of primary key
        for con in self.table().constraints():
            if con.type == ORTableConstraint.TypePrimaryKey and self.name == con.column:
                self.primaryKey = True
                break

    def type2String(self):
        if ("TIMESTAMP" in self.dataType or self.dataType in ["DATE", "SDO_GEOMETRY", "BINARY_FLOAT", "BINARY_DOUBLE"]):
            return "{}".format(self.dataType)
        if self.charMaxLen in [None, -1]:
            return "{}".format(self.dataType)
        elif self.modifier in [None, -1, 0]:
            return "{}({})".format(self.dataType, self.charMaxLen)

        return "{}({},{})".format(self.dataType, self.charMaxLen,
                                  self.modifier)

    def update(self, new_name, new_type_str=None, new_not_null=None,
               new_default_str=None):
        self.table().aboutToChange.emit()
        if self.name == new_name:
            new_name = None
        if self.type2String() == new_type_str:
            new_type_str = None
        if self.notNull == new_not_null:
            new_not_null = None
        if self.default2String() == new_default_str:
            new_default_str = None

        ret = self.table().database().connector.updateTableColumn(
            (self.table().schemaName(), self.table().name),
            self.name, new_name, new_type_str,
            new_not_null, new_default_str)

        # When changing a field, refresh also constraints and
        # indexes.
        if ret is not False:
            self.table().refreshFields()
            self.table().refreshConstraints()
            self.table().refreshIndexes()
        return ret


class ORTableConstraint(TableConstraint):
    TypeCheck, TypeForeignKey, TypePrimaryKey, \
        TypeUnique, TypeUnknown = list(range(5))

    types = {"c": TypeCheck, "r": TypeForeignKey,
             "p": TypePrimaryKey, "u": TypeUnique}

    def __init__(self, row, table):
        """ build constraints info from query """
        TableConstraint.__init__(self, table)
        self.name, constr_type_str, self.column, self.validated, \
            self.generated, self.status = row[0:6]
        constr_type_str = constr_type_str.lower()

        if constr_type_str in ORTableConstraint.types:
            self.type = ORTableConstraint.types[constr_type_str]
        else:
            self.type = ORTableConstraint.TypeUnknown

        if row[6] == NULL:
            self.checkSource = ""
        else:
            self.checkSource = row[6]

        if row[8] == NULL:
            self.foreignTable = ""
        else:
            self.foreignTable = row[8]

        if row[7] == NULL:
            self.foreignOnDelete = ""
        else:
            self.foreignOnDelete = row[7]

        if row[9] == NULL:
            self.foreignKey = ""
        else:
            self.foreignKey = row[9]

    def type2String(self):
        if self.type == ORTableConstraint.TypeCheck:
            return QApplication.translate("DBManagerPlugin", "Check")
        if self.type == ORTableConstraint.TypePrimaryKey:
            return QApplication.translate("DBManagerPlugin", "Primary key")
        if self.type == ORTableConstraint.TypeForeignKey:
            return QApplication.translate("DBManagerPlugin", "Foreign key")
        if self.type == ORTableConstraint.TypeUnique:
            return QApplication.translate("DBManagerPlugin", "Unique")

        return QApplication.translate("DBManagerPlugin", 'Unknown')

    def fields(self):
        """ Hack to make edit dialog box work """
        fields = self.table().fields()
        field = None
        for fld in fields:
            if fld.name == self.column:
                field = fld
        cols = {}
        cols[0] = field

        return cols


class ORTableIndex(TableIndex):

    def __init__(self, row, table):
        TableIndex.__init__(self, table)
        self.name, self.column, self.indexType, self.status, \
            self.analyzed, self.compression, self.isUnique = row

    def fields(self):
        """ Hack to make edit dialog box work """
        self.table().refreshFields()
        fields = self.table().fields()

        field = None
        for fld in fields:
            if fld.name == self.column:
                field = fld
        cols = {}
        cols[0] = field

        return cols


class ORTableTrigger(TableTrigger):

    def __init__(self, row, table):
        TableTrigger.__init__(self, table)
        self.name, self.event, self.type, self.enabled = row
