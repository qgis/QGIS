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

from qgis.PyQt.QtWidgets import QApplication

from ..info_model import TableInfo, VectorTableInfo, RasterTableInfo, DatabaseInfo
from ..html_elems import (
    HtmlSection,
    HtmlParagraph,
    HtmlTable,
    HtmlTableHeader,
    HtmlTableCol,
)


class PGDatabaseInfo(DatabaseInfo):

    def connectionDetails(self):
        tbl = [
            (
                QApplication.translate("DBManagerPlugin", "Host:"),
                self.db.connector.host,
            ),
            (
                QApplication.translate("DBManagerPlugin", "User:"),
                self.db.connector.user,
            ),
            (
                QApplication.translate("DBManagerPlugin", "Database:"),
                self.db.connector.dbname,
            ),
        ]
        return HtmlTable(tbl)


class PGTableInfo(TableInfo):

    def __init__(self, table):
        super().__init__(table)
        self.table = table

    def generalInfo(self):
        ret = []

        # if the estimation is less than 100 rows, try to count them - it shouldn't take long time
        if self.table.rowCount is None and self.table.estimatedRowCount < 100:
            # row count information is not displayed yet, so just block
            # table signals to avoid double refreshing (infoViewer->refreshRowCount->tableChanged->infoViewer)
            self.table.blockSignals(True)
            self.table.refreshRowCount()
            self.table.blockSignals(False)

        tbl = [
            (
                QApplication.translate("DBManagerPlugin", "Relation type:"),
                (
                    QApplication.translate("DBManagerPlugin", "View")
                    if self.table._relationType == "v"
                    else (
                        QApplication.translate("DBManagerPlugin", "Materialized view")
                        if self.table._relationType == "m"
                        else QApplication.translate("DBManagerPlugin", "Table")
                    )
                ),
            ),
            (QApplication.translate("DBManagerPlugin", "Owner:"), self.table.owner),
        ]
        if self.table.comment:
            tbl.append(
                (
                    QApplication.translate("DBManagerPlugin", "Comment:"),
                    self.table.comment,
                )
            )

        tbl.extend(
            [
                (QApplication.translate("DBManagerPlugin", "Pages:"), self.table.pages),
                (
                    QApplication.translate("DBManagerPlugin", "Rows (estimation):"),
                    self.table.estimatedRowCount,
                ),
            ]
        )

        # privileges
        # has the user access to this schema?
        schema_priv = (
            self.table.database().connector.getSchemaPrivileges(self.table.schemaName())
            if self.table.schema()
            else None
        )
        if schema_priv is None:
            pass
        elif not schema_priv[1]:  # no usage privileges on the schema
            tbl.append(
                (
                    QApplication.translate("DBManagerPlugin", "Privileges:"),
                    QApplication.translate(
                        "DBManagerPlugin",
                        "<warning> This user doesn't have usage privileges for this schema!",
                    ),
                )
            )
        else:
            table_priv = self.table.database().connector.getTablePrivileges(
                (self.table.schemaName(), self.table.name)
            )
            privileges = []
            if table_priv[0]:
                privileges.append("select")

                if self.table.rowCount is not None and self.table.rowCount >= 0:
                    tbl.append(
                        (
                            QApplication.translate(
                                "DBManagerPlugin", "Rows (counted):"
                            ),
                            (
                                self.table.rowCount
                                if self.table.rowCount is not None
                                else QApplication.translate(
                                    "DBManagerPlugin",
                                    'Unknown (<a href="action:rows/count">find out</a>)',
                                )
                            ),
                        )
                    )

            if table_priv[1]:
                privileges.append("insert")
            if table_priv[2]:
                privileges.append("update")
            if table_priv[3]:
                privileges.append("delete")
            priv_string = (
                ", ".join(privileges)
                if len(privileges) > 0
                else QApplication.translate(
                    "DBManagerPlugin", "<warning> This user has no privileges!"
                )
            )
            tbl.append(
                (QApplication.translate("DBManagerPlugin", "Privileges:"), priv_string)
            )

        ret.append(HtmlTable(tbl))

        if schema_priv is not None and schema_priv[1]:
            if (
                table_priv[0]
                and not table_priv[1]
                and not table_priv[2]
                and not table_priv[3]
            ):
                ret.append(
                    HtmlParagraph(
                        QApplication.translate(
                            "DBManagerPlugin",
                            "<warning> This user has read-only privileges.",
                        )
                    )
                )

        if not self.table.isView:
            if self.table.rowCount is not None:
                if abs(self.table.estimatedRowCount - self.table.rowCount) > 1 and (
                    self.table.estimatedRowCount > 2 * self.table.rowCount
                    or self.table.rowCount > 2 * self.table.estimatedRowCount
                ):
                    ret.append(
                        HtmlParagraph(
                            QApplication.translate(
                                "DBManagerPlugin",
                                "<warning> There's a significant difference between estimated and real row count. "
                                'Consider running <a href="action:vacuumanalyze/run">VACUUM ANALYZE</a>.',
                            )
                        )
                    )

        # primary key defined?
        if not self.table.isView:
            if len([fld for fld in self.table.fields() if fld.primaryKey]) <= 0:
                ret.append(
                    HtmlParagraph(
                        QApplication.translate(
                            "DBManagerPlugin",
                            "<warning> No primary key defined for this table!",
                        )
                    )
                )

        return ret

    def getSpatialInfo(self):
        ret = []

        info = self.db.connector.getSpatialInfo()
        if info is None:
            return

        tbl = [
            (QApplication.translate("DBManagerPlugin", "Library:"), info[0]),
            (QApplication.translate("DBManagerPlugin", "Scripts:"), info[3]),
            ("GEOS:", info[1]),
            ("Proj:", info[2]),
        ]
        ret.append(HtmlTable(tbl))

        if info[1] is not None and info[1] != info[2]:
            ret.append(
                HtmlParagraph(
                    QApplication.translate(
                        "DBManagerPlugin",
                        "<warning> Version of installed scripts doesn't match version of released scripts!\n"
                        "This is probably a result of incorrect PostGIS upgrade.",
                    )
                )
            )

        if not self.db.connector.has_geometry_columns:
            ret.append(
                HtmlParagraph(
                    QApplication.translate(
                        "DBManagerPlugin",
                        "<warning> geometry_columns table doesn't exist!\n"
                        "This table is essential for many GIS applications for enumeration of tables.",
                    )
                )
            )
        elif not self.db.connector.has_geometry_columns_access:
            ret.append(
                HtmlParagraph(
                    QApplication.translate(
                        "DBManagerPlugin",
                        "<warning> This user doesn't have privileges to read contents of geometry_columns table!\n"
                        "This table is essential for many GIS applications for enumeration of tables.",
                    )
                )
            )

        return ret

    def fieldsDetails(self):
        tbl = []

        # define the table header
        header = (
            "#",
            QApplication.translate("DBManagerPlugin", "Name"),
            QApplication.translate("DBManagerPlugin", "Type"),
            QApplication.translate("DBManagerPlugin", "Length"),
            QApplication.translate("DBManagerPlugin", "Null"),
            QApplication.translate("DBManagerPlugin", "Default"),
            QApplication.translate("DBManagerPlugin", "Comment"),
        )
        tbl.append(HtmlTableHeader(header))

        # add table contents
        for fld in self.table.fields():
            char_max_len = (
                fld.charMaxLen
                if fld.charMaxLen is not None and fld.charMaxLen != -1
                else ""
            )
            is_null_txt = "N" if fld.notNull else "Y"

            # make primary key field underlined
            attrs = {"class": "underline"} if fld.primaryKey else None
            name = HtmlTableCol(fld.name, attrs)

            tbl.append(
                (
                    fld.num,
                    name,
                    fld.type2String(),
                    char_max_len,
                    is_null_txt,
                    fld.default2String(),
                    fld.getComment(),
                )
            )

        return HtmlTable(tbl, {"class": "header"})

    def triggersDetails(self):
        if self.table.triggers() is None or len(self.table.triggers()) <= 0:
            return None

        ret = []

        tbl = []
        # define the table header
        header = (
            QApplication.translate("DBManagerPlugin", "Name"),
            QApplication.translate("DBManagerPlugin", "Function"),
            QApplication.translate("DBManagerPlugin", "Type"),
            QApplication.translate("DBManagerPlugin", "Enabled"),
        )
        tbl.append(HtmlTableHeader(header))

        # add table contents
        for trig in self.table.triggers():
            name = (
                '{name} (<a href="action:trigger/{name}/{action}">{action}</a>)'.format(
                    name=trig.name, action="delete"
                )
            )

            (enabled, action) = (
                (QApplication.translate("DBManagerPlugin", "Yes"), "disable")
                if trig.enabled
                else (QApplication.translate("DBManagerPlugin", "No"), "enable")
            )
            txt_enabled = '{enabled} (<a href="action:trigger/{name}/{action}">{action}</a>)'.format(
                name=trig.name, action=action, enabled=enabled
            )

            tbl.append((name, trig.function, trig.type2String(), txt_enabled))

        ret.append(HtmlTable(tbl, {"class": "header"}))

        ret.append(
            HtmlParagraph(
                QApplication.translate(
                    "DBManagerPlugin",
                    '<a href="action:triggers/enable">Enable all triggers</a> / <a href="action:triggers/disable">Disable all triggers</a>',
                )
            )
        )
        return ret

    def rulesDetails(self):
        if self.table.rules() is None or len(self.table.rules()) <= 0:
            return None

        tbl = []
        # define the table header
        header = (
            QApplication.translate("DBManagerPlugin", "Name"),
            QApplication.translate("DBManagerPlugin", "Definition"),
        )
        tbl.append(HtmlTableHeader(header))

        # add table contents
        for rule in self.table.rules():
            name = '{name} (<a href="action:rule/{name}/{action}">{action}</a>)'.format(
                name=rule.name, action="delete"
            )
            tbl.append((name, rule.definition))

        return HtmlTable(tbl, {"class": "header"})

    def getTableInfo(self):
        ret = TableInfo.getTableInfo(self)

        # rules
        rules_details = self.rulesDetails()
        if rules_details is None:
            pass
        else:
            ret.append(
                HtmlSection(
                    QApplication.translate("DBManagerPlugin", "Rules"), rules_details
                )
            )

        return ret


class PGVectorTableInfo(PGTableInfo, VectorTableInfo):

    def __init__(self, table):
        VectorTableInfo.__init__(self, table)
        PGTableInfo.__init__(self, table)

    def spatialInfo(self):
        return VectorTableInfo.spatialInfo(self)


class PGRasterTableInfo(PGTableInfo, RasterTableInfo):

    def __init__(self, table):
        RasterTableInfo.__init__(self, table)
        PGTableInfo.__init__(self, table)

    def spatialInfo(self):
        return RasterTableInfo.spatialInfo(self)
