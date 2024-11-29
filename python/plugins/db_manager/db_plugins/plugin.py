"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QGIS
Date                 : May 23, 2011
copyright            : (C) 2011 by Giuseppe Sucameli
email                : brush.tyler@gmail.com

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

from qgis.PyQt.QtCore import Qt, QObject, pyqtSignal, QByteArray

from qgis.PyQt.QtWidgets import (
    QFormLayout,
    QComboBox,
    QCheckBox,
    QDialogButtonBox,
    QPushButton,
    QLabel,
    QApplication,
    QAction,
    QMenu,
    QInputDialog,
    QMessageBox,
    QDialog,
    QWidget,
)

from qgis.PyQt.QtGui import QKeySequence

from qgis.core import (
    Qgis,
    QgsApplication,
    QgsSettings,
    QgsMapLayerType,
    QgsWkbTypes,
    QgsProviderConnectionException,
    QgsProviderRegistry,
    QgsVectorLayer,
    QgsRasterLayer,
    QgsProject,
    QgsMessageLog,
    QgsCoordinateReferenceSystem,
)

from qgis.gui import QgsMessageBarItem, QgsProjectionSelectionWidget

from ..db_plugins import createDbPlugin


class BaseError(Exception):
    """Base class for exceptions in the plugin."""

    def __init__(self, e):
        if isinstance(e, Exception):
            msg = e.args[0] if len(e.args) > 0 else ""
        else:
            msg = e

        if not isinstance(msg, str):
            msg = str(
                msg, "utf-8", "replace"
            )  # convert from utf8 and replace errors (if any)

        self.msg = msg
        Exception.__init__(self, msg)

    def __unicode__(self):
        return self.msg


class InvalidDataException(BaseError):
    pass


class ConnectionError(BaseError):
    pass


class DbError(BaseError):

    def __init__(self, e, query=None):
        BaseError.__init__(self, e)
        self.query = str(query) if query is not None else None

    def __unicode__(self):
        if self.query is None:
            return BaseError.__unicode__(self)

        msg = QApplication.translate("DBManagerPlugin", "Error:\n{0}").format(
            BaseError.__unicode__(self)
        )
        if self.query:
            msg += QApplication.translate("DBManagerPlugin", "\n\nQuery:\n{0}").format(
                self.query
            )
        return msg


class DBPlugin(QObject):
    deleted = pyqtSignal()
    changed = pyqtSignal()
    aboutToChange = pyqtSignal()

    def __init__(self, conn_name, parent=None):
        QObject.__init__(self, parent)
        self.connName = conn_name
        self.db = None

    def __del__(self):
        pass  # print "DBPlugin.__del__", self.connName

    def connectionIcon(self):
        return QgsApplication.getThemeIcon("/mIconDbSchema.svg")

    def connectionName(self):
        return self.connName

    def database(self):
        return self.db

    def info(self):
        from .info_model import DatabaseInfo

        return DatabaseInfo(None)

    def connect(self, parent=None):
        raise NotImplementedError("Needs to be implemented by subclasses")

    def connectToUri(self, uri):
        self.db = self.databasesFactory(self, uri)
        if self.db:
            return True
        return False

    def reconnect(self):
        if self.db is not None:
            uri = self.db.uri()
            self.db.deleteLater()
            self.db = None
            return self.connectToUri(uri)
        return self.connect(self.parent())

    def remove(self):

        # Try the new API first, fallback to legacy
        try:
            md = QgsProviderRegistry.instance().providerMetadata(self.providerName())
            md.deleteConnection(self.connectionName())
        except (AttributeError, QgsProviderConnectionException):
            settings = QgsSettings()
            settings.beginGroup(
                f"/{self.connectionSettingsKey()}/{self.connectionName()}"
            )
            settings.remove("")

        self.deleted.emit()
        return True

    @classmethod
    def addConnection(self, conn_name, uri):
        raise NotImplementedError("Needs to be implemented by subclasses")

    @classmethod
    def icon(self):
        return None

    @classmethod
    def typeName(self):
        # return the db typename (e.g. 'postgis')
        pass

    @classmethod
    def typeNameString(self):
        # return the db typename string (e.g. 'PostGIS')
        pass

    @classmethod
    def providerName(self):
        # return the provider's name (e.g. 'postgres')
        pass

    @classmethod
    def connectionSettingsKey(self):
        # return the key used to store the connections in settings
        pass

    @classmethod
    def connections(self):
        # get the list of connections

        conn_list = []

        # First try with the new core API, if that fails, proceed with legacy code
        try:
            md = QgsProviderRegistry.instance().providerMetadata(self.providerName())
            for name in md.dbConnections(False).keys():
                conn_list.append(createDbPlugin(self.typeName(), name))
        except (AttributeError, QgsProviderConnectionException):
            settings = QgsSettings()
            settings.beginGroup(self.connectionSettingsKey())
            for name in settings.childGroups():
                conn_list.append(createDbPlugin(self.typeName(), name))
            settings.endGroup()

        return conn_list

    def databasesFactory(self, connection, uri):
        return None

    @classmethod
    def addConnectionActionSlot(self, item, action, parent):
        raise NotImplementedError("Needs to be implemented by subclasses")

    def removeActionSlot(self, item, action, parent):
        QApplication.restoreOverrideCursor()
        try:
            res = QMessageBox.question(
                parent,
                QApplication.translate("DBManagerPlugin", "DB Manager"),
                QApplication.translate(
                    "DBManagerPlugin", "Really remove connection to {0}?"
                ).format(item.connectionName()),
                QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
            )
            if res != QMessageBox.StandardButton.Yes:
                return
        finally:
            QApplication.setOverrideCursor(Qt.CursorShape.WaitCursor)

        item.remove()


