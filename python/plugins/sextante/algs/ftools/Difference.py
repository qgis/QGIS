# -*- coding: utf-8 -*-

"""
***************************************************************************
    Difference.py
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


from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputVector import OutputVector
from sextante.algs.ftools import FToolsUtils as utils
from sextante.core.SextanteLog import SextanteLog
from sextante.core.GeoAlgorithm import GeoAlgorithm


class Difference(GeoAlgorithm):

    INPUT = "INPUT"
    INPUT2 = "INPUT2"
    OUTPUT = "OUTPUT"

    #===========================================================================
    # def getIcon(self):
    #    return QtGui.QIcon(os.path.dirname(__file__) + "/icons/difference.png")
    #===========================================================================

    def processAlgorithm(self, progress):
        vlayerA = QGisLayers.getObjectFromUri(self.getParameterValue(Difference.INPUT))
        vlayerB = QGisLayers.getObjectFromUri(self.getParameterValue(Difference.INPUT2))
        GEOS_EXCEPT = True
        FEATURE_EXCEPT = True
        vproviderA = vlayerA.dataProvider()
        vproviderB = vlayerB.dataProvider()
        fields = vproviderA.fields()
        # check for crs compatibility
        crsA = vproviderA.crs()
        crsB = vproviderB.crs()
        if not crsA.isValid() or not crsB.isValid():
            SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Difference. Invalid CRS. Results might be unexpected")
        else:
            if crsA != crsB:
                SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Difference. Non-matching CRSs. Results might be unexpected")
        writer = self.getOutputFromName(Difference.OUTPUT).getVectorWriter(fields, vproviderA.geometryType(), vproviderA.crs() )
        inFeatA = QgsFeature()
        inFeatB = QgsFeature()
        outFeat = QgsFeature()
        index = utils.createSpatialIndex(vlayerB)
        nElement = 0
        selectionA = QGisLayers.features(vlayerA)
        nFeat = len(selectionA)
        for inFeatA in selectionA:
            nElement += 1
            progress.setPercentage(nElement/float(nFeat) * 100)
            add = True
            geom = QgsGeometry( inFeatA.geometry() )
            diff_geom = QgsGeometry( geom )
            atMap = inFeatA.attributes()
            intersects = index.intersects( geom.boundingBox() )
            for id in intersects:
                vlayerB.featureAtId( int( id ), inFeatB , True, allAttrsB )
                tmpGeom = QgsGeometry( inFeatB.geometry() )
                try:
                    if diff_geom.intersects( tmpGeom ):
                        diff_geom = QgsGeometry( diff_geom.difference( tmpGeom ) )
                except:
                    GEOS_EXCEPT = False
                    add = False
                    break
            if add:
                try:
                    outFeat.setGeometry( diff_geom )
                    outFeat.setAttributes( atMap )
                    writer.addFeature( outFeat )
                except:
                    FEATURE_EXCEPT = False
                    continue


        del writer
        if not GEOS_EXCEPT:
            SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Geometry exception while computing difference")
        if not FEATURE_EXCEPT:
            SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Feature exception while computing difference")

    def defineCharacteristics(self):
        self.name = "Difference"
        self.group = "Vector overlay tools"
        self.addParameter(ParameterVector(Difference.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterVector(Difference.INPUT2, "Difference layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addOutput(OutputVector(Difference.OUTPUT, "Difference"))
