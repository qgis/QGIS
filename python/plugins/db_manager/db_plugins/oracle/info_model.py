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

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import QGis

from ..info_model import TableInfo, VectorTableInfo, DatabaseInfo, \
    SchemaInfo
from ..html_elems import HtmlContent, HtmlSection, HtmlParagraph, \
    HtmlList, HtmlTable, HtmlTableHeader, HtmlTableCol

# Syntax Highlight for VIEWS/MVIEWS
from pygments import highlight
from pygments.lexers import get_lexer_by_name
from pygments.formatters import HtmlFormatter


class ORDatabaseInfo(DatabaseInfo):

    def __init__(self, db):
        self.db = db

    def connectionDetails(self):
        tbl = []

        if self.db.connector.host != u"":
            tbl.append((QApplication.translate("DBManagerPlugin", "Host:"),
                        self.db.connector.host))
        tbl.append((QApplication.translate("DBManagerPlugin", "Database:"),
                   self.db.connector.dbname))
        tbl.append((QApplication.translate("DBManagerPlugin", "User:"),
                    self.db.connector.user))
        tbl.append((QApplication.translate("DBManagerPlugin",
                                           "SQLite list tables cache:"),
                    "Enabled" if self.db.connector.hasCache else
                    "Unavailable"))

        return HtmlTable(tbl)

    def spatialInfo(self):
        ret = []

        info = self.db.connector.getSpatialInfo()
        if not info:
            return

        tbl = [
            (QApplication.translate("DBManagerPlugin", "Oracle\
            Spatial:"),
             info[0])
        ]
        ret.append(HtmlTable(tbl))

        if not self.db.connector.has_geometry_columns:
            ret.append(
                HtmlParagraph(
                    QApplication.translate(
                        "DBManagerPlugin",
                        (u"<warning> ALL_SDO_GEOM_METADATA"
                         u" view doesn't exist!\n"
                         u"This view is essential for many"
                         u"GIS applications for enumeration of tables."))))

        return ret

    def privilegesDetails(self):
        """ find if user can create schemas (CREATE ANY TABLE or something)"""
        # TODO
        return None