class DbItemObject(QObject):
    changed = pyqtSignal()
    aboutToChange = pyqtSignal()
    deleted = pyqtSignal()

    def __init__(self, parent=None):
        super().__init__(parent)

    def database(self):
        return None

    def refresh(self):
        self.changed.emit()  # refresh the item data reading them from the db

    def info(self):
        pass

    def runAction(self):
        pass

    def registerActions(self, mainWindow):
        pass


class Database(DbItemObject):

    def __init__(self, dbplugin, uri):
        super().__init__(dbplugin)
        self.connector = self.connectorsFactory(uri)

    def connectorsFactory(self, uri):
        return None

    def __del__(self):
        self.connector = None
        pass  # print "Database.__del__", self

    def connection(self):
        return self.parent()

    def dbplugin(self):
        return self.parent()

    def database(self):
        return self

    def uri(self):
        return self.connector.uri()

    def publicUri(self):
        return self.connector.publicUri()

    def delete(self):
        self.aboutToChange.emit()
        ret = self.connection().remove()
        if ret is not False:
            self.deleted.emit()
        return ret

    def info(self):
        from .info_model import DatabaseInfo

        return DatabaseInfo(self)

    def sqlResultModel(self, sql, parent):
        from .data_model import SqlResultModel

        return SqlResultModel(self, sql, parent)

    def sqlResultModelAsync(self, sql, parent):
        from .data_model import SqlResultModelAsync

        return SqlResultModelAsync(self, sql, parent)

    def columnUniqueValuesModel(self, col, table, limit=10):
        l = ""
        if limit is not None:
            l = "LIMIT %d" % limit
        return self.sqlResultModel(f"SELECT DISTINCT {col} FROM {table} {l}", self)

    def uniqueIdFunction(self):
        """Return a SQL function used to generate a unique id for rows of a query"""
        # may be overloaded by derived classes
        return "row_number() over ()"

    def toSqlLayer(
        self,
        sql,
        geomCol,
        uniqueCol,
        layerName="QueryLayer",
        layerType=None,
        avoidSelectById=False,
        filter="",
    ):
        if uniqueCol is None:
            if hasattr(self, "uniqueIdFunction"):
                uniqueFct = self.uniqueIdFunction()
                if uniqueFct is not None:
                    q = 1
                    while "_subq_%d_" % q in sql:
                        q += 1
                    sql = "SELECT %s AS _uid_,* FROM (%s\n) AS _subq_%d_" % (
                        uniqueFct,
                        sql,
                        q,
                    )
                    uniqueCol = "_uid_"

        uri = self.uri()
        uri.setDataSource("", "(%s\n)" % sql, geomCol, filter, uniqueCol)
        if avoidSelectById:
            uri.disableSelectAtId(True)
        provider = self.dbplugin().providerName()
        if layerType == QgsMapLayerType.RasterLayer:
            return QgsRasterLayer(uri.uri(False), layerName, provider)
        return QgsVectorLayer(uri.uri(False), layerName, provider)

    def registerAllActions(self, mainWindow):
        self.registerDatabaseActions(mainWindow)
        self.registerSubPluginActions(mainWindow)

    def registerSubPluginActions(self, mainWindow):
        # load plugins!
        try:
            exec("from .%s.plugins import load" % self.dbplugin().typeName(), globals())
        except ImportError:
            pass
        else:
            load(self, mainWindow)  # NOQA

    def registerDatabaseActions(self, mainWindow):
        action = QAction(QApplication.translate("DBManagerPlugin", "&Re-connect"), self)
        mainWindow.registerAction(
            action,
            QApplication.translate("DBManagerPlugin", "&Database"),
            self.reconnectActionSlot,
        )

        if self.schemas() is not None:
            action = QAction(
                QApplication.translate("DBManagerPlugin", "&Create Schema…"), self
            )
            mainWindow.registerAction(
                action,
                QApplication.translate("DBManagerPlugin", "&Schema"),
                self.createSchemaActionSlot,
            )
            action = QAction(
                QApplication.translate("DBManagerPlugin", "&Delete (Empty) Schema"),
                self,
            )
            mainWindow.registerAction(
                action,
                QApplication.translate("DBManagerPlugin", "&Schema"),
                self.deleteSchemaActionSlot,
            )

        action = QAction(
            QApplication.translate("DBManagerPlugin", "Delete Selected Item"), self
        )
        mainWindow.registerAction(action, None, self.deleteActionSlot)
        action.setShortcuts(QKeySequence.StandardKey.Delete)

        action = QAction(
            QgsApplication.getThemeIcon("/mActionCreateTable.svg"),
            QApplication.translate("DBManagerPlugin", "&Create Table…"),
            self,
        )
        mainWindow.registerAction(
            action,
            QApplication.translate("DBManagerPlugin", "&Table"),
            self.createTableActionSlot,
        )
        action = QAction(
            QgsApplication.getThemeIcon("/mActionEditTable.svg"),
            QApplication.translate("DBManagerPlugin", "&Edit Table…"),
            self,
        )
        mainWindow.registerAction(
            action,
            QApplication.translate("DBManagerPlugin", "&Table"),
            self.editTableActionSlot,
        )
        action = QAction(
            QgsApplication.getThemeIcon("/mActionDeleteTable.svg"),
            QApplication.translate("DBManagerPlugin", "&Delete Table/View…"),
            self,
        )
        mainWindow.registerAction(
            action,
            QApplication.translate("DBManagerPlugin", "&Table"),
            self.deleteTableActionSlot,
        )
        action = QAction(
            QApplication.translate("DBManagerPlugin", "&Empty Table…"), self
        )
        mainWindow.registerAction(
            action,
            QApplication.translate("DBManagerPlugin", "&Table"),
            self.emptyTableActionSlot,
        )

        if self.schemas() is not None:
            action = QAction(
                QApplication.translate("DBManagerPlugin", "&Move to Schema"), self
            )
            action.setMenu(QMenu(mainWindow))

            def invoke_callback():
                return mainWindow.invokeCallback(
                    self.prepareMenuMoveTableToSchemaActionSlot
                )

            action.menu().aboutToShow.connect(invoke_callback)
            mainWindow.registerAction(
                action, QApplication.translate("DBManagerPlugin", "&Table")
            )

    def reconnectActionSlot(self, item, action, parent):
        db = item.database()
        db.connection().reconnect()
        db.refresh()

    def deleteActionSlot(self, item, action, parent):
        if isinstance(item, Schema):
            self.deleteSchemaActionSlot(item, action, parent)
        elif isinstance(item, Table):
            self.deleteTableActionSlot(item, action, parent)
        else:
            QApplication.restoreOverrideCursor()
            parent.infoBar.pushMessage(
                QApplication.translate(
                    "DBManagerPlugin", "Cannot delete the selected item."
                ),
                Qgis.MessageLevel.Info,
                parent.iface.messageTimeout(),
            )
            QApplication.setOverrideCursor(Qt.CursorShape.WaitCursor)

    def createSchemaActionSlot(self, item, action, parent):
        QApplication.restoreOverrideCursor()
        try:
            if (
                not isinstance(item, (DBPlugin, Schema, Table))
                or item.database() is None
            ):
                parent.infoBar.pushMessage(
                    QApplication.translate(
                        "DBManagerPlugin",
                        "No database selected or you are not connected to it.",
                    ),
                    Qgis.MessageLevel.Info,
                    parent.iface.messageTimeout(),
                )
                return
            (schema, ok) = QInputDialog.getText(
                parent,
                QApplication.translate("DBManagerPlugin", "New schema"),
                QApplication.translate("DBManagerPlugin", "Enter new schema name"),
            )
            if not ok:
                return
        finally:
            QApplication.setOverrideCursor(Qt.CursorShape.WaitCursor)

        self.createSchema(schema)

    def deleteSchemaActionSlot(self, item, action, parent):
        QApplication.restoreOverrideCursor()
        try:
            if not isinstance(item, Schema):
                parent.infoBar.pushMessage(
                    QApplication.translate(
                        "DBManagerPlugin", "Select an empty schema for deletion."
                    ),
                    Qgis.MessageLevel.Info,
                    parent.iface.messageTimeout(),
                )
                return
            res = QMessageBox.question(
                parent,
                QApplication.translate("DBManagerPlugin", "DB Manager"),
                QApplication.translate(
                    "DBManagerPlugin", "Really delete schema {0}?"
                ).format(item.name),
                QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
            )
            if res != QMessageBox.StandardButton.Yes:
                return
        finally:
            QApplication.setOverrideCursor(Qt.CursorShape.WaitCursor)

        item.delete()

    def schemasFactory(self, row, db):
        return None

    def schemas(self):
        schemas = self.connector.getSchemas()
        if schemas is not None:
            schemas = [self.schemasFactory(x, self) for x in schemas]
        return schemas

    def createSchema(self, name):
        self.connector.createSchema(name)
        self.refresh()

    def createTableActionSlot(self, item, action, parent):
        QApplication.restoreOverrideCursor()
        if not hasattr(item, "database") or item.database() is None:
            parent.infoBar.pushMessage(
                QApplication.translate(
                    "DBManagerPlugin",
                    "No database selected or you are not connected to it.",
                ),
                Qgis.MessageLevel.Info,
                parent.iface.messageTimeout(),
            )
            return
        from ..dlg_create_table import DlgCreateTable

        DlgCreateTable(item, parent).exec()
        QApplication.setOverrideCursor(Qt.CursorShape.WaitCursor)

    def editTableActionSlot(self, item, action, parent):
        QApplication.restoreOverrideCursor()
        try:
            if not isinstance(item, Table) or item.isView:
                parent.infoBar.pushMessage(
                    QApplication.translate(
                        "DBManagerPlugin", "Select a table to edit."
                    ),
                    Qgis.MessageLevel.Info,
                    parent.iface.messageTimeout(),
                )
                return

            if isinstance(item, RasterTable):
                parent.infoBar.pushMessage(
                    QApplication.translate(
                        "DBManagerPlugin", "Editing of raster tables is not supported."
                    ),
                    Qgis.MessageLevel.Info,
                    parent.iface.messageTimeout(),
                )
                return

            from ..dlg_table_properties import DlgTableProperties

            DlgTableProperties(item, parent).exec()
        finally:
            QApplication.setOverrideCursor(Qt.CursorShape.WaitCursor)

    def deleteTableActionSlot(self, item, action, parent):
        QApplication.restoreOverrideCursor()
        try:
            if not isinstance(item, Table):
                parent.infoBar.pushMessage(
                    QApplication.translate(
                        "DBManagerPlugin", "Select a table/view for deletion."
                    ),
                    Qgis.MessageLevel.Info,
                    parent.iface.messageTimeout(),
                )
                return
            res = QMessageBox.question(
                parent,
                QApplication.translate("DBManagerPlugin", "DB Manager"),
                QApplication.translate(
                    "DBManagerPlugin", "Really delete table/view {0}?"
                ).format(item.name),
                QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
            )
            if res != QMessageBox.StandardButton.Yes:
                return
        finally:
            QApplication.setOverrideCursor(Qt.CursorShape.WaitCursor)

        item.delete()

    def emptyTableActionSlot(self, item, action, parent):
        QApplication.restoreOverrideCursor()
        try:
            if not isinstance(item, Table) or item.isView:
                parent.infoBar.pushMessage(
                    QApplication.translate(
                        "DBManagerPlugin", "Select a table to empty it."
                    ),
                    Qgis.MessageLevel.Info,
                    parent.iface.messageTimeout(),
                )
                return
            res = QMessageBox.question(
                parent,
                QApplication.translate("DBManagerPlugin", "DB Manager"),
                QApplication.translate(
                    "DBManagerPlugin", "Really delete all items from table {0}?"
                ).format(item.name),
                QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
            )
            if res != QMessageBox.StandardButton.Yes:
                return
        finally:
            QApplication.setOverrideCursor(Qt.CursorShape.WaitCursor)

        item.empty()

    def prepareMenuMoveTableToSchemaActionSlot(self, item, menu, mainWindow):
        """populate menu with schemas"""

        def slot(x):
            return lambda: mainWindow.invokeCallback(
                self.moveTableToSchemaActionSlot, x
            )

        menu.clear()
        for schema in self.schemas():
            menu.addAction(schema.name, slot(schema))

    def moveTableToSchemaActionSlot(self, item, action, parent, new_schema):
        QApplication.restoreOverrideCursor()
        try:
            if not isinstance(item, Table):
                parent.infoBar.pushMessage(
                    QApplication.translate("DBManagerPlugin", "Select a table/view."),
                    Qgis.MessageLevel.Info,
                    parent.iface.messageTimeout(),
                )
                return
        finally:
            QApplication.setOverrideCursor(Qt.CursorShape.WaitCursor)

        item.moveToSchema(new_schema)

    def tablesFactory(self, row, db, schema=None):
        typ, row = row[0], row[1:]
        if typ == Table.VectorType:
            return self.vectorTablesFactory(row, db, schema)
        elif typ == Table.RasterType:
            return self.rasterTablesFactory(row, db, schema)
        return self.dataTablesFactory(row, db, schema)

    def dataTablesFactory(self, row, db, schema=None):
        return None

    def vectorTablesFactory(self, row, db, schema=None):
        return None

    def rasterTablesFactory(self, row, db, schema=None):
        return None

    def tables(self, schema=None, sys_tables=False):
        tables = self.connector.getTables(schema.name if schema else None, sys_tables)
        if tables is not None:
            ret = [self.tablesFactory(t, self, schema) for t in tables]
        return ret

    def createTable(self, table, fields, schema=None):
        field_defs = [x.definition() for x in fields]
        pkeys = [x for x in fields if x.primaryKey]
        pk_name = pkeys[0].name if len(pkeys) > 0 else None

        ret = self.connector.createTable((schema, table), field_defs, pk_name)
        if ret is not False:
            # Add comments if any, because definition does not include
            # the comment
            for f in fields:
                if f.comment:
                    self.connector.updateTableColumn(
                        (schema, table), f.name, comment=f.comment
                    )
            self.refresh()
        return ret

    def createVectorTable(self, table, fields, geom, schema=None):
        ret = self.createTable(table, fields, schema)
        if not ret:
            return False

        try:
            createGeomCol = geom is not None
            if createGeomCol:
                geomCol, geomType, geomSrid, geomDim = geom[:4]
                createSpatialIndex = geom[4] if len(geom) > 4 else False

                self.connector.addGeometryColumn(
                    (schema, table), geomCol, geomType, geomSrid, geomDim
                )

                if createSpatialIndex:
                    # commit data definition changes, otherwise index can't be built
                    self.connector._commit()
                    self.connector.createSpatialIndex((schema, table), geomCol)

        finally:
            self.refresh()
        return True

    def explicitSpatialIndex(self):
        return False

    def spatialIndexClause(self, src_table, src_column, dest_table, dest_table_column):
        return None

    def hasLowercaseFieldNamesOption(self):
        return False


