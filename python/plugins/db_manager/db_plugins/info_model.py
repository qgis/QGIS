# -*- coding: utf-8 -*-

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

from PyQt4.QtGui import QApplication

from .html_elems import HtmlContent, HtmlSection, HtmlParagraph, HtmlList, HtmlTable, HtmlTableHeader, HtmlTableCol


class DatabaseInfo:

    def __init__(self, db):
        self.db = db

    def __del__(self):
        self.db = None

    def generalInfo(self):
        info = self.db.connector.getInfo()
        tbl = [
            (QApplication.translate("DBManagerPlugin", "Server version: "), info[0])
        ]
        return HtmlTable(tbl)

    def connectionDetails(self):
        tbl = [
            (QApplication.translate("DBManagerPlugin", "Host:"), self.db.connector.host),
            (QApplication.translate("DBManagerPlugin", "User:"), self.db.connector.user)
        ]
        return HtmlTable(tbl)

    def spatialInfo(self):
        ret = []

        info = self.db.connector.getSpatialInfo()
        if info is None:
            return

        tbl = [
            (QApplication.translate("DBManagerPlugin", "Library:"), info[0]),
            ("GEOS:", info[1]),
            ("Proj:", info[2])
        ]
        ret.append(HtmlTable(tbl))

        if not self.db.connector.has_geometry_columns:
            ret.append(HtmlParagraph(
                QApplication.translate("DBManagerPlugin", "<warning> geometry_columns table doesn't exist!\n"
                                                          "This table is essential for many GIS applications for enumeration of tables.")))

        return ret

    def privilegesDetails(self):
        details = self.db.connector.getDatabasePrivileges()
        lst = []
        if details[0]:
            lst.append(QApplication.translate("DBManagerPlugin", "create new schemas"))
        if details[1]:
            lst.append(QApplication.translate("DBManagerPlugin", "create temporary tables"))
        return HtmlList(lst)

    def toHtml(self):
        if self.db is None:
            return HtmlSection(QApplication.translate("DBManagerPlugin", 'Not connected')).toHtml()

        ret = []

        # connection details
        conn_details = self.connectionDetails()
        if conn_details is None:
            pass
        else:
            ret.append(HtmlSection(QApplication.translate("DBManagerPlugin", 'Connection details'), conn_details))

        # database information
        general_info = self.generalInfo()
        if general_info is None:
            pass
        else:
            ret.append(HtmlSection(QApplication.translate("DBManagerPlugin", 'General info'), general_info))

        # has spatial enabled?
        spatial_info = self.spatialInfo()
        if spatial_info is None:
            pass
        else:
            typename = self.db.connection().typeNameString()
            spatial_info = HtmlContent(spatial_info)
            if not spatial_info.hasContents():
                spatial_info = QApplication.translate("DBManagerPlugin", '<warning> %s support not enabled!') % typename
            ret.append(HtmlSection(typename, spatial_info))

        # privileges
        priv_details = self.privilegesDetails()
        if priv_details is None:
            pass
        else:
            priv_details = HtmlContent(priv_details)
            if not priv_details.hasContents():
                priv_details = QApplication.translate("DBManagerPlugin", '<warning> This user has no privileges!')
            else:
                priv_details = [QApplication.translate("DBManagerPlugin", "User has privileges:"), priv_details]
            ret.append(HtmlSection(QApplication.translate("DBManagerPlugin", 'Privileges'), priv_details))

        return HtmlContent(ret).toHtml()


class SchemaInfo:

    def __init__(self, schema):
        self.schema = schema

    def __del__(self):
        self.schema = None

    def generalInfo(self):
        tbl = [
            # ("Tables:", self.schema.tableCount)
        ]
        if self.schema.owner:
            tbl.append((QApplication.translate("DBManagerPlugin", "Owner:"), self.schema.owner))
        if self.schema.comment:
            tbl.append((QApplication.translate("DBManagerPlugin", "Comment:"), self.schema.comment))
        return HtmlTable(tbl)

    def privilegesDetails(self):
        details = self.schema.database().connector.getSchemaPrivileges(self.schema.name)
        lst = []
        if details[0]:
            lst.append(QApplication.translate("DBManagerPlugin", "create new objects"))
        if details[1]:
            lst.append(QApplication.translate("DBManagerPlugin", "access objects"))
        return HtmlList(lst)

    def toHtml(self):
        ret = []

        general_info = self.generalInfo()
        if general_info is None:
            pass
        else:
            ret.append(HtmlSection(QApplication.translate("DBManagerPlugin", 'Schema details'), general_info))

        priv_details = self.privilegesDetails()
        if priv_details is None:
            pass
        else:
            priv_details = HtmlContent(priv_details)
            if not priv_details.hasContents():
                priv_details = QApplication.translate("DBManagerPlugin",
                                                      '<warning> This user has no privileges to access this schema!')
            else:
                priv_details = [QApplication.translate("DBManagerPlugin", "User has privileges:"), priv_details]
            ret.append(HtmlSection(QApplication.translate("DBManagerPlugin", 'Privileges'), priv_details))

        return HtmlContent(ret).toHtml()