class ORTableInfo(TableInfo):

    def __init__(self, table):
        self.table = table
        if not self.table.objectType:
            self.table.getType()
        if not self.table.comment:
            self.table.getComment()
        if not self.table.estimatedRowCount and not self.table.isView:
            self.table.refreshRowEstimation()
        if not self.table.creationDate:
            self.table.getDates()

    def generalInfo(self):
        ret = []

        # if the estimation is less than 100 rows, try to count them - it
        # shouldn't take long time
        if (not self.table.isView
                and not self.table.rowCount
                and self.table.estimatedRowCount < 100):
            # row count information is not displayed yet, so just block
            # table signals to avoid double refreshing
            # (infoViewer->refreshRowCount->tableChanged->infoViewer)
            self.table.blockSignals(True)
            self.table.refreshRowCount()
            self.table.blockSignals(False)

        relation_type = QApplication.translate(
            "DBManagerPlugin", self.table.objectType)

        tbl = [
            (QApplication.translate("DBManagerPlugin", "Object type:"),
             relation_type),
            (QApplication.translate("DBManagerPlugin", "Owner:"),
             self.table.owner)
        ]

        if self.table.comment:
            tbl.append(
                (QApplication.translate(
                    "DBManagerPlugin",
                    "Comment:"),
                 self.table.comment))

        # Estimated rows
        if not self.table.isView:
            tbl.append(
                (QApplication.translate(
                    "DBManagerPlugin", "Rows (estimation):"),
                 self.table.estimatedRowCount)
            )
        if self.table.rowCount or self.table.rowCount >= 0:
            # Add a real count of rows
            tbl.append(
                (QApplication.translate("DBManagerPlugin", "Rows (counted):"),
                 self.table.rowCount)
            )
        else:
            tbl.append(
                (QApplication.translate("DBManagerPlugin", "Rows (counted):"),
                 'Unknown (<a href="action:rows/recount">find out</a>)')
            )

        # Add creation and modification dates
        if self.table.creationDate:
            tbl.append(
                (QApplication.translate("DBManagerPlugin", "Creation Date:"),
                 self.table.creationDate))

        if self.table.modificationDate:
            tbl.append(
                (QApplication.translate(
                    "DBManagerPlugin", "Last Modification Date:"),
                 self.table.modificationDate))

        # privileges
        # has the user access to this schema?
        schema_priv = self.table.database().connector.getSchemaPrivileges(
            self.table.schemaName()) if self.table.schema() else None
        if not schema_priv:
            pass
        elif schema_priv[1] is False:  # no usage privileges on the schema
            tbl.append((QApplication.translate(
                "DBManagerPlugin", "Privileges:"),
                QApplication.translate(
                "DBManagerPlugin",
                (u"<warning> This user doesn't have usage privileges"
                 u"for this schema!"))))
        else:
            table_priv = self.table.database().connector.getTablePrivileges(
                (self.table.schemaName(), self.table.name))
            privileges = []
            if table_priv[0]:
                privileges.append("select")
            if table_priv[1]:
                privileges.append("insert")
            if table_priv[2]:
                privileges.append("update")
            if table_priv[3]:
                privileges.append("delete")

            if len(privileges) > 0:
                priv_string = u", ".join(privileges)
            else:
                priv_string = QApplication.translate(
                    "DBManagerPlugin",
                    '<warning> This user has no privileges!')

            tbl.append(
                (QApplication.translate(
                    "DBManagerPlugin", "Privileges:"),
                 priv_string))

        ret.append(HtmlTable(tbl))

        if schema_priv and schema_priv[1]:
            if (table_priv[0]
                    and not table_priv[1]
                    and not table_priv[2]
                    and not table_priv[3]):
                ret.append(
                    HtmlParagraph(QApplication.translate(
                        "DBManagerPlugin",
                        "<warning> This user has read-only privileges.")))

        # primary key defined?
        if (not self.table.isView
                and self.table.objectType != u"MATERIALIZED VIEW"):
            pk = filter(lambda fld: fld.primaryKey, self.table.fields())
            if len(pk) <= 0:
                ret.append(
                    HtmlParagraph(QApplication.translate(
                        "DBManagerPlugin",
                        "<warning> No primary key defined for this table!")))

        return ret

    def getSpatialInfo(self):
        ret = []

        info = self.db.connector.getSpatialInfo()
        if not info:
            return

        tbl = [
            (QApplication.translate(
             "DBManagerPlugin", "Library:"), info[0])  # ,
        ]
        ret.append(HtmlTable(tbl))

        if not self.db.connector.has_geometry_columns:
            ret.append(HtmlParagraph(
                QApplication.translate(
                    "DBManagerPlugin",
                    (u"<warning> ALL_SDO_GEOM_METADATA table doesn't exist!\n"
                     u"This table is essential for many GIS"
                     u"applications for enumeration of tables."))))

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
            QApplication.translate("DBManagerPlugin", "Comment"))
        tbl.append(HtmlTableHeader(header))

        # add table contents
        for fld in self.table.fields():
            char_max_len = fld.charMaxLen if fld.charMaxLen else ""
            if fld.modifier:
                char_max_len = u"{},{}".format(char_max_len, fld.modifier)
            is_null_txt = "N" if fld.notNull else "Y"

            # make primary key field underlined
            attrs = {"class": "underline"} if fld.primaryKey else None
            name = HtmlTableCol(fld.name, attrs)

            tbl.append(
                (fld.num, name, fld.type2String(), char_max_len,
                 is_null_txt, fld.default2String(), fld.comment))

        return HtmlTable(tbl, {"class": "header"})

    def constraintsDetails(self):
        if not self.table.constraints():
            return None

        tbl = []

        # define the table header
        header = (QApplication.translate("DBManagerPlugin", "Name"),
                  QApplication.translate("DBManagerPlugin", "Type"),
                  QApplication.translate("DBManagerPlugin", "Column"),
                  QApplication.translate("DBManagerPlugin", "Status"),
                  QApplication.translate("DBManagerPlugin", "Validated"),
                  QApplication.translate("DBManagerPlugin", "Generated"),
                  QApplication.translate("DBManagerPlugin", "Check condition"),
                  QApplication.translate("DBManagerPlugin", "Foreign Table"),
                  QApplication.translate("DBManagerPlugin", "Foreign column"),
                  QApplication.translate("DBManagerPlugin", "On Delete"))
        tbl.append(HtmlTableHeader(header))

        # add table contents
        for con in self.table.constraints():
            tbl.append((con.name, con.type2String(), con.column,
                        con.status, con.validated, con.generated,
                        con.checkSource, con.foreignTable,
                        con.foreignKey, con.foreignOnDelete))

        return HtmlTable(tbl, {"class": "header"})

    def indexesDetails(self):
        if not self.table.indexes():
            return None

        tbl = []

        # define the table header
        header = (QApplication.translate("DBManagerPlugin", "Name"),
                  QApplication.translate("DBManagerPlugin", "Column(s)"),
                  QApplication.translate("DBManagerPlugin", "Index Type"),
                  QApplication.translate("DBManagerPlugin", "Status"),
                  QApplication.translate("DBManagerPlugin", "Last analyzed"),
                  QApplication.translate("DBManagerPlugin", "Compression"),
                  QApplication.translate("DBManagerPlugin", "Uniqueness"),
                  QApplication.translate("DBManagerPlugin", "Action"))
        tbl.append(HtmlTableHeader(header))

        # add table contents
        for idx in self.table.indexes():
            # get the fields the index is defined on
            tbl.append((idx.name, idx.column, idx.indexType,
                        idx.status, idx.analyzed, idx.compression,
                        idx.isUnique,
                        (u'<a href="action:index/{}/rebuild">Rebuild'
                         u"""</a>""".format(idx.name))))

        return HtmlTable(tbl, {"class": "header"})

    def triggersDetails(self):
        if not self.table.triggers():
            return None

        ret = []

        tbl = []
        # define the table header
        header = (
            QApplication.translate("DBManagerPlugin", "Name"),
            QApplication.translate("DBManagerPlugin", "Event"),
            QApplication.translate("DBManagerPlugin", "Type"),
            QApplication.translate("DBManagerPlugin", "Enabled"))
        tbl.append(HtmlTableHeader(header))

        # add table contents
        for trig in self.table.triggers():
            name = (u"""{0} (<a href="action:trigger/"""
                    u"""{0}/{1}">{1}</a>)""".format(trig.name, "delete"))

            if trig.enabled == u"ENABLED":
                enabled, action = (
                    QApplication.translate("DBManagerPlugin", "Yes"),
                    u"disable")
            else:
                enabled, action = (
                    QApplication.translate("DBManagerPlugin", "No"),
                    "enable")

            txt_enabled = (u"""{0} (<a href="action:trigger/"""
                           u"""{1}/{2}">{2}</a>)""".format(
                               enabled, trig.name, action))

            tbl.append((name, trig.event, trig.type, txt_enabled))

        ret.append(HtmlTable(tbl, {"class": "header"}))

        ret.append(
            HtmlParagraph(
                QApplication.translate(
                    "DBManagerPlugin",
                    (u'<a href="action:triggers/enable">'
                     u'Enable all triggers</a> / '
                     u'<a href="action:triggers/disable">'
                     u'Disable all triggers</a>'))))

        return ret

    def getTableInfo(self):
        ret = []

        general_info = self.generalInfo()
        if not general_info:
            pass
        else:
            ret.append(
                HtmlSection(
                    QApplication.translate(
                        "DBManagerPlugin", 'General info'),
                    general_info))

        # spatial info
        spatial_info = self.spatialInfo()
        if not spatial_info:
            pass
        else:
            spatial_info = HtmlContent(spatial_info)
            if not spatial_info.hasContents():
                spatial_info = QApplication.translate(
                    "DBManagerPlugin",
                    '<warning> This is not a spatial table.')
            ret.append(
                HtmlSection(
                    self.table.database().connection().typeNameString(),
                    spatial_info))

        # fields
        fields_details = self.fieldsDetails()
        if not fields_details:
            pass
        else:
            ret.append(
                HtmlSection(
                    QApplication.translate(
                        "DBManagerPlugin",
                        'Fields'),
                    fields_details))

        # constraints
        constraints_details = self.constraintsDetails()
        if not constraints_details:
            pass
        else:
            ret.append(
                HtmlSection(
                    QApplication.translate(
                        "DBManagerPlugin",
                        'Constraints'),
                    constraints_details))

        # indexes
        indexes_details = self.indexesDetails()
        if not indexes_details:
            pass
        else:
            ret.append(
                HtmlSection(
                    QApplication.translate(
                        "DBManagerPlugin",
                        'Indexes'),
                    indexes_details))

        # triggers
        triggers_details = self.triggersDetails()
        if not triggers_details:
            pass
        else:
            ret.append(
                HtmlSection(
                    QApplication.translate(
                        "DBManagerPlugin",
                        'Triggers'),
                    triggers_details))

        if self.table.objectType == u"MATERIALIZED VIEW":
            mview_info = self.getMViewInfo()
            ret.append(
                HtmlSection(
                    QApplication.translate(
                        "DBManagerPlugin",
                        'Materialized View information'),
                    mview_info))

        return ret

    def getMViewInfo(self):
        """If the table is a materialized view, grab more
        information...
        """
        ret = []
        tbl = []
        values = self.table.getMViewInfo()
        tbl.append((QApplication.translate("DBManagerPlugin",
                                           "Refresh Mode:"),
                    values[0]))
        tbl.append((QApplication.translate("DBManagerPlugin",
                                           "Refresh Method:"),
                    values[1]))
        tbl.append((QApplication.translate("DBManagerPlugin",
                                           "Build Mode:"),
                    values[2]))
        tbl.append((QApplication.translate("DBManagerPlugin",
                                           "Last Refresh Date:"),
                    values[5]))
        tbl.append((QApplication.translate("DBManagerPlugin",
                                           "Last Refresh Type:"),
                    values[4]))
        tbl.append((QApplication.translate("DBManagerPlugin",
                                           "Fast Refreshable:"),
                    values[3]))
        tbl.append((QApplication.translate("DBManagerPlugin",
                                           "Staleness:"),
                    values[6]))
        tbl.append((QApplication.translate("DBManagerPlugin",
                                           "Stale since:"),
                    values[7]))
        tbl.append((QApplication.translate("DBManagerPlugin",
                                           "Compile State:"),
                    values[8]))
        tbl.append((QApplication.translate("DBManagerPlugin",
                                           "Use no index:"),
                    values[9]))
        tbl.append((QApplication.translate(
            "DBManagerPlugin",
            (u'<a href="action:mview/refresh">Refresh the materializ'
             u'ed view</a>')),
            u""))
        ret.append(HtmlTable(tbl))

        return ret

    def getViewInfo(self):
        """If the table is a view or a materialized view, add the
        definition of the view.
        """

        if self.table.objectType not in [u"VIEW", u"MATERIALIZED VIEW"]:
            return []

        ret = self.getTableInfo()

        # view definition
        view_def = self.table.getDefinition()

        # Syntax highlight
        lexer = get_lexer_by_name("sql")
        formatter = HtmlFormatter(
            linenos=True, cssclass="source", noclasses=True)
        result = highlight(view_def, lexer, formatter)

        if view_def:
            if self.table.objectType == u"VIEW":
                title = u"View Definition"
            else:
                title = u"Materialized View Definition"
            ret.append(
                HtmlSection(
                    QApplication.translate("DBManagerPlugin", title),
                    result))

        return ret

    def toHtml(self):
        if self.table.objectType in [u"VIEW", u"MATERIALIZED VIEW"]:
            ret = self.getViewInfo()
        else:
            ret = self.getTableInfo()
        return HtmlContent(ret).toHtml()