class Schema(DbItemObject):

    def __init__(self, db):
        DbItemObject.__init__(self, db)
        self.oid = self.name = self.owner = self.perms = None
        self.comment = None
        self.tableCount = 0

    def __del__(self):
        pass  # print "Schema.__del__", self

    def database(self):
        return self.parent()

    def schema(self):
        return self

    def tables(self):
        return self.database().tables(self)

    def delete(self):
        self.aboutToChange.emit()
        ret = self.database().connector.deleteSchema(self.name)
        if ret is not False:
            self.deleted.emit()
        return ret

    def rename(self, new_name):
        self.aboutToChange.emit()
        ret = self.database().connector.renameSchema(self.name, new_name)
        if ret is not False:
            self.name = new_name
            # FIXME: refresh triggers
            self.refresh()
        return ret

    def info(self):
        from .info_model import SchemaInfo

        return SchemaInfo(self)


class Table(DbItemObject):
    TableType, VectorType, RasterType = list(range(3))

    def __init__(self, db, schema=None, parent=None):
        DbItemObject.__init__(self, db)
        self._schema = schema
        if hasattr(self, "type"):
            return
        self.type = Table.TableType

        self.name = self.isView = self.owner = self.pages = None
        self.comment = None
        self.rowCount = None

        self._fields = self._indexes = self._constraints = self._triggers = (
            self._rules
        ) = None

    def __del__(self):
        pass  # print "Table.__del__", self

    def canBeAddedToCanvas(self):
        return True

    def database(self):
        return self.parent()

    def schema(self):
        return self._schema

    def schemaName(self):
        return self.schema().name if self.schema() else None

    def quotedName(self):
        return self.database().connector.quoteId((self.schemaName(), self.name))

    def delete(self):
        self.aboutToChange.emit()
        if self.isView:
            ret = self.database().connector.deleteView((self.schemaName(), self.name))
        else:
            ret = self.database().connector.deleteTable((self.schemaName(), self.name))
        if ret is not False:
            self.deleted.emit()
        return ret

    def rename(self, new_name):
        self.aboutToChange.emit()
        ret = self.database().connector.renameTable(
            (self.schemaName(), self.name), new_name
        )
        if ret is not False:
            self.name = new_name
            self._triggers = None
            self._rules = None
            self._constraints = None
            self.refresh()
        return ret

    def empty(self):
        self.aboutToChange.emit()
        ret = self.database().connector.emptyTable((self.schemaName(), self.name))
        if ret is not False:
            self.refreshRowCount()
        return ret

    def moveToSchema(self, schema):
        self.aboutToChange.emit()
        if self.schema() == schema:
            return True
        ret = self.database().connector.moveTableToSchema(
            (self.schemaName(), self.name), schema.name
        )
        if ret is not False:
            self.schema().refresh()
            schema.refresh()
        return ret

    def info(self):
        from .info_model import TableInfo

        return TableInfo(self)

    def uri(self):
        uri = self.database().uri()
        schema = self.schemaName() if self.schemaName() else ""
        geomCol = (
            self.geomColumn if self.type in [Table.VectorType, Table.RasterType] else ""
        )
        uniqueCol = self.getValidQgisUniqueFields(True) if self.isView else None
        uri.setDataSource(
            schema,
            self.name,
            geomCol if geomCol else None,
            None,
            uniqueCol.name if uniqueCol else "",
        )
        return uri

    def crs(self):
        """Returns the CRS of this table or an invalid CRS if this is not a spatial table
        This should be overwritten by any additional db plugins"""
        return QgsCoordinateReferenceSystem()

    def mimeUri(self):
        layerType = "raster" if self.type == Table.RasterType else "vector"
        return "{}:{}:{}:{}".format(
            layerType,
            self.database().dbplugin().providerName(),
            self.name,
            self.uri().uri(False),
        )

    def toMapLayer(self, geometryType=None, crs=None):
        provider = self.database().dbplugin().providerName()
        dataSourceUri = self.uri()
        if geometryType:
            dataSourceUri.setWkbType(QgsWkbTypes.parseType(geometryType))

        if crs:
            dataSourceUri.setSrid(str(crs.postgisSrid()))

        uri = dataSourceUri.uri(False)
        if self.type == Table.RasterType:
            return QgsRasterLayer(uri, self.name, provider)
        return QgsVectorLayer(uri, self.name, provider)

    def geometryType(self):
        pass

    def getValidQgisUniqueFields(self, onlyOne=False):
        """list of fields valid to load the table as layer in QGIS canvas.
        QGIS automatically search for a valid unique field, so it's
        needed only for queries and views"""

        ret = []

        # add the pk
        pkcols = [x for x in self.fields() if x.primaryKey]
        if len(pkcols) == 1:
            ret.append(pkcols[0])

        # then add both oid, serial and int fields with an unique index
        indexes = self.indexes()
        if indexes is not None:
            for idx in indexes:
                if idx.isUnique and len(idx.columns) == 1:
                    fld = idx.fields()[idx.columns[0]]
                    if (
                        fld.dataType in ["oid", "serial", "int4", "int8"]
                        and fld not in ret
                    ):
                        ret.append(fld)

        # and finally append the other suitable fields
        for fld in self.fields():
            if fld.dataType in ["oid", "serial", "int4", "int8"] and fld not in ret:
                ret.append(fld)

        if onlyOne:
            return ret[0] if len(ret) > 0 else None
        return ret

    def tableDataModel(self, parent):
        pass

    def tableFieldsFactory(self, row, table):
        raise NotImplementedError("Needs to be implemented by subclasses")

    def fields(self):
        if self._fields is None:
            fields = self.database().connector.getTableFields(
                (self.schemaName(), self.name)
            )
            if fields is not None:
                self._fields = [self.tableFieldsFactory(x, self) for x in fields]
        return self._fields

    def refreshFields(self):
        self._fields = None  # refresh table fields
        self.refresh()

    def addField(self, fld):
        self.aboutToChange.emit()
        ret = self.database().connector.addTableColumn(
            (self.schemaName(), self.name), fld.definition()
        )
        if ret is not False:
            self.refreshFields()
        return ret

    def deleteField(self, fld):
        self.aboutToChange.emit()
        ret = self.database().connector.deleteTableColumn(
            (self.schemaName(), self.name), fld.name
        )
        if ret is not False:
            self.refreshFields()
            self.refreshConstraints()
            self.refreshIndexes()
        return ret

    def addGeometryColumn(self, geomCol, geomType, srid, dim, createSpatialIndex=False):
        self.aboutToChange.emit()
        ret = self.database().connector.addGeometryColumn(
            (self.schemaName(), self.name), geomCol, geomType, srid, dim
        )
        if not ret:
            return False

        try:
            if createSpatialIndex:
                # commit data definition changes, otherwise index can't be built
                self.database().connector._commit()
                self.database().connector.createSpatialIndex(
                    (self.schemaName(), self.name), geomCol
                )

        finally:
            (
                self.schema().refresh() if self.schema() else self.database().refresh()
            )  # another table was added
        return True

    def tableConstraintsFactory(self):
        return None

    def constraints(self):
        if self._constraints is None:
            constraints = self.database().connector.getTableConstraints(
                (self.schemaName(), self.name)
            )
            if constraints is not None:
                self._constraints = [
                    self.tableConstraintsFactory(x, self) for x in constraints
                ]
        return self._constraints

    def refreshConstraints(self):
        self._constraints = None  # refresh table constraints
        self.refresh()

    def addConstraint(self, constr):
        self.aboutToChange.emit()
        if constr.type == TableConstraint.TypePrimaryKey:
            ret = self.database().connector.addTablePrimaryKey(
                (self.schemaName(), self.name), constr.fields()[constr.columns[0]].name
            )
        elif constr.type == TableConstraint.TypeUnique:
            ret = self.database().connector.addTableUniqueConstraint(
                (self.schemaName(), self.name), constr.fields()[constr.columns[0]].name
            )
        else:
            return False
        if ret is not False:
            self.refreshConstraints()
        return ret

    def deleteConstraint(self, constr):
        self.aboutToChange.emit()
        ret = self.database().connector.deleteTableConstraint(
            (self.schemaName(), self.name), constr.name
        )
        if ret is not False:
            self.refreshConstraints()
        return ret

    def tableIndexesFactory(self):
        return None

    def indexes(self):
        if self._indexes is None:
            indexes = self.database().connector.getTableIndexes(
                (self.schemaName(), self.name)
            )
            if indexes is not None:
                self._indexes = [self.tableIndexesFactory(x, self) for x in indexes]
        return self._indexes

    def refreshIndexes(self):
        self._indexes = None  # refresh table indexes
        self.refresh()

    def addIndex(self, idx):
        self.aboutToChange.emit()
        ret = self.database().connector.createTableIndex(
            (self.schemaName(), self.name), idx.name, idx.fields()[idx.columns[0]].name
        )
        if ret is not False:
            self.refreshIndexes()
        return ret

    def deleteIndex(self, idx):
        self.aboutToChange.emit()
        ret = self.database().connector.deleteTableIndex(
            (self.schemaName(), self.name), idx.name
        )
        if ret is not False:
            self.refreshIndexes()
        return ret

    def tableTriggersFactory(self, row, table):
        return None

    def triggers(self):
        if self._triggers is None:
            triggers = self.database().connector.getTableTriggers(
                (self.schemaName(), self.name)
            )
            if triggers is not None:
                self._triggers = [self.tableTriggersFactory(x, self) for x in triggers]
        return self._triggers

    def refreshTriggers(self):
        self._triggers = None  # refresh table triggers
        self.refresh()

    def tableRulesFactory(self, row, table):
        return None

    def rules(self):
        if self._rules is None:
            rules = self.database().connector.getTableRules(
                (self.schemaName(), self.name)
            )
            if rules is not None:
                self._rules = [self.tableRulesFactory(x, self) for x in rules]
        return self._rules

    def refreshRules(self):
        self._rules = None  # refresh table rules
        self.refresh()

    def refreshRowCount(self):
        self.aboutToChange.emit()
        prevRowCount = self.rowCount
        try:
            self.rowCount = self.database().connector.getTableRowCount(
                (self.schemaName(), self.name)
            )
            self.rowCount = int(self.rowCount) if self.rowCount is not None else None
        except DbError:
            self.rowCount = None
        if self.rowCount != prevRowCount:
            self.refresh()

    def runAction(self, action):
        action = str(action)

        if action.startswith("rows/"):
            if action == "rows/count":
                self.refreshRowCount()
                return True

        elif action.startswith("triggers/"):
            parts = action.split("/")
            trigger_action = parts[1]

            msg = QApplication.translate(
                "DBManagerPlugin", "Do you want to {0} all triggers?"
            ).format(trigger_action)
            QApplication.restoreOverrideCursor()
            try:
                if (
                    QMessageBox.question(
                        None,
                        QApplication.translate("DBManagerPlugin", "Table triggers"),
                        msg,
                        QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
                    )
                    == QMessageBox.StandardButton.No
                ):
                    return False
            finally:
                QApplication.setOverrideCursor(Qt.CursorShape.WaitCursor)

            if trigger_action == "enable" or trigger_action == "disable":
                enable = trigger_action == "enable"
                self.aboutToChange.emit()
                self.database().connector.enableAllTableTriggers(
                    enable, (self.schemaName(), self.name)
                )
                self.refreshTriggers()
                return True

        elif action.startswith("trigger/"):
            parts = action.split("/")
            trigger_name = parts[1]
            trigger_action = parts[2]

            msg = QApplication.translate(
                "DBManagerPlugin", "Do you want to {0} trigger {1}?"
            ).format(trigger_action, trigger_name)
            QApplication.restoreOverrideCursor()
            try:
                if (
                    QMessageBox.question(
                        None,
                        QApplication.translate("DBManagerPlugin", "Table trigger"),
                        msg,
                        QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
                    )
                    == QMessageBox.StandardButton.No
                ):
                    return False
            finally:
                QApplication.setOverrideCursor(Qt.CursorShape.WaitCursor)

            if trigger_action == "delete":
                self.aboutToChange.emit()
                self.database().connector.deleteTableTrigger(
                    trigger_name, (self.schemaName(), self.name)
                )
                self.refreshTriggers()
                return True

            elif trigger_action == "enable" or trigger_action == "disable":
                enable = trigger_action == "enable"
                self.aboutToChange.emit()
                self.database().connector.enableTableTrigger(
                    trigger_name, enable, (self.schemaName(), self.name)
                )
                self.refreshTriggers()
                return True

        return False

    def addExtraContextMenuEntries(self, menu):
        """Called whenever a context menu is shown for this table. Can be used to add additional actions to the menu."""
        pass


