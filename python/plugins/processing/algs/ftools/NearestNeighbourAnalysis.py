# -*- coding: utf-8 -*-

"""
***************************************************************************
    NearestNeighbourAnalysis.py
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

import math
from qgis.core import *
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.tools import dataobjects, vector
from processing.parameters.ParameterVector import ParameterVector
from processing.outputs.OutputHTML import OutputHTML
from processing.outputs.OutputNumber import OutputNumber
from processing.tools import vector as utils

class NearestNeighbourAnalysis(GeoAlgorithm):

    POINTS = "POINTS"

    OUTPUT = "OUTPUT"

    OBSERVED_MD = "OBSERVED_MD"
    EXPECTED_MD = "EXPECTED_MD"
    NN_INDEX = "NN_INDEX"
    POINT_COUNT = "POINT_COUNT"
    Z_SCORE = "Z_SCORE"

    #===========================================================================
    # def getIcon(self):
    #    return QtGui.QIcon(os.path.dirname(__file__) + "/icons/neighbour.png")
    #===========================================================================

    def defineCharacteristics(self):
        self.name = "Nearest neighbour analysis"
        self.group = "Vector analysis tools"

        self.addParameter(ParameterVector(self.POINTS, "Points", [ParameterVector.VECTOR_TYPE_POINT]))

        self.addOutput(OutputHTML(self.OUTPUT, "Result"))

        self.addOutput(OutputNumber(self.OBSERVED_MD, "Observed mean distance"))
        self.addOutput(OutputNumber(self.EXPECTED_MD, "Expected mean distance"))
        self.addOutput(OutputNumber(self.NN_INDEX, "Nearest neighbour index"))
        self.addOutput(OutputNumber(self.POINT_COUNT, "Number of points"))
        self.addOutput(OutputNumber(self.Z_SCORE, "Z-Score"))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(self.getParameterValue(self.POINTS))
        output = self.getOutputValue(self.OUTPUT)

        spatialIndex = utils.spatialindex(layer)

        neighbour = QgsFeature()
        distance = QgsDistanceArea()

        sumDist = 0.00
        A = layer.extent()
        A = float(A.width() * A.height())

        current = 0
        features = vector.features(layer)
        count = len(features)
        total = 100.0 / float(len(features))
        for feat in features:
            neighbourID = spatialIndex.nearestNeighbor(feat.geometry().asPoint(), 2)[1]
            request = QgsFeatureRequest().setFilterFid(neighbourID)
            neighbour = layer.getFeatures(request).next()
            sumDist += distance.measureLine(neighbour.geometry().asPoint(), feat.geometry().asPoint())

            current += 1
            progress.setPercentage(int(current * total))

        do = float(sumDist) / count
        de = float(0.5 / math.sqrt(count / A))
        d = float(do / de)
        SE = float(0.26136 / math.sqrt(( count ** 2) / A))
        zscore = float((do - de) / SE)

        data = []
        data.append("Observed mean distance: " + unicode(do))
        data.append("Expected mean distance: " + unicode(de))
        data.append("Nearest neighbour index: " + unicode(d))
        data.append("Number of points: " + unicode(count))
        data.append("Z-Score: " + unicode(zscore))

        self.createHTML(output, data)

        self.setOutputValue(self.OBSERVED_MD, float( data[ 0 ].split( ": " )[ 1 ] ) )
        self.setOutputValue(self.EXPECTED_MD, float( data[ 1 ].split( ": " )[ 1 ] ) )
        self.setOutputValue(self.NN_INDEX, float( data[ 2 ].split( ": " )[ 1 ] ) )
        self.setOutputValue(self.POINT_COUNT, float( data[ 3 ].split( ": " )[ 1 ] ) )
        self.setOutputValue(self.Z_SCORE, float( data[ 4 ].split( ": " )[ 1 ] ) )

    def createHTML(self, outputFile, algData):
        f = open(outputFile, "w")
        for s in algData:
            f.write("<p>" + str(s) + "</p>")
        f.close()