class ORVectorTableInfo(ORTableInfo, VectorTableInfo):

    def __init__(self, table):
        VectorTableInfo.__init__(self, table)
        ORTableInfo.__init__(self, table)

    def spatialInfo(self):
        ret = []
        if not self.table.geomType:
            return ret

        tbl = [
            (QApplication.translate("DBManagerPlugin", "Column:"),
             self.table.geomColumn),
            (QApplication.translate("DBManagerPlugin", "Geometry:"),
             self.table.geomType),
            (QApplication.translate("DBManagerPlugin",
                                    "QGis Geometry type:"),
             QGis.featureType(self.table.wkbType))
        ]

        # only if we have info from geometry_columns
        if self.table.geomDim:
            tbl.append(
                (QApplication.translate(
                    "DBManagerPlugin",
                    "Dimension:"),
                 self.table.geomDim))

        srid = self.table.srid if self.table.srid else -1
        if srid != -1:
            sr_info = (
                self.table.database().connector.getSpatialRefInfo(srid))
        else:
            sr_info = QApplication.translate("DBManagerPlugin",
                                             "Undefined")
        if sr_info:
            tbl.append(
                (QApplication.translate(
                    "DBManagerPlugin", "Spatial ref:"),
                 u"{0} ({1})".format(sr_info, srid)))

        # estimated extent
        if not self.table.estimatedExtent:
            # estimated extent information is not displayed yet, so just block
            # table signals to avoid double refreshing
            # (infoViewer->refreshEstimatedExtent->tableChanged->infoViewer)
            self.table.blockSignals(True)
            self.table.refreshTableEstimatedExtent()
            self.table.blockSignals(False)

        if self.table.estimatedExtent:
            estimated_extent_str = (u"{:.9f}, {:.9f} - {:.9f}, "
                                    u"{:.9f}".format(
                                        *self.table.estimatedExtent))

            tbl.append(
                (QApplication.translate(
                    "DBManagerPlugin", "Estimated extent:"),
                 estimated_extent_str))

        # extent
        extent_str = None
        if self.table.extent and len(self.table.extent) == 4:
            extent_str = (u"{:.9f}, {:.9f} - {:.9f}, "
                          u"{:.9f}".format(*self.table.extent))
        elif self.table.rowCount > 0 or self.table.estimatedRowCount > 0:
            # Can't calculate an extent on empty layer
            extent_str = QApplication.translate(
                "DBManagerPlugin",
                '(unknown) (<a href="action:extent/get">find out</a>)')

        if extent_str:
            tbl.append(
                (QApplication.translate(
                    "DBManagerPlugin", "Extent:"),
                 extent_str))

        ret.append(HtmlTable(tbl))

        # Handle extent update metadata
        if (self.table.extent
                and self.table.extent != self.table.estimatedExtent
                and self.table.canUpdateMetadata()):
            ret.append(
                HtmlParagraph(
                    QApplication.translate(
                        "DBManagerPlugin",
                        (u'<warning> Metadata extent is different from'
                         u'real extent. You should <a href="action:extent'
                         u'/update">update it</a> !'))))

        # is there an entry in geometry_columns?
        if self.table.geomType.lower() == 'geometry':
            ret.append(
                HtmlParagraph(
                    QApplication.translate(
                        "DBManagerPlugin",
                        "<warning> There isn't entry in geometry_columns!")))

        # find out whether the geometry column has spatial index on it
        if not self.table.isView:
            if not self.table.hasSpatialIndex():
                ret.append(
                    HtmlParagraph(
                        QApplication.translate(
                            "DBManagerPlugin",
                            (u'<warning> No spatial index defined (<a href='
                             u'"action:spatialindex/create">'
                             u'create it</a>).'))))

        return ret