class VectorTable(Table):

    def __init__(self, db, schema=None, parent=None):
        if not hasattr(
            self, "type"
        ):  # check if the superclass constructor was called yet!
            Table.__init__(self, db, schema, parent)
        self.type = Table.VectorType
        self.geomColumn = self.geomType = self.geomDim = self.srid = None
        self.estimatedExtent = self.extent = None

    def info(self):
        from .info_model import VectorTableInfo

        return VectorTableInfo(self)

    def uri(self):
        uri = super().uri()
        for f in self.fields():
            if f.primaryKey:
                uri.setKeyColumn(f.name)
                break
        uri.setWkbType(QgsWkbTypes.parseType(self.geomType))
        return uri

    def hasSpatialIndex(self, geom_column=None):
        geom_column = geom_column if geom_column is not None else self.geomColumn
        fld = None
        for fld in self.fields():
            if fld.name == geom_column:
                break
        if fld is None:
            return False

        for idx in self.indexes():
            if fld.num in idx.columns:
                return True
        return False

    def createSpatialIndex(self, geom_column=None):
        self.aboutToChange.emit()
        geom_column = geom_column if geom_column is not None else self.geomColumn
        ret = self.database().connector.createSpatialIndex(
            (self.schemaName(), self.name), geom_column
        )
        if ret is not False:
            self.refreshIndexes()
        return ret

    def deleteSpatialIndex(self, geom_column=None):
        self.aboutToChange.emit()
        geom_column = geom_column if geom_column is not None else self.geomColumn
        ret = self.database().connector.deleteSpatialIndex(
            (self.schemaName(), self.name), geom_column
        )
        if ret is not False:
            self.refreshIndexes()
        return ret

    def refreshTableExtent(self):
        prevExtent = self.extent
        try:
            self.extent = self.database().connector.getTableExtent(
                (self.schemaName(), self.name), self.geomColumn
            )
        except DbError:
            self.extent = None
        if self.extent != prevExtent:
            self.refresh()

    def refreshTableEstimatedExtent(self):
        prevEstimatedExtent = self.estimatedExtent
        try:
            self.estimatedExtent = self.database().connector.getTableEstimatedExtent(
                (self.schemaName(), self.name), self.geomColumn
            )
        except DbError:
            self.estimatedExtent = None
        if self.estimatedExtent != prevEstimatedExtent:
            self.refresh()

    def runAction(self, action):
        action = str(action)

        if action.startswith("spatialindex/"):
            parts = action.split("/")
            spatialIndex_action = parts[1]

            msg = QApplication.translate(
                "DBManagerPlugin", "Do you want to {0} spatial index for field {1}?"
            ).format(spatialIndex_action, self.geomColumn)
            QApplication.restoreOverrideCursor()
            try:
                if (
                    QMessageBox.question(
                        None,
                        QApplication.translate("DBManagerPlugin", "Spatial Index"),
                        msg,
                        QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
                    )
                    == QMessageBox.StandardButton.No
                ):
                    return False
            finally:
                QApplication.setOverrideCursor(Qt.CursorShape.WaitCursor)

            if spatialIndex_action == "create":
                self.createSpatialIndex()
                return True
            elif spatialIndex_action == "delete":
                self.deleteSpatialIndex()
                return True

        if action.startswith("extent/"):
            if action == "extent/get":
                self.refreshTableExtent()
                return True

            if action == "extent/estimated/get":
                self.refreshTableEstimatedExtent()
                return True

        return Table.runAction(self, action)

    def addLayer(self, geometryType=None, crs=None):
        layer = self.toMapLayer(geometryType, crs)
        layers = QgsProject.instance().addMapLayers([layer])
        if len(layers) != 1:
            QgsMessageLog.logMessage(
                self.tr("{layer} is an invalid layer - not loaded").format(
                    layer=layer.publicSource()
                )
            )
            msgLabel = QLabel(
                self.tr(
                    '{layer} is an invalid layer and cannot be loaded. Please check the <a href="#messageLog">message log</a> for further info.'
                ).format(layer=layer.publicSource()),
                self.mainWindow.infoBar,
            )
            msgLabel.setWordWrap(True)
            msgLabel.linkActivated.connect(
                self.mainWindow.iface.mainWindow().findChild(QWidget, "MessageLog").show
            )
            msgLabel.linkActivated.connect(self.mainWindow.iface.mainWindow().raise_)
            self.mainWindow.infoBar.pushItem(
                QgsMessageBarItem(msgLabel, Qgis.MessageLevel.Warning)
            )

    def showAdvancedVectorDialog(self):
        dlg = QDialog()
        dlg.setObjectName("dbManagerAdvancedVectorDialog")
        settings = QgsSettings()
        dlg.restoreGeometry(
            settings.value(
                "/DB_Manager/advancedAddDialog/geometry", QByteArray(), type=QByteArray
            )
        )
        layout = QFormLayout()
        dlg.setLayout(layout)
        dlg.setWindowTitle(self.tr("Add Layer {}").format(self.name))
        geometryTypeComboBox = QComboBox()
        geometryTypeComboBox.addItem(self.tr("Point"), "POINT")
        geometryTypeComboBox.addItem(self.tr("Line"), "LINESTRING")
        geometryTypeComboBox.addItem(self.tr("Polygon"), "POLYGON")
        layout.addRow(self.tr("Geometry Type"), geometryTypeComboBox)
        zCheckBox = QCheckBox(self.tr("With Z"))
        mCheckBox = QCheckBox(self.tr("With M"))
        layout.addRow(zCheckBox)
        layout.addRow(mCheckBox)
        crsSelector = QgsProjectionSelectionWidget()
        crsSelector.setCrs(self.crs())
        layout.addRow(self.tr("CRS"), crsSelector)

        def selectedGeometryType():
            geomType = geometryTypeComboBox.currentData()
            if zCheckBox.isChecked():
                geomType += "Z"
            if mCheckBox.isChecked():
                geomType += "M"

            return geomType

        def selectedCrs():
            return crsSelector.crs()

        addButton = QPushButton(self.tr("Load Layer"))
        addButton.clicked.connect(
            lambda: self.addLayer(selectedGeometryType(), selectedCrs())
        )
        btns = QDialogButtonBox(QDialogButtonBox.StandardButton.Cancel)
        btns.addButton(addButton, QDialogButtonBox.ButtonRole.ActionRole)

        layout.addRow(btns)

        addButton.clicked.connect(dlg.accept)
        btns.accepted.connect(dlg.accept)
        btns.rejected.connect(dlg.reject)

        dlg.exec()

        settings = QgsSettings()
        settings.setValue("/DB_Manager/advancedAddDialog/geometry", dlg.saveGeometry())

    def addExtraContextMenuEntries(self, menu):
        """Called whenever a context menu is shown for this table. Can be used to add additional actions to the menu."""

        if self.geomType == "GEOMETRY":
            menu.addAction(
                QApplication.translate("DBManagerPlugin", "Add Layer (Advanced)…"),
                self.showAdvancedVectorDialog,
            )


