# -*- coding: utf-8 -*-

"""
***************************************************************************
    SimplifyGeometries.py
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

from qgis.core import *

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.tools import dataobjects, vector
from processing.core.ProcessingLog import ProcessingLog
from processing.parameters.ParameterVector import ParameterVector
from processing.parameters.ParameterNumber import ParameterNumber
from processing.outputs.OutputVector import OutputVector

class SimplifyGeometries(GeoAlgorithm):

    INPUT = "INPUT"
    TOLERANCE = "TOLERANCE"
    OUTPUT = "OUTPUT"

    #===========================================================================
    # def getIcon(self):
    #    return QtGui.QIcon(os.path.dirname(__file__) + "/icons/simplify.png")
    #===========================================================================

    def defineCharacteristics(self):
        self.name = "Simplify geometries"
        self.group = "Vector geometry tools"

        self.addParameter(ParameterVector(self.INPUT, "Input layer", [ParameterVector.VECTOR_TYPE_POLYGON, ParameterVector.VECTOR_TYPE_LINE]))
        self.addParameter(ParameterNumber(self.TOLERANCE, "Tolerance", 0.0, 10000000.0, 1.0))

        self.addOutput(OutputVector(self.OUTPUT, "Simplified layer"))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT))
        tolerance =self.getParameterValue(self.TOLERANCE)

        pointsBefore = 0
        pointsAfter = 0

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(layer.pendingFields().toList(),
                     layer.wkbType(), layer.crs())

        current = 0
        selection = vector.features(layer)
        total =  100.0 / float(len(selection))
        for f in selection:
            featGeometry = QgsGeometry(f.geometry())
            attrs = f.attributes()
            pointsBefore += self.geomVertexCount(featGeometry)
            newGeometry = featGeometry.simplify(tolerance)
            pointsAfter += self.geomVertexCount(newGeometry)
            feature = QgsFeature()
            feature.setGeometry(newGeometry)
            feature.setAttributes(attrs)
            writer.addFeature(feature)
            current += 1
            progress.setPercentage(int(current * total))

        del writer

        ProcessingLog.addToLog(ProcessingLog.LOG_INFO, "Simplify: Input geometries have been simplified from"
                             + str(pointsBefore) + " to "  + str(pointsAfter) + " points.")

    def geomVertexCount(self, geometry):
        geomType = geometry.type()

        if geomType == QGis.Line:
            if geometry.isMultipart():
                pointsList = geometry.asMultiPolyline()
                points = sum( pointsList, [] )
            else:
                points = geometry.asPolyline()
            return len( points )
        elif geomType == QGis.Polygon:
            if geometry.isMultipart():
                polylinesList = geometry.asMultiPolygon()
                polylines = sum( polylinesList, [] )
            else:
                polylines = geometry.asPolygon()

            points = []
            for l in polylines:
                points.extend( l )

            return len( points )
        else:
            return None
