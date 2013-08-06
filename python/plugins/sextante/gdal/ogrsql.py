# -*- coding: utf-8 -*-

"""
***************************************************************************
    ogrsql.py
    ---------------------
    Date                 : November 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'November 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from sextante.outputs.OutputHTML import OutputHTML
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterString import ParameterString
from sextante.core.SextanteLog import SextanteLog

from qgis.core import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
import string
import re

try:
  from osgeo import ogr
  ogrAvailable = True
except:
  ogrAvailable = False

from sextante.gdal.OgrAlgorithm import OgrAlgorithm

class OgrSql(OgrAlgorithm):

    #constants used to refer to parameters and outputs.
    #They will be used when calling the algorithm from another algorithm,
    #or when calling SEXTANTE from the QGIS console.
    OUTPUT = "OUTPUT"
    INPUT_LAYER = "INPUT_LAYER"
    SQL = "SQL"

    def defineCharacteristics(self):
        self.name = "Execute SQL"
        self.group = "[OGR] Miscellaneous"

        #we add the input vector layer. It can have any kind of geometry
        #It is a mandatory (not optional) one, hence the False argument
        self.addParameter(ParameterVector(self.INPUT_LAYER, "Input layer", [ParameterVector.VECTOR_TYPE_ANY], False))
        self.addParameter(ParameterString(self.SQL, "SQL", ""))

        self.addOutput(OutputHTML(self.OUTPUT, "SQL result"))


    def processAlgorithm(self, progress):
        if not ogrAvailable:
            SextanteLog.addToLog(SextanteLog.LOG_ERROR, "OGR bindings not installed" )
            return

        input = self.getParameterValue(self.INPUT_LAYER)
        sql = self.getParameterValue(self.SQL)
        ogrLayer = self.ogrConnectionString(input)

        output = self.getOutputValue(self.OUTPUT)

        qDebug("Opening data source '%s'" % ogrLayer)
        poDS = ogr.Open( ogrLayer, False )
        if poDS is None:
            SextanteLog.addToLog(SextanteLog.LOG_ERROR, self.failure(ogrLayer))
            return

        result = self.select_values(poDS, sql)

        f = open(output, "w")
        f.write("<table>")
        for row in result:
            f.write("<tr>")
            for col in row:
                f.write("<td>"+col+"</td>")
            f.write("</tr>")
        f.write("</table>")
        f.close()

    def execute_sql(self, ds, sql_statement):
        poResultSet = ds.ExecuteSQL( sql_statement, None, None )
        if poResultSet is not None:
          ds.ReleaseResultSet( poResultSet )

    def select_values(self, ds, sql_statement):
        """Returns an array of the columns and values of select statement:
            select_values(ds, "SELECT id FROM companies") => [['id'],[1],[2],[3]]
        """
        poResultSet = ds.ExecuteSQL( sql_statement, None, None )
        #TODO: redirect error messages
        fields = []
        rows = []
        if poResultSet is not None:
            poDefn = poResultSet.GetLayerDefn()
            for iField in range(poDefn.GetFieldCount()):
                poFDefn = poDefn.GetFieldDefn(iField)
                fields.append(poFDefn.GetNameRef())
            #poGeometry = poFeature.GetGeometryRef()
            #if poGeometry is not None:
            #    line = ("  %s = [GEOMETRY]" % poGeometry.GetGeometryName() )

            poFeature = poResultSet.GetNextFeature()
            while poFeature is not None:
                values = []
                for iField in range(poDefn.GetFieldCount()):
                    if poFeature.IsFieldSet( iField ):
                        values.append(poFeature.GetFieldAsString( iField ))
                    else:
                        values.append("(null)")
                rows.append(values)
                poFeature = poResultSet.GetNextFeature()
            ds.ReleaseResultSet( poResultSet )
        return [fields] + rows
