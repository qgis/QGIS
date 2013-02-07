# -*- coding: utf-8 -*-

"""
***************************************************************************
    Clip.py
    ---------------------
    Date                 : August 2012
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
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from sextante.core.GeoAlgorithm import GeoAlgorithm
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputVector import OutputVector
from sextante.algs.ftools import FToolsUtils as utils
from sextante.core.SextanteLog import SextanteLog

class Clip(GeoAlgorithm):

    INPUT = "INPUT"
    INPUT2 = "INPUT2"
    OUTPUT = "OUTPUT"

    #===========================================================================
    # def getIcon(self):
    #    return QtGui.QIcon(os.path.dirname(__file__) + "/icons/clip.png")
    #===========================================================================

    def processAlgorithm(self, progress):
        settings = QSettings()
        systemEncoding = settings.value( "/UI/encoding", "System" ).toString()
        output = self.getOutputValue(Clip.OUTPUT)
        vlayerA = QGisLayers.getObjectFromUri(self.getParameterValue(Clip.INPUT))
        vlayerB = QGisLayers.getObjectFromUri(self.getParameterValue(Clip.INPUT2))
        GEOS_EXCEPT = True
        FEATURE_EXCEPT = True
        vproviderA = vlayerA.dataProvider()
        #allAttrsA = vproviderA.attributeIndexes()
        #vproviderA.select( allAttrsA )
        vproviderB = vlayerB.dataProvider()
        #allAttrsB = vproviderB.attributeIndexes()
        #vproviderB.select( allAttrsB )

        # check for crs compatibility
        crsA = vproviderA.crs()
        crsB = vproviderB.crs()
        if not crsA.isValid() or not crsB.isValid():
            SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Intersection. Invalid CRS. Results might be unexpected")
        else:
            if crsA != crsB:
                SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Intersection. Non-matching CRSs. Results might be unexpected")
        fields = vproviderA.fields()
        writer = QgsVectorFileWriter( output, systemEncoding,fields, vproviderA.geometryType(), vproviderA.crs() )

        inFeatA = QgsFeature()
        inFeatB = QgsFeature()
        outFeat = QgsFeature()
        index = utils.createSpatialIndex(vlayerB)
        #vproviderA.rewind()
        nElement = 0
        selectionA = QGisLayers.features(vlayerA)
        nFeat = len(selectionA)
        for inFeatA in selectionA:
            nElement += 1
            progress.setPercentage(nElement/float(nFeat) * 100)
            geom = QgsGeometry( inFeatA.geometry() )
            int_geom = QgsGeometry( geom )
            atMap = inFeatA.attributes()
            intersects = index.intersects(geom.boundingBox())
            found = False
            first = True
            for id in intersects:
                vlayerB.featureAtId(int(id), inFeatB)
                tmpGeom = QgsGeometry(inFeatB.geometry())
                if tmpGeom.intersects(geom):
                    found = True
                    if first:
                        outFeat.setGeometry(QgsGeometry(tmpGeom))
                        first = False
                    else:
                        try:
                            cur_geom = QgsGeometry( outFeat.geometry() )
                            new_geom = QgsGeometry( cur_geom.combine( tmpGeom ) )
                            outFeat.setGeometry( QgsGeometry( new_geom ) )
                        except:
                            GEOS_EXCEPT = False
                            break
            if found:
                try:
                    cur_geom = QgsGeometry( outFeat.geometry() )
                    new_geom = QgsGeometry( geom.intersection( cur_geom ) )
                    if new_geom.wkbType() == 7:
                        int_com = QgsGeometry( geom.combine( cur_geom ) )
                        int_sym = QgsGeometry( geom.symDifference( cur_geom ) )
                        new_geom = QgsGeometry( int_com.difference( int_sym ) )
                    try:
                        outFeat.setGeometry( new_geom )
                        outFeat.setAttributes( atMap )
                        writer.addFeature( outFeat )
                    except:
                        FEATURE_EXCEPT = False
                        continue
                except:
                    GEOS_EXCEPT = False
                    continue

        if not GEOS_EXCEPT:
            SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Geometry exception while computing clip")
        if not FEATURE_EXCEPT:
            SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Feature exception while computing clip")

    def defineCharacteristics(self):
        self.name = "Clip"
        self.group = "Vector overlay tools"
        self.addParameter(ParameterVector(Clip.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterVector(Clip.INPUT2, "Clip layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addOutput(OutputVector(Clip.OUTPUT, "Clipped"))