class RasterTable(Table):

    def __init__(self, db, schema=None, parent=None):
        if not hasattr(
            self, "type"
        ):  # check if the superclass constructor was called yet!
            Table.__init__(self, db, schema, parent)
        self.type = Table.RasterType
        self.geomColumn = self.geomType = self.pixelSizeX = self.pixelSizeY = (
            self.pixelType
        ) = self.isExternal = self.srid = None
        self.extent = None

    def info(self):
        from .info_model import RasterTableInfo

        return RasterTableInfo(self)


class TableSubItemObject(QObject):

    def __init__(self, table):
        QObject.__init__(self, table)

    def table(self):
        return self.parent()

    def database(self):
        return self.table().database() if self.table() else None


class TableField(TableSubItemObject):

    def __init__(self, table):
        TableSubItemObject.__init__(self, table)
        self.num = self.name = self.dataType = self.modifier = self.notNull = (
            self.default
        ) = self.hasDefault = self.primaryKey = None
        self.comment = None

    def type2String(self):
        if self.modifier is None or self.modifier == -1:
            return "%s" % self.dataType
        return f"{self.dataType} ({self.modifier})"

    def default2String(self):
        if not self.hasDefault:
            return ""
        return self.default if self.default is not None else "NULL"

    def definition(self):
        from .connector import DBConnector

        quoteIdFunc = (
            self.database().connector.quoteId
            if self.database()
            else DBConnector.quoteId
        )

        name = quoteIdFunc(self.name)
        not_null = "NOT NULL" if self.notNull else ""

        txt = f"{name} {self.type2String()} {not_null}"
        if self.hasDefault:
            txt += " DEFAULT %s" % self.default2String()
        return txt

    def getComment(self):
        """Returns the comment for a field"""
        return ""

    def delete(self):
        return self.table().deleteField(self)

    def rename(self, new_name):
        return self.update(new_name)

    def update(
        self,
        new_name,
        new_type_str=None,
        new_not_null=None,
        new_default_str=None,
        new_comment=None,
    ):
        self.table().aboutToChange.emit()
        if self.name == new_name:
            new_name = None
        if self.type2String() == new_type_str:
            new_type_str = None
        if self.notNull == new_not_null:
            new_not_null = None
        if self.default2String() == new_default_str:
            new_default_str = None
        if self.comment == new_comment:
            new_comment = None
        ret = (
            self.table()
            .database()
            .connector.updateTableColumn(
                (self.table().schemaName(), self.table().name),
                self.name,
                new_name,
                new_type_str,
                new_not_null,
                new_default_str,
                new_comment,
            )
        )
        if ret is not False:
            self.table().refreshFields()
        return ret


