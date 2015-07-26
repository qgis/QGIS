# -*- coding: utf-8 -*-

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
from .connector import OracleDBConnector

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from ..plugin import ConnectionError, InvalidDataException, DBPlugin, \
    Database, Schema, Table, VectorTable, TableField, TableConstraint, \
    TableIndex, TableTrigger, TableRule

try:
    from . import resources_rc
except ImportError:
    pass

from ..html_elems import HtmlParagraph, HtmlList, HtmlTable

from qgis.core import QgsCredentials


def classFactory():
    return OracleDBPlugin


class OracleDBPlugin(DBPlugin):

    @classmethod
    def icon(self):
        return QIcon(":/db_manager/oracle/icon")

    @classmethod
    def typeName(self):
        return 'oracle'

    @classmethod
    def typeNameString(self):
        return 'Oracle Spatial'

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
        settings = QSettings()
        settings.beginGroup(u"/{0}/{1}".format(
                            self.connectionSettingsKey(), conn_name))

        if not settings.contains("database"):  # non-existent entry?
            raise InvalidDataException(
                self.tr('There is no defined database connection "{}".'.format(
                    conn_name)))

        from qgis.core import QgsDataSourceURI
        uri = QgsDataSourceURI()

        settingsList = ["host", "port", "database", "username", "password"]
        host, port, database, username, password = map(
            lambda x: settings.value(x, "", type=str), settingsList)

        # qgis1.5 use 'savePassword' instead of 'save' setting
        savedPassword = settings.value("save", False, type=bool) or \
            settings.value("savePassword", False, type=bool)

        # get all of the connexion options

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

        settings.endGroup()

        uri.setConnection(host, port, database, username, password)

        uri.setUseEstimatedMetadata(useEstimatedMetadata)

        err = u""
        try:
            return self.connectToUri(uri)
        except ConnectionError, e:
            err = str(e)

        # ask for valid credentials
        max_attempts = 3
        for i in range(max_attempts):
            (ok, username, password) = QgsCredentials.instance().get(
                uri.connectionInfo(), username, password, err)

            if not ok:
                return False

            uri.setConnection(host, port, database, username, password)

            try:
                self.connectToUri(uri)
            except ConnectionError, e:
                if i == max_attempts - 1:  # failed the last attempt
                    raise e
                err = str(e)
                continue

            QgsCredentials.instance().put(
                uri.connectionInfo(), username, password)

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
        l = u""
        if limit:
            l = u"WHERE ROWNUM < {:d}".format(limit)
        con = self.database().connector
        # Prevent geometry column show
        tableName = table.replace(u'"', u"").split(u".")
        if len(tableName) == 0:
            tableName = [None, tableName[0]]
        colName = col.replace(u'"', u"").split(u".")[-1]

        if con.isGeometryColumn(tableName, colName):
            return None

        query = u"SELECT DISTINCT {} FROM {} {}".format(col, table, l)
        return self.sqlResultModel(query, self)

    def sqlResultModel(self, sql, parent):
        from .data_model import ORSqlResultModel
        return ORSqlResultModel(self, sql, parent)

    def toSqlLayer(self, sql, geomCol, uniqueCol,
                   layerName=u"QueryLayer", layerType=None,
                   avoidSelectById=False):
        from qgis.core import QgsMapLayer, QgsVectorLayer

        uri = self.uri()
        con = self.database().connector

        uri.setDataSource(u"", u"({})".format(sql), geomCol, u"", uniqueCol)
        if avoidSelectById:
            uri.disableSelectAtId(True)
        provider = self.dbplugin().providerName()
        vlayer = QgsVectorLayer(uri.uri(), layerName, provider)

        # handling undetermined geometry type
        if not vlayer.isValid():

            wkbType, srid = con.getTableMainGeomType(
                u"({})".format(sql), geomCol)
            uri.setWkbType(wkbType)
            if srid:
                uri.setSrid(unicode(srid))
            vlayer = QgsVectorLayer(uri.uri(), layerName, provider)

        return vlayer

    def registerDatabaseActions(self, mainWindow):
        action = QAction(QApplication.translate(
            "DBManagerPlugin", "&Re-connect"), self)
        mainWindow.registerAction(action, QApplication.translate(
            "DBManagerPlugin", "&Database"), self.reconnectActionSlot)

        if self.schemas():
            action = QAction(QApplication.translate(
                "DBManagerPlugin", "&Create schema"), self)
            mainWindow.registerAction(action, QApplication.translate(
                "DBManagerPlugin", "&Schema"), self.createSchemaActionSlot)
            action = QAction(QApplication.translate(
                "DBManagerPlugin", "&Delete (empty) schema"), self)
            mainWindow.registerAction(action, QApplication.translate(
                "DBManagerPlugin", "&Schema"), self.deleteSchemaActionSlot)

        action = QAction(QApplication.translate(
            "DBManagerPlugin", "Delete selected item"), self)
        mainWindow.registerAction(action, None, self.deleteActionSlot)
        action.setShortcuts(QKeySequence.Delete)

        action = QAction(QIcon(":/db_manager/actions/create_table"),
                         QApplication.translate(
                             "DBManagerPlugin", "&Create table"), self)
        mainWindow.registerAction(action, QApplication.translate(
            "DBManagerPlugin", "&Table"), self.createTableActionSlot)
        action = QAction(QIcon(":/db_manager/actions/edit_table"),
                         QApplication.translate(
                             "DBManagerPlugin", "&Edit table"), self)
        mainWindow.registerAction(action, QApplication.translate(
            "DBManagerPlugin", "&Table"), self.editTableActionSlot)
        action = QAction(QIcon(":/db_manager/actions/del_table"),
                         QApplication.translate(
                             "DBManagerPlugin", "&Delete table/view"), self)
        mainWindow.registerAction(action, QApplication.translate(
            "DBManagerPlugin", "&Table"), self.deleteTableActionSlot)
        action = QAction(QApplication.translate(
            "DBManagerPlugin", "&Empty table"), self)
        mainWindow.registerAction(action, QApplication.translate(
            "DBManagerPlugin", "&Table"), self.emptyTableActionSlot)


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
        self.objectType = None
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
        if self.objectType == u"MATERIALIZED VIEW":
            return self.database().connector.getMViewInfo(
                (self.schemaName(), self.name))
        else:
            return None

    def runAction(self, action):
        action = unicode(action)

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
                "Do you want to {} index {} ?".format(
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
                self.aboutToChange()
                self.database().connector.rebuildTableIndex(
                    (self.schemaName(), self.name), index_name)
                self.refreshIndexes()
                return True
        elif action.startswith(u"mview/"):
            if action == "mview/refresh":
                self.aboutToChange()
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

    def getValidQGisUniqueFields(self, onlyOne=False):
        """ list of fields valid to load the table as layer in QGis canvas.
        QGis automatically search for a valid unique field, so it's
        needed only for queries and views.
        """

        ret = []

        # add the pk
        pkcols = filter(lambda x: x.primaryKey, self.fields())
        if len(pkcols) == 1:
            ret.append(pkcols[0])

        # then add integer fields with an unique index
        indexes = self.indexes()
        if indexes is not None:
            for idx in indexes:
                if idx.isUnique and len(idx.columns) == 1:
                    fld = idx.fields()[idx.columns[0]]
                    if (fld.dataType == u"NUMBER"
                            and not fld.modifier
                            and fld.notNull
                            and fld not in ret):
                        ret.append(fld)

        # and finally append the other suitable fields
        for fld in self.fields():
            if (fld.dataType == u"NUMBER"
                    and not fld.modifier
                    and fld.notNull
                    and fld not in ret):
                ret.append(fld)

        if onlyOne:
            return ret[0] if len(ret) > 0 else None
        return ret

    def uri(self):
        uri = self.database().uri()
        schema = self.schemaName() if self.schemaName() else ''
        geomCol = self.geomColumn if self.type in [
            Table.VectorType, Table.RasterType] else ""
        uniqueCol = self.getValidQGisUniqueFields(
            True) if self.isView else None
        uri.setDataSource(schema, self.name, geomCol if geomCol else None,
                          None, uniqueCol.name if uniqueCol else "")

        # Handle geographic table
        if geomCol:
            uri.setWkbType(self.wkbType)
            uri.setSrid(unicode(self.srid))

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
                self.aboutToChange()
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
        if isinstance(self.charMaxLen, QPyNullVariant):
            self.charMaxLen = None
        else:
            self.charMaxLen = int(self.charMaxLen)

        if isinstance(self.modifier, QPyNullVariant):
            self.modifier = None
        else:
            self.modifier = int(self.modifier)

        if self.notNull.upper() == u"Y":
            self.notNull = False
        else:
            self.notNull = True

        if isinstance(self.comment, QPyNullVariant):
            self.comment = u""

        # find out whether fields are part of primary key
        for con in self.table().constraints():
            if (con.type == ORTableConstraint.TypePrimaryKey
                    and self.name == con.column):
                self.primaryKey = True
                break

    def type2String(self):
        if (u"TIMESTAMP" in self.dataType
            or self.dataType in [u"DATE", u"SDO_GEOMETRY",
                                 u"BINARY_FLOAT", u"BINARY_DOUBLE"]):
            return u"{}".format(self.dataType)
        if self.charMaxLen in [None, -1]:
            return u"{}".format(self.dataType)
        elif self.modifier in [None, -1, 0]:
            return u"{}({})".format(self.dataType, self.charMaxLen)

        return u"{}({},{})".format(self.dataType, self.charMaxLen,
                                   self.modifier)

    def update(self, new_name, new_type_str=None, new_not_null=None,
               new_default_str=None):
        self.table().aboutToChange()
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
        TypeUnique, TypeUnknown = range(5)

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

        if isinstance(row[6], QPyNullVariant):
            self.checkSource = u""
        else:
            self.checkSource = row[6]

        if isinstance(row[8], QPyNullVariant):
            self.foreignTable = u""
        else:
            self.foreignTable = row[8]

        if isinstance(row[7], QPyNullVariant):
            self.foreignOnDelete = u""
        else:
            self.foreignOnDelete = row[7]

        if isinstance(row[9], QPyNullVariant):
            self.foreignKey = u""
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
