# -*- coding: utf-8 -*-

"""
***************************************************************************
    ogr2ogr.py
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

import tempfile
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterString import ParameterString
from sextante.outputs.OutputVector import OutputVector
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.gdal.OgrAlgorithm import OgrAlgorithm
from sextante.gdal.pyogr.ogr2ogr import *

try:
    from osgeo import gdal, ogr, osr
    gdalAvailable = True
except:
    gdalAvailable = False

GeomOperation = Enum(["NONE", "SEGMENTIZE", "SIMPLIFY_PRESERVE_TOPOLOGY"])

FORMATS = ['ESRI Shapefile','GeoJSON',' GeoRSS','SQLite','Generic Mapping Tools','Mapinfo TAB','ESRI Shapefile','INTERLIS 1',
           'Geography Markup Language','Geoconcept','AutoCAD DXF','INTERLIS 2','','Microstation DGN',
           'Comma Separated Value','Atlas BNAGPS eXchange Format','S-57 Base file','Keyhole Markup Language']
EXTS = ["shp",'geojson','.xml','.sqlite','.gmt','.tab','.shp','.ili','.gml','.txt','.dxf','.ili','.dgn','.csv','.bna','.gpx','.000','.kml']

class Ogr2Ogr(OgrAlgorithm):

    OUTPUT_LAYER = "OUTPUT_LAYER"
    INPUT_LAYER = "INPUT_LAYER"
    DEST_DS = "DEST_DS"
    DEST_FORMAT = "DEST_FORMAT"
    DEST_DSCO = "DEST_DSCO"

    def defineCharacteristics(self):
        self.name = "ogr2ogr"
        self.group = "[OGR] Transformation"

        #we add the input vector layer. It can have any kind of geometry
        #It is a mandatory (not optional) one, hence the False argument
        self.addParameter(ParameterVector(self.INPUT_LAYER, "Input layer", [ParameterVector.VECTOR_TYPE_ANY], False))
        self.addParameter(ParameterSelection(self.DEST_FORMAT, "Destination Format", FORMATS))
        self.addParameter(ParameterString(self.DEST_DSCO, "Creation Options", ""))

        self.addOutput(OutputVector(self.OUTPUT_LAYER, "Output layer"))

    def processAlgorithm(self, progress):
        '''Here is where the processing itself takes place'''

        if not gdalAvailable:
            raise GeoAlgorithmExecutionException("GDAL bindings not installed.")

        input = self.getParameterValue(self.INPUT_LAYER)
        ogrLayer = self.ogrConnectionString(input)

        output = self.getOutputFromName(self.OUTPUT_LAYER)
        outfile = output.value

        formatIdx = self.getParameterValue(self.DEST_FORMAT)

        ext = EXTS[formatIdx]
        if not outfile.endswith(ext):
            outfile = outfile + ext;
            output.value = outfile

        dst_ds = self.ogrConnectionString(outfile)
        dst_format = FORMATS[formatIdx]
        ogr_dsco = [self.getParameterValue(self.DEST_DSCO)]

        poDS = ogr.Open( ogrLayer, False )
        if poDS is None:
            raise GeoAlgorithmExecutionException(self.failure(ogrLayer))

        if dst_format == "SQLite" and os.path.isfile(dst_ds):
            os.remove(dst_ds)
        driver = ogr.GetDriverByName(str(dst_format))
        poDstDS = driver.CreateDataSource(dst_ds, options = ogr_dsco)
        if poDstDS is None:
            raise GeoAlgorithmExecutionException("Error creating %s" % dst_ds)
            return
        self.ogrtransform(poDS, poDstDS, bOverwrite = True)


    def ogrtransform(self,
                     poSrcDS,
                    poDstDS,
                    papszLayers = [],
                    papszLCO = [],
                    bTransform = False,
                    bAppend = False,
                    bUpdate = False,
                    bOverwrite = False,
                    poOutputSRS = None,
                    poSourceSRS = None,
                    pszNewLayerName = None,
                    pszWHERE = None,
                    papszSelFields = None,
                    eGType = -2,
                    eGeomOp = GeomOperation.NONE,
                    dfGeomOpParam = 0,
                    papszFieldTypesToString = [],
                    pfnProgress = None,
                    pProgressData = None,
                    nCountLayerFeatures = 0,
                    poClipSrc = None,
                    poClipDst = None,
                    bExplodeCollections = False,
                    pszZField = None):

        # Process each data source layer
        if len(papszLayers) == 0:
            nLayerCount = poSrcDS.GetLayerCount()
            papoLayers = [None for i in range(nLayerCount)]
            iLayer = 0

            for iLayer in range(nLayerCount):
                poLayer = poSrcDS.GetLayer(iLayer)

                if poLayer is None:
                    raise GeoAlgorithmExecutionException( "FAILURE: Couldn't fetch advertised layer %d!" % iLayer)

                papoLayers[iLayer] = poLayer
                iLayer = iLayer + 1

        # Process specified data source layers
        else:
            nLayerCount = len(papszLayers)
            papoLayers = [None for i in range(nLayerCount)]
            iLayer = 0

            for layername in papszLayers:
                poLayer = poSrcDS.GetLayerByName(layername)

                if poLayer is None:
                    raise GeoAlgorithmExecutionException("FAILURE: Couldn't fetch advertised layer %s!" % layername)

                papoLayers[iLayer] = poLayer
                iLayer = iLayer + 1

        for poSrcLayer in papoLayers:
          ok = TranslateLayer( poSrcDS, poSrcLayer, poDstDS, papszLCO, pszNewLayerName, \
                        bTransform, poOutputSRS, poSourceSRS, papszSelFields, \
                        bAppend, eGType, bOverwrite, eGeomOp, dfGeomOpParam, \
                        papszFieldTypesToString, nCountLayerFeatures, \
                        poClipSrc, poClipDst, bExplodeCollections, pszZField, pszWHERE, \
                        pfnProgress, pProgressData)
        return True


