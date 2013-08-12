# -*- coding: utf-8 -*-

"""
***************************************************************************
    Intersection.py
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
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from processing.core.GeoAlgorithm import GeoAlgorithm
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from processing.parameters.ParameterVector import ParameterVector
from processing.core.QGisLayers import QGisLayers
from processing.outputs.OutputVector import OutputVector
from processing.algs.ftools import FToolsUtils as utils
from processing.core.ProcessingLog import ProcessingLog

class Intersection(GeoAlgorithm):

    INPUT = "INPUT"
    INPUT2 = "INPUT2"
    OUTPUT = "OUTPUT"

    #===========================================================================
    # def getIcon(self):
    #    return QtGui.QIcon(os.path.dirname(__file__) + "/icons/intersect.png")
    #===========================================================================

    def processAlgorithm(self, progress):
        vlayerA = QGisLayers.getObjectFromUri(self.getParameterValue(Intersection.INPUT))
        vlayerB = QGisLayers.getObjectFromUri(self.getParameterValue(Intersection.INPUT2))
        vproviderA = vlayerA.dataProvider()

        fields = utils.combineVectorFields(vlayerA, vlayerB)
        writer = self.getOutputFromName(Intersection.OUTPUT).getVectorWriter(fields, vproviderA.geometryType(), vproviderA.crs() )
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
            geom = QgsGeometry( inFeatA.geometry() )
            atMapA = inFeatA.attributes()
            intersects = index.intersects( geom.boundingBox() )
            for i in intersects:
                request = QgsFeatureRequest().setFilterFid(i)
                inFeatB = vlayerB.getFeatures(request).next()
                tmpGeom = QgsGeometry(inFeatB.geometry())
                if geom.intersects( tmpGeom ):
                    atMapB = inFeatB.attributes()
                    int_geom = QgsGeometry( geom.intersection( tmpGeom ) )
                    if int_geom.wkbType() == 7:
                        int_com = geom.combine( tmpGeom )
                        int_sym = geom.symDifference( tmpGeom )
                        int_geom = QgsGeometry( int_com.difference( int_sym ) )
                    outFeat.setGeometry( int_geom )
                    attrs = []
                    attrs.extend(atMapA)
                    attrs.extend(atMapB)
                    outFeat.setAttributes(attrs)
                    writer.addFeature( outFeat )



        del writer


    def defineCharacteristics(self):
        self.name = "Intersection"
        self.group = "Vector overlay tools"
        self.addParameter(ParameterVector(Intersection.INPUT, "Input layer", [ParameterVector.VECTOR_TYPE_ANY]))
        self.addParameter(ParameterVector(Intersection.INPUT2, "Intersect layer", [ParameterVector.VECTOR_TYPE_ANY]))
        self.addOutput(OutputVector(Intersection.OUTPUT, "Intersection"))