class TableInfo:

    def __init__(self, table):
        self.table = table

    def __del__(self):
        self.table = None

    def generalInfo(self):
        if self.table.rowCount is None:
            # row count information is not displayed yet, so just block
            # table signals to avoid double refreshing (infoViewer->refreshRowCount->tableChanged->infoViewer)
            self.table.blockSignals(True)
            self.table.refreshRowCount()
            self.table.blockSignals(False)

        tbl = [
            (QApplication.translate("DBManagerPlugin", "Relation type:"),
             QApplication.translate("DBManagerPlugin", "View") if self.table.isView else QApplication.translate(
                 "DBManagerPlugin", "Table")),
            (QApplication.translate("DBManagerPlugin", "Rows:"),
             self.table.rowCount if self.table.rowCount is not None else QApplication.translate("DBManagerPlugin",
                                                                                                'Unknown (<a href="action:rows/count">find out</a>)'))
        ]
        if self.table.comment:
            tbl.append((QApplication.translate("DBManagerPlugin", "Comment:"), self.table.comment))

        return HtmlTable(tbl)

    def spatialInfo(self):  # implemented in subclasses
        return None

    def fieldsDetails(self):
        tbl = []

        # define the table header
        header = (
            "#", QApplication.translate("DBManagerPlugin", "Name"), QApplication.translate("DBManagerPlugin", "Type"),
            QApplication.translate("DBManagerPlugin", "Null"), QApplication.translate("DBManagerPlugin", "Default"))
        tbl.append(HtmlTableHeader(header))

        # add table contents
        for fld in self.table.fields():
            is_null_txt = "N" if fld.notNull else "Y"

            # make primary key field underlined
            attrs = {"class": "underline"} if fld.primaryKey else None
            name = HtmlTableCol(fld.name, attrs)

            tbl.append((fld.num, name, fld.type2String(), is_null_txt, fld.default2String()))

        return HtmlTable(tbl, {"class": "header"})

    def constraintsDetails(self):
        if self.table.constraints() is None or len(self.table.constraints()) <= 0:
            return None

        tbl = []

        # define the table header
        header = (QApplication.translate("DBManagerPlugin", "Name"), QApplication.translate("DBManagerPlugin", "Type"),
                  QApplication.translate("DBManagerPlugin", "Column(s)"))
        tbl.append(HtmlTableHeader(header))

        # add table contents
        for con in self.table.constraints():
            # get the fields the constraint is defined on
            cols = map(lambda p: p[1].name if p[1] is not None else u"??? (#%d)" % p[0], con.fields().iteritems())
            tbl.append((con.name, con.type2String(), u'\n'.join(cols)))

        return HtmlTable(tbl, {"class": "header"})

    def indexesDetails(self):
        if self.table.indexes() is None or len(self.table.indexes()) <= 0:
            return None

        tbl = []

        # define the table header
        header = (
            QApplication.translate("DBManagerPlugin", "Name"), QApplication.translate("DBManagerPlugin", "Column(s)"))
        tbl.append(HtmlTableHeader(header))

        # add table contents
        for idx in self.table.indexes():
            # get the fields the index is defined on
            cols = map(lambda p: p[1].name if p[1] is not None else u"??? (#%d)" % p[0], idx.fields().iteritems())
            tbl.append((idx.name, u'\n'.join(cols)))

        return HtmlTable(tbl, {"class": "header"})

    def triggersDetails(self):
        if self.table.triggers() is None or len(self.table.triggers()) <= 0:
            return None

        tbl = []

        # define the table header
        header = (
            QApplication.translate("DBManagerPlugin", "Name"), QApplication.translate("DBManagerPlugin", "Function"))
        tbl.append(HtmlTableHeader(header))

        # add table contents
        for trig in self.table.triggers():
            name = u'%(name)s (<a href="action:trigger/%(name)s/%(action)s">%(action)s</a>)' % {"name": trig.name,
                                                                                                "action": "delete"}
            tbl.append((name, trig.function))

        return HtmlTable(tbl, {"class": "header"})

    def getViewDefinition(self):
        if not self.table.isView:
            return None
        return self.table.database().connector.getViewDefinition((self.table.schemaName(), self.table.name))

    def getTableInfo(self):
        ret = []

        general_info = self.generalInfo()
        if general_info is None:
            pass
        else:
            ret.append(HtmlSection(QApplication.translate("DBManagerPlugin", 'General info'), general_info))

        # spatial info
        spatial_info = self.spatialInfo()
        if spatial_info is None:
            pass
        else:
            spatial_info = HtmlContent(spatial_info)
            if not spatial_info.hasContents():
                spatial_info = QApplication.translate("DBManagerPlugin", '<warning> This is not a spatial table.')
            ret.append(HtmlSection(self.table.database().connection().typeNameString(), spatial_info))

        # fields
        fields_details = self.fieldsDetails()
        if fields_details is None:
            pass
        else:
            ret.append(HtmlSection(QApplication.translate("DBManagerPlugin", 'Fields'), fields_details))

        # constraints
        constraints_details = self.constraintsDetails()
        if constraints_details is None:
            pass
        else:
            ret.append(HtmlSection(QApplication.translate("DBManagerPlugin", 'Constraints'), constraints_details))

        # indexes
        indexes_details = self.indexesDetails()
        if indexes_details is None:
            pass
        else:
            ret.append(HtmlSection(QApplication.translate("DBManagerPlugin", 'Indexes'), indexes_details))

        # triggers
        triggers_details = self.triggersDetails()
        if triggers_details is None:
            pass
        else:
            ret.append(HtmlSection(QApplication.translate("DBManagerPlugin", 'Triggers'), triggers_details))

        return ret

    def getViewInfo(self):
        if not self.table.isView:
            return []

        ret = self.getTableInfo()

        # view definition
        view_def = self.getViewDefinition()
        if view_def is None:
            pass
        else:
            ret.append(HtmlSection(QApplication.translate("DBManagerPlugin", 'View definition'), view_def))

        return ret

    def toHtml(self):
        if self.table.isView:
            ret = self.getViewInfo()
        else:
            ret = self.getTableInfo()
        return HtmlContent(ret).toHtml()


