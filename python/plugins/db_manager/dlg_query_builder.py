"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QGIS
Date                 : March 2015
copyright            : (C) 2015 Hugo Mercier / Oslandia
email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

Query builder dialog, based on the QSpatialite plugin (GPLv2+) by Romain Riviere
"""

from qgis.PyQt import uic
from qgis.PyQt.QtCore import Qt, QObject, QEvent
from qgis.PyQt.QtWidgets import QDialog, QMessageBox, QTextEdit

from .db_plugins.plugin import VectorTable
from .gui_utils import GuiUtils

Ui_Dialog, _ = uic.loadUiType(GuiUtils.get_ui_file_path("DlgQueryBuilder.ui"))


class FocusEventFilter(QObject):

    def __init__(self, parent):
        QObject.__init__(self, parent)
        self.focus = ""

    def eventFilter(self, obj, event):
        if event.type() == QEvent.Type.FocusIn:
            self.focus = obj.objectName()
        return QObject.eventFilter(self, obj, event)


def insertWithSelection(widget, text):
    if widget.textCursor().hasSelection():  # user has selectedsomething...
        selection = widget.textCursor().selectedText()
        widget.insertPlainText(text + selection + ")")
    else:
        widget.insertPlainText(text)


def insertWithSelectionOn(parent, objectname, text):
    """Insert the text in a QTextEdit given by its objectname"""
    w = parent.findChild(QTextEdit, objectname)
    insertWithSelection(w, text)


class QueryBuilderDlg(QDialog):
    # object used to store parameters between invocations
    saveParameter = None

    def __init__(self, iface, db, parent=None, reset=False):
        QDialog.__init__(self, parent)
        self.iface = iface
        self.db = db
        self.query = ""
        self.ui = Ui_Dialog()
        self.ui.setupUi(self)
        self.ui.group.setMaximumHeight(self.ui.tab.sizeHint().height())
        self.ui.order.setMaximumHeight(self.ui.tab.sizeHint().height())

        self.evt = FocusEventFilter(self)
        self.ui.col.installEventFilter(self.evt)
        self.ui.where.installEventFilter(self.evt)
        self.ui.group.installEventFilter(self.evt)
        self.ui.order.installEventFilter(self.evt)

        d = self.db.connector.getQueryBuilderDictionary()
        # Application default parameters
        self.table = None
        self.col_col = []
        self.col_where = []
        self.coltables = []
        self.ui.extract.setChecked(True)
        # ComboBox default values
        self.ui.functions.insertItems(1, d["function"])
        self.ui.math.insertItems(1, d["math"])
        self.ui.aggregates.insertItems(1, d["aggregate"])
        self.ui.operators.insertItems(1, d["operator"])
        self.ui.stringfct.insertItems(1, d["string"])
        # self.ui.Rtree.insertItems(1,rtreecommand)

        # restore last query if needed
        if reset:
            QueryBuilderDlg.saveParameter = None
        if QueryBuilderDlg.saveParameter is not None:
            self.restoreLastQuery()

        # Show Tables
        self.show_tables()

        # Signal/slot
        self.ui.aggregates.currentIndexChanged.connect(self.add_aggregate)
        self.ui.stringfct.currentIndexChanged.connect(self.add_stringfct)
        self.ui.operators.currentIndexChanged.connect(self.add_operators)
        self.ui.functions.currentIndexChanged.connect(self.add_functions)
        self.ui.math.currentIndexChanged.connect(self.add_math)
        self.ui.tables.currentIndexChanged.connect(self.add_tables)
        self.ui.tables.currentIndexChanged.connect(self.list_cols)
        self.ui.columns.currentIndexChanged.connect(self.add_columns)
        self.ui.columns_2.currentIndexChanged.connect(self.list_values)
        self.ui.reset.clicked.connect(self.reset)
        self.ui.extract.stateChanged.connect(self.list_values)
        self.ui.values.doubleClicked.connect(self.query_item)
        self.ui.buttonBox.accepted.connect(self.validate)
        self.ui.checkBox.stateChanged.connect(self.show_tables)

        if self.db.explicitSpatialIndex():
            self.tablesGeo = [
                table for table in self.tables if isinstance(table, VectorTable)
            ]
            tablesGeo = [
                f'"{table.name}"."{table.geomColumn}"' for table in self.tablesGeo
            ]
            self.ui.table_target.insertItems(1, tablesGeo)
            self.idxTables = [
                table for table in self.tablesGeo if table.hasSpatialIndex()
            ]
            idxTables = [
                f'"{table.name}"."{table.geomColumn}"' for table in self.idxTables
            ]
            self.ui.table_idx.insertItems(1, idxTables)

            self.ui.usertree.clicked.connect(self.use_rtree)
        else:
            self.ui.toolBox.setItemEnabled(2, False)

    def update_table_list(self):
        self.tables = []
        add_sys_tables = self.ui.checkBox.isChecked()
        schemas = self.db.schemas()
        if schemas is None:
            self.tables = self.db.tables(None, add_sys_tables)
        else:
            for schema in schemas:
                self.tables += self.db.tables(schema, add_sys_tables)

    def show_tables(self):
        self.update_table_list()
        self.ui.tables.clear()
        self.ui.tables.insertItems(0, ["Tables"])
        self.ui.tables.insertItems(1, [t.name for t in self.tables])

    def add_aggregate(self):
        if self.ui.aggregates.currentIndex() <= 0:
            return
        ag = self.ui.aggregates.currentText()

        insertWithSelection(self.ui.col, ag)

        self.ui.aggregates.setCurrentIndex(0)

    def add_functions(self):
        if self.ui.functions.currentIndex() <= 0:
            return
        ag = self.ui.functions.currentText()

        insertWithSelectionOn(self, self.evt.focus, ag)

        self.ui.functions.setCurrentIndex(0)

    def add_stringfct(self):
        if self.ui.stringfct.currentIndex() <= 0:
            return
        ag = self.ui.stringfct.currentText()

        insertWithSelectionOn(self, self.evt.focus, ag)

        self.ui.stringfct.setCurrentIndex(0)

    def add_math(self):
        if self.ui.math.currentIndex() <= 0:
            return
        ag = self.ui.math.currentText()

        insertWithSelectionOn(self, self.evt.focus, ag)

        self.ui.math.setCurrentIndex(0)

    def add_operators(self):
        if self.ui.operators.currentIndex() <= 0:
            return
        ag = self.ui.operators.currentText()

        if self.evt.focus == "where":  # in where section
            self.ui.where.insertPlainText(ag)
        else:
            self.ui.col.insertPlainText(ag)
        self.ui.operators.setCurrentIndex(0)

    def add_tables(self):
        if self.ui.tables.currentIndex() <= 0:
            return
        ag = self.ui.tables.currentText()
        # Retrieve Table Object from txt
        tableObj = [table for table in self.tables if table.name.upper() == ag.upper()]
        if len(tableObj) != 1:
            return  # No object with this name
        self.table = tableObj[0]
        if ag in self.coltables:  # table already use
            response = QMessageBox.question(
                self,
                "Table already used",
                "Do you want to add table %s again?" % ag,
                QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
            )
            if response == QMessageBox.StandardButton.No:
                return
        ag = self.table.quotedName()
        txt = self.ui.tab.text()
        if (txt is None) or (txt in ("", " ")):
            self.ui.tab.setText("%s" % ag)
        else:
            self.ui.tab.setText(f"{txt}, {ag}")
        self.ui.tables.setCurrentIndex(0)

    def add_columns(self):
        if self.ui.columns.currentIndex() <= 0:
            return
        ag = self.ui.columns.currentText()
        if self.evt.focus == "where":  # in where section
            if ag in self.col_where:  # column already called in where section
                response = QMessageBox.question(
                    self,
                    "Column already used in WHERE clause",
                    "Do you want to add column %s again?" % ag,
                    QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
                )
                if response == QMessageBox.StandardButton.No:
                    self.ui.columns.setCurrentIndex(0)
                    return
            self.ui.where.insertPlainText(ag)
            self.col_where.append(ag)
        elif self.evt.focus == "col":
            if ag in self.col_col:  # column already called in col section
                response = QMessageBox.question(
                    self,
                    "Column already used in COLUMNS section",
                    "Do you want to add column %s again?" % ag,
                    QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
                )
                if response == QMessageBox.StandardButton.No:
                    self.ui.columns.setCurrentIndex(0)
                    return
            if len(self.ui.col.toPlainText().strip()) > 0:
                self.ui.col.insertPlainText(",\n" + ag)
            else:
                self.ui.col.insertPlainText(ag)
            self.col_col.append(ag)
        elif self.evt.focus == "group":
            if len(self.ui.group.toPlainText().strip()) > 0:
                self.ui.group.insertPlainText(", " + ag)
            else:
                self.ui.group.insertPlainText(ag)
        elif self.evt.focus == "order":
            if len(self.ui.order.toPlainText().strip()) > 0:
                self.ui.order.insertPlainText(", " + ag)
            else:
                self.ui.order.insertPlainText(ag)

        self.ui.columns.setCurrentIndex(0)

    def list_cols(self):
        table = self.table
        if table is None:
            return
        if table.name in self.coltables:
            return

        columns = [f'"{table.name}"."{col.name}"' for col in table.fields()]
        # add special '*' column:
        columns = ['"%s".*' % table.name] + columns
        self.coltables.append(table.name)  # table columns have been listed
        # first and second col combobox
        end = self.ui.columns.count()
        self.ui.columns.insertItems(end, columns)
        self.ui.columns_2.insertItems(end, columns)
        end = self.ui.columns.count()
        self.ui.columns.insertSeparator(end)
        self.ui.columns_2.insertSeparator(end)

    def list_values(self):
        if self.ui.columns_2.currentIndex() <= 0:
            return
        item = self.ui.columns_2.currentText()
        # recover column and table:
        column = item.split(".")  # "table".'column'
        table = column[0]
        if column[1] == "*":
            return
        table = table[1:-1]

        qtable = [t for t in self.tables if t.name.lower() == table.lower()][
            0
        ].quotedName()

        if self.ui.extract.isChecked():
            limit = 10
        else:
            limit = None
        model = self.db.columnUniqueValuesModel(item, qtable, limit)
        self.ui.values.setModel(model)

    def query_item(self, index):
        value = index.data(Qt.ItemDataRole.EditRole)

        if value is None:
            queryWord = "NULL"
        elif isinstance(value, (int, float)):
            queryWord = str(value)
        else:
            queryWord = self.db.connector.quoteString(value)

        if queryWord.strip() != "":
            self.ui.where.insertPlainText(" " + queryWord)
            self.ui.where.setFocus()

    def use_rtree(self):
        idx = self.ui.table_idx.currentText()
        if idx in (None, "", " ", "Table (with Spatial Index)"):
            return
        try:
            tab_idx = idx.split(".")[0][1:-1]  # remove "
            col_idx = idx.split(".")[1][1:-1]  # remove '
        except:
            QMessageBox.warning(
                self,
                "Use R-Tree",
                "All fields are necessary",
                QMessageBox.StandardButton.Cancel,
            )
        tgt = self.ui.table_target.currentText()
        if tgt in (None, "", " ", "Table (Target)"):
            return
        tgt_tab = tgt.split(".")[0][1:-1]
        tgt_col = tgt.split(".")[1][1:-1]
        sql = ""
        if self.ui.where.toPlainText() not in (None, "", " "):
            sql += "\nAND"
        sql += self.db.spatialIndexClause(tab_idx, col_idx, tgt_tab, tgt_col)
        self.ui.where.insertPlainText(sql)

    def reset(self):
        # reset lists:
        self.ui.values.setModel(None)
        self.ui.columns_2.clear()
        self.ui.columns.insertItems(0, ["Columns"])
        self.ui.columns_2.insertItems(0, ["Columns"])
        self.coltables = []
        self.col_col = []
        self.col_where = []

    def validate(self):
        query_col = str(self.ui.col.toPlainText())
        query_table = str(self.ui.tab.text())
        query_where = str(self.ui.where.toPlainText())
        query_group = str(self.ui.group.toPlainText())
        query_order = str(self.ui.order.toPlainText())
        query = ""
        if query_col.strip() != "":
            query += f"SELECT {query_col} \nFROM {query_table}"
        if query_where.strip() != "":
            query += "\nWHERE %s" % query_where
        if query_group.strip() != "":
            query += "\nGROUP BY %s" % query_group
        if query_order.strip() != "":
            query += "\nORDER BY %s" % query_order
        if query == "":
            return
        self.query = query

        saveParameter = {
            "coltables": self.coltables,
            "col_col": self.col_col,
            "col_where": self.col_where,
            "col": query_col,
            "tab": query_table,
            "where": query_where,
            "group": query_group,
            "order": query_order,
        }

        QueryBuilderDlg.saveParameter = saveParameter

    def restoreLastQuery(self):
        self.update_table_list()

        saveParameter = QueryBuilderDlg.saveParameter
        self.coltables = saveParameter["coltables"]
        self.col_col = saveParameter["col_col"]
        self.col_where = saveParameter["col_where"]
        self.ui.col.insertPlainText(saveParameter["col"])
        self.ui.tab.setText(saveParameter["tab"])
        self.ui.where.insertPlainText(saveParameter["where"])
        self.ui.order.setPlainText(saveParameter["order"])
        self.ui.group.setPlainText(saveParameter["group"])
        # list previous colist:
        for tablename in self.coltables:
            # Retrieve table object from table name:
            table = [
                table
                for table in self.tables
                if table.name.upper() == tablename.upper()
            ]
            if len(table) != 1:
                break
            table = table[0]
            columns = [f'"{table.name}"."{col.name}"' for col in table.fields()]
            # first and second col combobox
            end = self.ui.columns.count()
            self.ui.columns.insertItems(end, columns)
            self.ui.columns_2.insertItems(end, columns)
            end = self.ui.columns.count()
            self.ui.columns.insertSeparator(end)
            self.ui.columns_2.insertSeparator(end)
