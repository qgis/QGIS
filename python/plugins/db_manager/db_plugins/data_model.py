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

from qgis.PyQt.QtCore import (
    Qt,
    QElapsedTimer,
    QRegularExpression,
    QAbstractTableModel,
    pyqtSignal,
    QObject,
)
from qgis.PyQt.QtGui import QFont, QStandardItemModel, QStandardItem
from qgis.PyQt.QtWidgets import QApplication

from qgis.core import QgsTask

from .plugin import DbError, BaseError


class BaseTableModel(QAbstractTableModel):

    def __init__(self, header=None, data=None, parent=None):
        QAbstractTableModel.__init__(self, parent)
        self._header = header if header else []
        self.resdata = data if data else []

    def headerToString(self, sep="\t"):
        header = self._header
        return sep.join(header)

    def rowToString(self, row, sep="\t"):
        return sep.join(
            str(self.getData(row, col)) for col in range(self.columnCount())
        )

    def getData(self, row, col):
        return self.resdata[row][col]

    def columnNames(self):
        return list(self._header)

    def rowCount(self, parent=None):
        return len(self.resdata)

    def columnCount(self, parent=None):
        return len(self._header)

    def data(self, index, role):
        if role not in [
            Qt.ItemDataRole.DisplayRole,
            Qt.ItemDataRole.EditRole,
            Qt.ItemDataRole.FontRole,
        ]:
            return None

        val = self.getData(index.row(), index.column())

        if role == Qt.ItemDataRole.EditRole:
            return val

        if role == Qt.ItemDataRole.FontRole:  # draw NULL in italic
            if val is not None:
                return None
            f = QFont()
            f.setItalic(True)
            return f

        if val is None:
            return "NULL"
        elif isinstance(val, memoryview):
            # hide binary data
            return None
        elif isinstance(val, str) and len(val) > 300:
            # too much data to display, elide the string
            val = val[:300]
        try:
            return str(val)  # convert to Unicode
        except UnicodeDecodeError:
            return str(
                val, "utf-8", "replace"
            )  # convert from utf8 and replace errors (if any)

    def headerData(self, section, orientation, role):
        if role != Qt.ItemDataRole.DisplayRole:
            return None

        if orientation == Qt.Orientation.Vertical:
            # header for a row
            return section + 1
        else:
            # header for a column
            return self._header[section]


class TableDataModel(BaseTableModel):

    def __init__(self, table, parent=None):
        self.db = table.database().connector
        self.table = table

        fieldNames = [x.name for x in table.fields()]
        BaseTableModel.__init__(self, fieldNames, None, parent)

        # get table fields
        self.fields = []
        for fld in table.fields():
            self.fields.append(self._sanitizeTableField(fld))

        self.fetchedCount = 201
        self.fetchedFrom = (
            -self.fetchedCount - 1
        )  # so the first call to getData will exec fetchMoreData(0)

    def _sanitizeTableField(self, field):
        """quote column names to avoid some problems (e.g. columns with upper case)"""
        return self.db.quoteId(field)

    def getData(self, row, col):
        if row < self.fetchedFrom or row >= self.fetchedFrom + self.fetchedCount:
            margin = self.fetchedCount / 2
            start = int(
                self.rowCount() - margin
                if row + margin >= self.rowCount()
                else row - margin
            )
            if start < 0:
                start = 0
            self.fetchMoreData(start)
        return self.resdata[row - self.fetchedFrom][col]

    def fetchMoreData(self, row_start):
        pass

    def rowCount(self, index=None):
        # case for tables with no columns ... any reason to use them? :-)
        return (
            self.table.rowCount
            if self.table.rowCount is not None and self.columnCount(index) > 0
            else 0
        )


