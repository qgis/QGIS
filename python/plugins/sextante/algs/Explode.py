# -*- coding: utf-8 -*-

"""
***************************************************************************
    Explode.py
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

class Explode(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    #===========================================================================
    # def getIcon(self):
    #    return QtGui.QIcon(os.path.dirname(__file__) + "/../images/toolbox.png")
    #===========================================================================

    def processAlgorithm(self, progress):
        vlayer = QGisLayers.getObjectFromUri(self.getParameterValue(self.INPUT))
        output = self.getOutputFromName(self.OUTPUT)
        vprovider = vlayer.dataProvider()
        fields = vprovider.fields()
        writer = output.getVectorWriter(fields, QGis.WKBLineString, vlayer.crs() )
        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        nElement = 0
        features = QGisLayers.features(vlayer)
        nFeat = len(features)
        for feature in features:
            nElement += 1
            progress.setPercentage((nElement*100)/nFeat)
            inGeom = feature.geometry()
            atMap = feature.attributes()
            segments = self.extractAsSingleSegments( inGeom )
            outFeat.setAttributes( atMap )
            for segment in segments:
                outFeat.setGeometry(segment)
                writer.addFeature(outFeat)
        del writer


    def extractAsSingleSegments( self, geom ):
        segments = []
        if geom.isMultipart():
            multi = geom.asMultiPolyline()
            for polyline in multi:
                segments.extend( self.getPolylineAsSingleSegments(polyline))
        else:
            segments.extend( self.getPolylineAsSingleSegments(geom.asPolyline()))
        return segments

    def getPolylineAsSingleSegments(self, polyline):
        segments = []
        for i in range(len(polyline)-1):
            ptA = polyline[i]
            ptB = polyline[i+1]
            segment = QgsGeometry.fromPolyline([ptA, ptB])
            segments.append(segment)
        return segments

    def defineCharacteristics(self):
        self.name = "Explode lines"
        self.group = "Vector geometry tools"
        self.addParameter(ParameterVector(self.INPUT, "Input layer",ParameterVector.VECTOR_TYPE_LINE))
        self.addOutput(OutputVector(self.OUTPUT, "Output layer"))

