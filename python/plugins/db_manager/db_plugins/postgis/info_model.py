# -*- coding: utf-8 -*-

"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QuantumGIS
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

from PyQt4.QtCore import *
from PyQt4.QtGui import *

from ..info_model import TableInfo, VectorTableInfo, RasterTableInfo
from ..html_elems import HtmlSection, HtmlParagraph, HtmlList, HtmlTable, HtmlTableHeader, HtmlTableCol

class PGTableInfo(TableInfo):
	def __init__(self, table):
		self.table = table


	def generalInfo(self):
		ret = []

		# if the estimation is less than 100 rows, try to count them - it shouldn't take long time
		if self.table.rowCount == None and self.table.estimatedRowCount < 100:
			# row count information is not displayed yet, so just block
			# table signals to avoid double refreshing (infoViewer->refreshRowCount->tableChanged->infoViewer)
			self.table.blockSignals(True)
			self.table.refreshRowCount()
			self.table.blockSignals(False)

		tbl = [
			("Relation type:", "View" if self.table.isView else "Table"),
			("Owner:", self.table.owner)
		]
		if self.table.comment:
			tbl.append( ("Comment:", self.table.comment) )

		tbl.extend([
			("Pages:", self.table.pages),
			("Rows (estimation):", self.table.estimatedRowCount )
		])

		# privileges
		# has the user access to this schema?
		schema_priv = self.table.database().connector.getSchemaPrivileges(self.table.schemaName()) if self.table.schema() else None
		if schema_priv == None:
			pass
		elif schema_priv[1] == False:	# no usage privileges on the schema
			tbl.append( ("Privileges:", u"<warning> This user doesn't have usage privileges for this schema!" ) )
		else:
			table_priv = self.table.database().connector.getTablePrivileges( (self.table.schemaName(), self.table.name) )
			privileges = []
			if table_priv[0]:
				privileges.append("select")

				if self.table.rowCount == None or self.table.rowCount >= 0:
					tbl.append( ("Rows (counted):", self.table.rowCount if self.table.rowCount != None else 'Unknown (<a href="action:rows/count">find out</a>)') )

			if table_priv[1]: privileges.append("insert")
			if table_priv[2]: privileges.append("update")
			if table_priv[3]: privileges.append("delete")
			priv_string = u", ".join(privileges) if len(privileges) > 0 else u'<warning> This user has no privileges!'
			tbl.append( ("Privileges:", priv_string ) )

		ret.append( HtmlTable( tbl ) )

		if schema_priv != None and schema_priv[1]:
			if table_priv[0] and not table_priv[1] and not table_priv[2] and not table_priv[3]:
				ret.append( HtmlParagraph( u"<warning> This user has read-only privileges." ) )

		if not self.table.isView:
			if self.table.rowCount != None:
				if abs(self.table.estimatedRowCount - self.table.rowCount) > 1 and \
						(self.table.estimatedRowCount > 2 * self.table.rowCount or \
						self.table.rowCount > 2 * self.table.estimatedRowCount):
					ret.append( HtmlParagraph( u"<warning> There's a significant difference between estimated and real row count. " \
						'Consider running <a href="action:vacuumanalyze/run">VACUUM ANALYZE</a>.' ) )

		# primary key defined?
		if not self.table.isView:
			if len( filter(lambda fld: fld.primaryKey, self.table.fields()) ) <= 0:
				ret.append( HtmlParagraph( u"<warning> No primary key defined for this table!" ) )

		return ret

	def getSpatialInfo(self):
		ret = []

		info = self.db.connector.getSpatialInfo()
		if info == None:
			return

		tbl = [
			("Library:", info[0]),
			("Scripts:", info[3]),
			("GEOS:", info[1]),
			("Proj:", info[2])
		]
		ret.append( HtmlTable( tbl ) )

		if info[1] != None and info[1] != info[2]:
			ret.append( HtmlParagraph( u"<warning> Version of installed scripts doesn't match version of released scripts!\n" \
				"This is probably a result of incorrect PostGIS upgrade." ) )

		if not self.db.connector.has_geometry_columns:
			ret.append( HtmlParagraph( u"<warning> geometry_columns table doesn't exist!\n" \
				"This table is essential for many GIS applications for enumeration of tables." ) )
		elif not self.db.connector.has_geometry_columns_access:
			ret.append( HtmlParagraph( u"<warning> This user doesn't have privileges to read contents of geometry_columns table!\n" \
				"This table is essential for many GIS applications for enumeration of tables." ) )

		return ret


	def fieldsDetails(self):
		tbl = []

		# define the table header
		header = ( "#", "Name", "Type", "Length", "Null", "Default" )
		tbl.append( HtmlTableHeader( header ) )

		# add table contents
		for fld in self.table.fields():
			char_max_len = fld.charMaxLen if fld.charMaxLen != None and fld.charMaxLen != -1 else ""
			is_null_txt = "N" if fld.notNull else "Y"

			# make primary key field underlined
			attrs = {"class":"underline"} if fld.primaryKey else None
			name = HtmlTableCol( fld.name, attrs )

			tbl.append( (fld.num, name, fld.type2String(), char_max_len, is_null_txt, fld.default2String()) )

		return HtmlTable( tbl, {"class":"header"} )


	def triggersDetails(self):
		if self.table.triggers() == None or len(self.table.triggers()) <= 0:
			return None

		ret = []

		tbl = []
		# define the table header
		header = ( "Name", "Function", "Type", "Enabled" )
		tbl.append( HtmlTableHeader( header ) )

		# add table contents
		for trig in self.table.triggers():
			name = u'%(name)s (<a href="action:trigger/%(name)s/%(action)s">%(action)s</a>)' % { "name":trig.name, "action":"delete" }

			(enabled, action) = ("Yes", "disable") if trig.enabled else ("No", "enable")
			txt_enabled = u'%(enabled)s (<a href="action:trigger/%(name)s/%(action)s">%(action)s</a>)' % { "name":trig.name, "action":action, "enabled":enabled }

			tbl.append( (name, trig.function, trig.type2String(), txt_enabled) )

		ret.append( HtmlTable( tbl, {"class":"header"} ) )

		ret.append( HtmlParagraph( '<a href="action:triggers/enable">Enable all triggers</a> / <a href="action:triggers/disable">Disable all triggers</a>' ) )
		return ret


	def rulesDetails(self):
		if self.table.rules() == None or len(self.table.rules()) <= 0:
			return None

		tbl = []
		# define the table header
		header = ( "Name", "Definition" )
		tbl.append( HtmlTableHeader( header ) )

		# add table contents
		for rule in self.table.rules():
			name = u'%(name)s (<a href="action:rule/%(name)s/%(action)s">%(action)s</a>)' % { "name":rule.name, "action":"delete" }
			tbl.append( (name, rule.definition) )

		return HtmlTable( tbl, {"class":"header"} )


	def getTableInfo(self):
		ret = TableInfo.getTableInfo(self)

		# rules
		rules_details = self.rulesDetails()
		if rules_details == None:
			pass
		else:
			ret.append( HtmlSection( 'Rules', rules_details ) )

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