class SqlResultModelAsync(QObject):
    done = pyqtSignal()

    def __init__(self):
        super().__init__()
        self.error = BaseError("")
        self.status = None
        self.model = None
        self.task = None
        self.canceled = False

    def cancel(self):
        self.canceled = True
        if self.task:
            self.task.cancel()

    def modelDone(self):
        if self.task:
            self.status = self.task.status
            self.model = self.task.model
            self.error = self.task.error

        self.done.emit()


class SqlResultModelTask(QgsTask):

    def __init__(self, db, sql, parent):
        super().__init__(
            description=QApplication.translate("DBManagerPlugin", "Executing SQL")
        )
        self.db = db
        self.sql = sql
        self.parent = parent
        self.error = BaseError("")
        self.model = None


class SqlResultModel(BaseTableModel):

    def __init__(self, db, sql, parent=None):
        self.db = db.connector

        t = QElapsedTimer()
        t.start()
        c = self.db._execute(None, sql)

        self._affectedRows = 0
        data = []
        header = self.db._get_cursor_columns(c)
        if header is None:
            header = []

        try:
            if len(header) > 0:
                data = self.db._fetchall(c)
            self._affectedRows = len(data)
        except DbError:
            # nothing to fetch!
            data = []
            header = []

        super().__init__(header, data, parent)

        # commit before closing the cursor to make sure that the changes are stored
        self.db._commit()
        c.close()
        self._secs = t.elapsed() / 1000.0
        del c
        del t

    def secs(self):
        return self._secs

    def affectedRows(self):
        return self._affectedRows


class SimpleTableModel(QStandardItemModel):

    def __init__(self, header, editable=False, parent=None):
        self.header = header
        self.editable = editable
        QStandardItemModel.__init__(self, 0, len(self.header), parent)

    def rowFromData(self, data):
        row = []
        for c in data:
            item = QStandardItem(str(c))
            item.setFlags(
                (item.flags() | Qt.ItemFlag.ItemIsEditable)
                if self.editable
                else (item.flags() & ~Qt.ItemFlag.ItemIsEditable)
            )
            row.append(item)
        return row

    def headerData(self, section, orientation, role):
        if (
            orientation == Qt.Orientation.Horizontal
            and role == Qt.ItemDataRole.DisplayRole
        ):
            return self.header[section]
        return None

    def _getNewObject(self):
        pass

    def getObject(self, row):
        return self._getNewObject()

    def getObjectIter(self):
        for row in range(self.rowCount()):
            yield self.getObject(row)


class TableFieldsModel(SimpleTableModel):

    def __init__(self, parent, editable=False):
        SimpleTableModel.__init__(
            self, ["Name", "Type", "Null", "Default", "Comment"], editable, parent
        )

    def headerData(self, section, orientation, role):
        if (
            orientation == Qt.Orientation.Vertical
            and role == Qt.ItemDataRole.DisplayRole
        ):
            return section + 1
        return SimpleTableModel.headerData(self, section, orientation, role)

    def flags(self, index):
        flags = SimpleTableModel.flags(self, index)
        if (
            index.column() == 2 and flags & Qt.ItemFlag.ItemIsEditable
        ):  # set Null column as checkable instead of editable
            flags = (
                flags & ~Qt.ItemFlag.ItemIsEditable | Qt.ItemFlag.ItemIsUserCheckable
            )
        return flags

    def append(self, fld):
        data = [
            fld.name,
            fld.type2String(),
            not fld.notNull,
            fld.default2String(),
            fld.getComment(),
        ]
        self.appendRow(self.rowFromData(data))
        row = self.rowCount() - 1
        self.setData(self.index(row, 0), fld, Qt.ItemDataRole.UserRole)
        self.setData(self.index(row, 1), fld.primaryKey, Qt.ItemDataRole.UserRole)
        self.setData(self.index(row, 2), None, Qt.ItemDataRole.DisplayRole)
        self.setData(
            self.index(row, 2),
            Qt.CheckState.Unchecked if fld.notNull else Qt.CheckState.Checked,
            Qt.ItemDataRole.CheckStateRole,
        )

    def _getNewObject(self):
        from .plugin import TableField

        return TableField(None)

    def getObject(self, row):
        val = self.data(self.index(row, 0), Qt.ItemDataRole.UserRole)
        fld = val if val is not None else self._getNewObject()
        fld.name = self.data(self.index(row, 0)) or ""
        typestr = self.data(self.index(row, 1)) or ""
        regex = QRegularExpression(r"([^\(]+)\(([^\)]+)\)")
        match = regex.match(typestr)
        if match.hasMatch():
            fld.dataType = match.captured(1).strip()
            fld.modifier = match.captured(2).strip()
        else:
            fld.modifier = None
            fld.dataType = typestr

        fld.notNull = (
            self.data(self.index(row, 2), Qt.ItemDataRole.CheckStateRole)
            == Qt.CheckState.Unchecked
        )
        fld.primaryKey = self.data(self.index(row, 1), Qt.ItemDataRole.UserRole)
        fld.comment = self.data(self.index(row, 4))
        return fld

    def getFields(self):
        return [fld for fld in self.getObjectIter()]


