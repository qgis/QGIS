# -*- coding: utf-8 -*-

"""
***************************************************************************
    ogrinfo.py
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

from processing.outputs.OutputHTML import OutputHTML
from processing.parameters.ParameterVector import ParameterVector
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

from processing.gdal.OgrAlgorithm import OgrAlgorithm

class OgrInfo(OgrAlgorithm):

    #constants used to refer to parameters and outputs.
    #They will be used when calling the algorithm from another algorithm,
    #or when calling from the QGIS console.
    OUTPUT = "OUTPUT"
    INPUT_LAYER = "INPUT_LAYER"

    def defineCharacteristics(self):
        self.name = "ogrinfo"
        self.group = "[OGR] Miscellaneous"

        self.addParameter(ParameterVector(self.INPUT_LAYER, "Input layer", [ParameterVector.VECTOR_TYPE_ANY], False))

        self.addOutput(OutputHTML(self.OUTPUT, "Layer information"))


    def processAlgorithm(self, progress):

        input = self.getParameterValue(self.INPUT_LAYER)
        ogrLayer = self.ogrConnectionString(input)

        output = self.getOutputValue(self.OUTPUT)

        self.ogrinfo( ogrLayer )

        f = open(output, "w")
        f.write("<pre>" + self.info + "</pre>")
        f.close()

    def out(self, text):
        self.info = self.info + text + '\n'

    def ogrinfo(self, pszDataSource):
        bVerbose = True
        bSummaryOnly = True
        self.info = ''

        if not ogrAvailable:
            self.info = "OGR bindings not installed"
            return

        qDebug("Opening data source '%s'" % pszDataSource)
        poDS = ogr.Open( pszDataSource, False )

        if poDS is None:
            self.info = self.failure(pszDataSource)
            return

        poDriver = poDS.GetDriver()

        if bVerbose:
            self.out( "INFO: Open of `%s'\n"
                    "      using driver `%s' successful." % (pszDataSource, poDriver.GetName()) )

        poDS_Name = poDS.GetName()
        if str(type(pszDataSource)) == "<type 'unicode'>" and str(type(poDS_Name)) == "<type 'str'>":
            poDS_Name = unicode(poDS_Name, "utf8")
        if bVerbose and pszDataSource != poDS_Name:
            self.out( "INFO: Internal data source name `%s'\n"
                    "      different from user name `%s'." % (poDS_Name, pszDataSource ))
        #/* -------------------------------------------------------------------- */
        #/*      Process each data source layer.                                 */
        #/* -------------------------------------------------------------------- */
        for iLayer in range(poDS.GetLayerCount()):
            poLayer = poDS.GetLayer(iLayer)

            if poLayer is None:
                self.out( "FAILURE: Couldn't fetch advertised layer %d!" % iLayer )
                return 1

            self.ReportOnLayer( poLayer )

    def ReportOnLayer( self, poLayer, pszWHERE=None, poSpatialFilter=None ):
        bVerbose = True

        poDefn = poLayer.GetLayerDefn()

    #/* -------------------------------------------------------------------- */
    #/*      Set filters if provided.                                        */
    #/* -------------------------------------------------------------------- */
        if pszWHERE is not None:
            if poLayer.SetAttributeFilter( pszWHERE ) != 0:
                self.out("FAILURE: SetAttributeFilter(%s) failed." % pszWHERE)
                return

        if poSpatialFilter is not None:
            poLayer.SetSpatialFilter( poSpatialFilter )

    #/* -------------------------------------------------------------------- */
    #/*      Report various overall information.                             */
    #/* -------------------------------------------------------------------- */
        self.out( "" )

        self.out( "Layer name: %s" % poDefn.GetName() )

        if bVerbose:
            self.out( "Geometry: %s" % ogr.GeometryTypeToName( poDefn.GetGeomType() ) )

            self.out( "Feature Count: %d" % poLayer.GetFeatureCount() )

            oExt = poLayer.GetExtent(True, can_return_null = True)
            if oExt is not None:
                self.out("Extent: (%f, %f) - (%f, %f)" % (oExt[0], oExt[1], oExt[2], oExt[3]))

            if poLayer.GetSpatialRef() is None:
                pszWKT = "(unknown)"
            else:
                pszWKT = poLayer.GetSpatialRef().ExportToPrettyWkt()

            self.out( "Layer SRS WKT:\n%s" % pszWKT )

            if len(poLayer.GetFIDColumn()) > 0:
                self.out( "FID Column = %s" % poLayer.GetFIDColumn() )

            if len(poLayer.GetGeometryColumn()) > 0:
                self.out( "Geometry Column = %s" % poLayer.GetGeometryColumn() )

            for iAttr in range(poDefn.GetFieldCount()):
                poField = poDefn.GetFieldDefn( iAttr )

                self.out( "%s: %s (%d.%d)" % ( \
                        poField.GetNameRef(), \
                        poField.GetFieldTypeName( poField.GetType() ), \
                        poField.GetWidth(), \
                        poField.GetPrecision() ))
