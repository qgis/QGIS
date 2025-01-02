"""
/***************************************************************************
Name                 : Versioning plugin for DB Manager
Description          : Set up versioning support for a table
Date                 : Mar 12, 2012
copyright            : (C) 2012 by Giuseppe Sucameli
email                : brush.tyler@gmail.com

Based on PG_Manager by Martin Dobias <wonder.sk@gmail.com> (GPLv2 license)
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

from pathlib import Path

from qgis.PyQt import uic
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtWidgets import QDialog, QDialogButtonBox, QMessageBox, QApplication

from .....dlg_db_error import DlgDbError
from ....plugin import BaseError, Table

Ui_DlgVersioning, _ = uic.loadUiType(Path(__file__).parent / "DlgVersioining.ui")


class DlgVersioning(QDialog, Ui_DlgVersioning):

    def __init__(self, item, parent=None):
        QDialog.__init__(self, parent)
        self.item = item
        self.setupUi(self)

        self.db = self.item.database()
        self.schemas = self.db.schemas()
        self.hasSchemas = self.schemas is not None

        self.buttonBox.accepted.connect(self.onOK)
        self.buttonBox.helpRequested.connect(self.showHelp)

        self.populateSchemas()
        self.populateTables()

        if isinstance(item, Table):
            index = self.cboTable.findText(self.item.name)
            if index >= 0:
                self.cboTable.setCurrentIndex(index)

        self.cboSchema.currentIndexChanged.connect(self.populateTables)

        # updates of SQL window
        self.cboSchema.currentIndexChanged.connect(self.updateSql)
        self.cboTable.currentIndexChanged.connect(self.updateSql)
        self.chkCreateCurrent.stateChanged.connect(self.updateSql)
        self.editPkey.textChanged.connect(self.updateSql)
        self.editStart.textChanged.connect(self.updateSql)
        self.editEnd.textChanged.connect(self.updateSql)
        self.editUser.textChanged.connect(self.updateSql)

        self.updateSql()

    def populateSchemas(self):
        self.cboSchema.clear()
        if not self.hasSchemas:
            self.hideSchemas()
            return

        index = -1
        for schema in self.schemas:
            self.cboSchema.addItem(schema.name)
            if hasattr(self.item, "schema") and schema.name == self.item.schema().name:
                index = self.cboSchema.count() - 1
        self.cboSchema.setCurrentIndex(index)

    def hideSchemas(self):
        self.cboSchema.setEnabled(False)

    def populateTables(self):
        self.tables = []

        schemas = self.db.schemas()
        if schemas is not None:
            schema_name = self.cboSchema.currentText()
            matching_schemas = [x for x in schemas if x.name == schema_name]
            tables = matching_schemas[0].tables() if len(matching_schemas) > 0 else []
        else:
            tables = self.db.tables()

        self.cboTable.clear()
        for table in tables:
            if table.type == table.VectorType:  # contains geometry column?
                self.tables.append(table)
                self.cboTable.addItem(table.name)

    def get_escaped_name(self, schema, table, suffix):
        name = self.db.connector.quoteId(f"{table}{suffix}")
        schema_name = self.db.connector.quoteId(schema) if schema else None
        return f"{schema_name}.{name}" if schema_name else name

    def updateSql(self):
        if (
            self.cboTable.currentIndex() < 0
            or len(self.tables) < self.cboTable.currentIndex()
        ):
            return

        self.table = self.tables[self.cboTable.currentIndex()]
        self.schematable = self.table.quotedName()

        self.current = self.chkCreateCurrent.isChecked()

        self.colPkey = self.db.connector.quoteId(self.editPkey.text())
        self.colStart = self.db.connector.quoteId(self.editStart.text())
        self.colEnd = self.db.connector.quoteId(self.editEnd.text())
        self.colUser = self.db.connector.quoteId(self.editUser.text())

        self.columns = [self.db.connector.quoteId(x.name) for x in self.table.fields()]

        self.colOrigPkey = None
        for constr in self.table.constraints():
            if constr.type == constr.TypePrimaryKey:
                self.origPkeyName = self.db.connector.quoteId(constr.name)
                self.colOrigPkey = [
                    self.db.connector.quoteId(x_y[1].name)
                    for x_y in iter(list(constr.fields().items()))
                ]
                break

        if self.colOrigPkey is None:
            self.txtSql.setPlainText("Table doesn't have a primary key!")
            self.buttonBox.button(QDialogButtonBox.StandardButton.Ok).setEnabled(False)
            return
        elif len(self.colOrigPkey) > 1:
            self.txtSql.setPlainText("Table has multicolumn primary key!")
            self.buttonBox.button(QDialogButtonBox.StandardButton.Ok).setEnabled(False)
            return

        # take first (and only column of the pkey)
        self.colOrigPkey = self.colOrigPkey[0]

        # define view, function, rule and trigger names
        self.view = self.get_escaped_name(
            self.table.schemaName(), self.table.name, "_current"
        )

        self.func_at_time = self.get_escaped_name(
            self.table.schemaName(), self.table.name, "_at_time"
        )
        self.func_update = self.get_escaped_name(
            self.table.schemaName(), self.table.name, "_update"
        )
        self.func_insert = self.get_escaped_name(
            self.table.schemaName(), self.table.name, "_insert"
        )

        self.rule_del = self.get_escaped_name(None, self.table.name, "_del")
        self.trigger_update = self.get_escaped_name(None, self.table.name, "_update")
        self.trigger_insert = self.get_escaped_name(None, self.table.name, "_insert")

        sql = []

        # modify table: add serial column, start time, end time
        sql.append(self.sql_alterTable())
        # add primary key to the table
        sql.append(self.sql_setPkey())

        sql.append(self.sql_currentView())
        # add X_at_time, X_update, X_delete functions
        sql.append(self.sql_functions())
        # add insert, update trigger, delete rule
        sql.append(self.sql_triggers())
        # add _current view + updatable
        # if self.current:
        sql.append(self.sql_updatesView())

        self.txtSql.setPlainText("\n\n".join(sql))
        self.buttonBox.button(QDialogButtonBox.StandardButton.Ok).setEnabled(True)

        return sql

    def showHelp(self):
        helpText = """In this dialog you can set up versioning support for a table. The table will be modified so that all changes will be recorded: there will be a column with start time and end time. Every row will have its start time, end time is assigned when the feature gets deleted. When a row is modified, the original data is marked with end time and new row is created. With this system, it's possible to get back to state of the table any time in history. When selecting rows from the table, you will always have to specify at what time do you want the rows."""
        QMessageBox.information(self, "Help", helpText)

    def sql_alterTable(self):
        return "ALTER TABLE {} ADD {} serial, ADD {} timestamp default '-infinity', ADD {} timestamp, ADD {} varchar;".format(
            self.schematable, self.colPkey, self.colStart, self.colEnd, self.colUser
        )

    def sql_setPkey(self):
        return "ALTER TABLE {} DROP CONSTRAINT {}, ADD PRIMARY KEY ({});".format(
            self.schematable, self.origPkeyName, self.colPkey
        )

    def sql_currentView(self):
        cols = self.colPkey + "," + ",".join(self.columns)

        return (
            "CREATE VIEW %(view)s AS SELECT %(cols)s FROM %(schematable)s WHERE %(end)s IS NULL;"
            % {
                "view": self.view,
                "cols": cols,
                "schematable": self.schematable,
                "end": self.colEnd,
            }
        )

    def sql_functions(self):
        cols = ",".join(self.columns)
        all_cols = self.colPkey + "," + ",".join(self.columns)
        old_cols = ",".join("OLD." + x for x in self.columns)

        sql = """
CREATE OR REPLACE FUNCTION {func_at_time}(timestamp)
RETURNS SETOF {view} AS
$$
SELECT {all_cols} FROM {schematable} WHERE
  ( SELECT CASE WHEN {end} IS NULL THEN ({start} <= $1) ELSE ({start} <= $1 AND {end} > $1) END );
$$
LANGUAGE 'sql';

CREATE OR REPLACE FUNCTION {func_update}()
RETURNS TRIGGER AS
$$
BEGIN
  IF OLD.{end} IS NOT NULL THEN
    RETURN NULL;
  END IF;
  IF NEW.{end} IS NULL THEN
    INSERT INTO {schematable} ({cols}, {start}, {end}) VALUES ({oldcols}, OLD.{start}, current_timestamp);
    NEW.{start} = current_timestamp;
    NEW.{user} = current_user;
  END IF;
  RETURN NEW;
END;
$$
LANGUAGE 'plpgsql';

CREATE OR REPLACE FUNCTION {func_insert}()
RETURNS trigger AS
$$
BEGIN
  if NEW.{start} IS NULL then
    NEW.{start} = now();
    NEW.{end} = null;
    NEW.{user} = current_user;
  end if;
  RETURN NEW;
END;
$$
LANGUAGE 'plpgsql';""".format(
            view=self.view,
            schematable=self.schematable,
            cols=cols,
            oldcols=old_cols,
            start=self.colStart,
            end=self.colEnd,
            user=self.colUser,
            func_at_time=self.func_at_time,
            all_cols=all_cols,
            func_update=self.func_update,
            func_insert=self.func_insert,
        )
        return sql

    def sql_triggers(self):
        return """
CREATE RULE {rule_del} AS ON DELETE TO {schematable}
DO INSTEAD UPDATE {schematable} SET {end} = current_timestamp WHERE {pkey} = OLD.{pkey} AND {end} IS NULL;

CREATE TRIGGER {trigger_update} BEFORE UPDATE ON {schematable}
FOR EACH ROW EXECUTE PROCEDURE {func_update}();

CREATE TRIGGER {trigger_insert} BEFORE INSERT ON {schematable}
FOR EACH ROW EXECUTE PROCEDURE {func_insert}();""".format(
            rule_del=self.rule_del,
            trigger_update=self.trigger_update,
            trigger_insert=self.trigger_insert,
            func_update=self.func_update,
            func_insert=self.func_insert,
            schematable=self.schematable,
            pkey=self.colPkey,
            end=self.colEnd,
        )

    def sql_updatesView(self):
        cols = ",".join(self.columns)
        return_cols = self.colPkey + "," + ",".join(self.columns)
        new_cols = ",".join("NEW." + x for x in self.columns)
        assign_cols = ",".join(f"{x} = NEW.{x}" for x in self.columns)

        return """
CREATE OR REPLACE RULE "_DELETE" AS ON DELETE TO {view} DO INSTEAD
  DELETE FROM {schematable} WHERE {origpkey} = old.{origpkey};
CREATE OR REPLACE RULE "_INSERT" AS ON INSERT TO {view} DO INSTEAD
  INSERT INTO {schematable} ({cols}) VALUES ({newcols}) RETURNING {return_cols};
CREATE OR REPLACE RULE "_UPDATE" AS ON UPDATE TO {view} DO INSTEAD
  UPDATE {schematable} SET {assign} WHERE {origpkey} = NEW.{origpkey};""".format(
            view=self.view,
            schematable=self.schematable,
            cols=cols,
            newcols=new_cols,
            return_cols=return_cols,
            assign=assign_cols,
            origpkey=self.colOrigPkey,
        )

    def onOK(self):
        # execute and commit the code
        QApplication.setOverrideCursor(Qt.CursorShape.WaitCursor)
        try:
            sql = "\n".join(self.updateSql())
            self.db.connector._execute_and_commit(sql)

        except BaseError as e:
            DlgDbError.showError(e, self)
            return

        finally:
            QApplication.restoreOverrideCursor()

        QMessageBox.information(
            self, "DB Manager", "Versioning was successfully created."
        )
        self.accept()