class VectorTableInfo(TableInfo):

    def __init__(self, table):
        TableInfo.__init__(self, table)

    def spatialInfo(self):
        ret = []
        if self.table.geomType is None:
            return ret

        tbl = [
            (QApplication.translate("DBManagerPlugin", "Column:"), self.table.geomColumn),
            (QApplication.translate("DBManagerPlugin", "Geometry:"), self.table.geomType)
        ]

        # only if we have info from geometry_columns
        if self.table.geomDim:
            tbl.append((QApplication.translate("DBManagerPlugin", "Dimension:"), self.table.geomDim))

        srid = self.table.srid if self.table.srid is not None else -1
        sr_info = self.table.database().connector.getSpatialRefInfo(srid) if srid != -1 else QApplication.translate(
            "DBManagerPlugin", "Undefined")
        if sr_info:
            tbl.append((QApplication.translate("DBManagerPlugin", "Spatial ref:"), u"%s (%d)" % (sr_info, srid)))

        # estimated extent
        if not self.table.isView:
            if self.table.estimatedExtent is None:
                # estimated extent information is not displayed yet, so just block
                # table signals to avoid double refreshing (infoViewer->refreshEstimatedExtent->tableChanged->infoViewer)
                self.table.blockSignals(True)
                self.table.refreshTableEstimatedExtent()
                self.table.blockSignals(False)

            if self.table.estimatedExtent is not None and self.table.estimatedExtent[0] is not None:
                estimated_extent_str = '%.5f, %.5f - %.5f, %.5f' % self.table.estimatedExtent
                tbl.append((QApplication.translate("DBManagerPlugin", "Estimated extent:"), estimated_extent_str))

        # extent
        if self.table.extent is not None and self.table.extent[0] is not None:
            extent_str = '%.5f, %.5f - %.5f, %.5f' % self.table.extent
        else:
            extent_str = QApplication.translate("DBManagerPlugin",
                                                '(unknown) (<a href="action:extent/get">find out</a>)')
        tbl.append((QApplication.translate("DBManagerPlugin", "Extent:"), extent_str))

        ret.append(HtmlTable(tbl))

        # is there an entry in geometry_columns?
        if self.table.geomType.lower() == 'geometry':
            ret.append(HtmlParagraph(
                QApplication.translate("DBManagerPlugin", "<warning> There is no entry in geometry_columns!")))

        # find out whether the geometry column has spatial index on it
        if not self.table.isView:
            if not self.table.hasSpatialIndex():
                ret.append(HtmlParagraph(QApplication.translate("DBManagerPlugin",
                                                                '<warning> No spatial index defined (<a href="action:spatialindex/create">create it</a>)')))

        return ret


class RasterTableInfo(TableInfo):

    def __init__(self, table):
        TableInfo.__init__(self, table)

    def spatialInfo(self):
        ret = []
        if self.table.geomType is None:
            return ret

        tbl = [
            (QApplication.translate("DBManagerPlugin", "Column:"), self.table.geomColumn),
            (QApplication.translate("DBManagerPlugin", "Geometry:"), self.table.geomType)
        ]

        # only if we have info from geometry_columns
        srid = self.table.srid if self.table.srid is not None else -1
        sr_info = self.table.database().connector.getSpatialRefInfo(srid) if srid != -1 else QApplication.translate(
            "DBManagerPlugin", "Undefined")
        if sr_info:
            tbl.append((QApplication.translate("DBManagerPlugin", "Spatial ref:"), u"%s (%d)" % (sr_info, srid)))

        # extent
        if self.table.extent is not None and self.table.extent[0] is not None:
            extent_str = '%.5f, %.5f - %.5f, %.5f' % self.table.extent
        else:
            extent_str = QApplication.translate("DBManagerPlugin",
                                                '(unknown) (<a href="action:extent/get">find out</a>)')
        tbl.append((QApplication.translate("DBManagerPlugin", "Extent:"), extent_str))

        ret.append(HtmlTable(tbl))
        return ret