class TableConstraintsModel(SimpleTableModel):

    def __init__(self, parent, editable=False):
        SimpleTableModel.__init__(
            self,
            [
                QApplication.translate("DBManagerPlugin", "Name"),
                QApplication.translate("DBManagerPlugin", "Type"),
                QApplication.translate("DBManagerPlugin", "Column(s)"),
            ],
            editable,
            parent,
        )

    def append(self, constr):
        field_names = [str(k_v[1].name) for k_v in iter(list(constr.fields().items()))]
        data = [constr.name, constr.type2String(), ", ".join(field_names)]
        self.appendRow(self.rowFromData(data))
        row = self.rowCount() - 1
        self.setData(self.index(row, 0), constr, Qt.ItemDataRole.UserRole)
        self.setData(self.index(row, 1), constr.type, Qt.ItemDataRole.UserRole)
        self.setData(self.index(row, 2), constr.columns, Qt.ItemDataRole.UserRole)

    def _getNewObject(self):
        from .plugin import TableConstraint

        return TableConstraint(None)

    def getObject(self, row):
        constr = self.data(self.index(row, 0), Qt.ItemDataRole.UserRole)
        if not constr:
            constr = self._getNewObject()
        constr.name = self.data(self.index(row, 0)) or ""
        constr.type = self.data(self.index(row, 1), Qt.ItemDataRole.UserRole)
        constr.columns = self.data(self.index(row, 2), Qt.ItemDataRole.UserRole)
        return constr

    def getConstraints(self):
        return [constr for constr in self.getObjectIter()]


class TableIndexesModel(SimpleTableModel):

    def __init__(self, parent, editable=False):
        SimpleTableModel.__init__(
            self,
            [
                QApplication.translate("DBManagerPlugin", "Name"),
                QApplication.translate("DBManagerPlugin", "Column(s)"),
            ],
            editable,
            parent,
        )

    def append(self, idx):
        field_names = [str(k_v1[1].name) for k_v1 in iter(list(idx.fields().items()))]
        data = [idx.name, ", ".join(field_names)]
        self.appendRow(self.rowFromData(data))
        row = self.rowCount() - 1
        self.setData(self.index(row, 0), idx, Qt.ItemDataRole.UserRole)
        self.setData(self.index(row, 1), idx.columns, Qt.ItemDataRole.UserRole)

    def _getNewObject(self):
        from .plugin import TableIndex

        return TableIndex(None)

    def getObject(self, row):
        idx = self.data(self.index(row, 0), Qt.ItemDataRole.UserRole)
        if not idx:
            idx = self._getNewObject()
        idx.name = self.data(self.index(row, 0))
        idx.columns = self.data(self.index(row, 1), Qt.ItemDataRole.UserRole)
        return idx

    def getIndexes(self):
        return [idx for idx in self.getObjectIter()]