class TableConstraint(TableSubItemObject):
    """class that represents a constraint of a table (relation)"""

    (
        TypeCheck,
        TypeForeignKey,
        TypePrimaryKey,
        TypeUnique,
        TypeExclusion,
        TypeUnknown,
    ) = list(range(6))
    types = {
        "c": TypeCheck,
        "f": TypeForeignKey,
        "p": TypePrimaryKey,
        "u": TypeUnique,
        "x": TypeExclusion,
    }

    onAction = {
        "a": "NO ACTION",
        "r": "RESTRICT",
        "c": "CASCADE",
        "n": "SET NULL",
        "d": "SET DEFAULT",
    }
    matchTypes = {"u": "UNSPECIFIED", "f": "FULL", "p": "PARTIAL", "s": "SIMPLE"}

    def __init__(self, table):
        TableSubItemObject.__init__(self, table)
        self.name = self.type = self.columns = None

    def type2String(self):
        if self.type == TableConstraint.TypeCheck:
            return QApplication.translate("DBManagerPlugin", "Check")
        if self.type == TableConstraint.TypePrimaryKey:
            return QApplication.translate("DBManagerPlugin", "Primary key")
        if self.type == TableConstraint.TypeForeignKey:
            return QApplication.translate("DBManagerPlugin", "Foreign key")
        if self.type == TableConstraint.TypeUnique:
            return QApplication.translate("DBManagerPlugin", "Unique")
        if self.type == TableConstraint.TypeExclusion:
            return QApplication.translate("DBManagerPlugin", "Exclusion")
        return QApplication.translate("DBManagerPlugin", "Unknown")

    def fields(self):
        def fieldFromNum(num, fields):
            """return field specified by its number or None if doesn't exist"""
            for fld in fields:
                if fld.num == num:
                    return fld
            return None

        fields = self.table().fields()
        cols = {}
        for num in self.columns:
            cols[num] = fieldFromNum(num, fields)
        return cols

    def delete(self):
        return self.table().deleteConstraint(self)


class TableIndex(TableSubItemObject):

    def __init__(self, table):
        TableSubItemObject.__init__(self, table)
        self.name = self.columns = self.isUnique = None

    def fields(self):
        def fieldFromNum(num, fields):
            """return field specified by its number or None if doesn't exist"""
            for fld in fields:
                if fld.num == num:
                    return fld
            return None

        fields = self.table().fields()
        cols = {}
        for num in self.columns:
            cols[num] = fieldFromNum(num, fields)
        return cols

    def delete(self):
        return self.table().deleteIndex(self)


class TableTrigger(TableSubItemObject):
    """class that represents a trigger"""

    # Bits within tgtype (pg_trigger.h)
    TypeRow = 1 << 0  # row or statement
    TypeBefore = 1 << 1  # before or after
    # events: one or more
    TypeInsert = 1 << 2
    TypeDelete = 1 << 3
    TypeUpdate = 1 << 4
    TypeTruncate = 1 << 5

    def __init__(self, table):
        TableSubItemObject.__init__(self, table)
        self.name = self.function = None

    def type2String(self):
        trig_type = ""
        trig_type += "Before " if self.type & TableTrigger.TypeBefore else "After "
        if self.type & TableTrigger.TypeInsert:
            trig_type += "INSERT "
        if self.type & TableTrigger.TypeUpdate:
            trig_type += "UPDATE "
        if self.type & TableTrigger.TypeDelete:
            trig_type += "DELETE "
        if self.type & TableTrigger.TypeTruncate:
            trig_type += "TRUNCATE "
        trig_type += "\n"
        trig_type += "for each "
        trig_type += "row" if self.type & TableTrigger.TypeRow else "statement"
        return trig_type


class TableRule(TableSubItemObject):

    def __init__(self, table):
        TableSubItemObject.__init__(self, table)
        self.name = self.definition = None
