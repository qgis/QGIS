# -*- coding: utf-8 -*-

"""
***************************************************************************
    PointDistance.py
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

import csv
import math
import codecs
import cStringIO

from qgis.core import *

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.QGisLayers import QGisLayers

from processing.parameters.ParameterNumber import ParameterNumber
from processing.parameters.ParameterVector import ParameterVector
from processing.parameters.ParameterSelection import ParameterSelection
from processing.parameters.ParameterTableField import ParameterTableField

from processing.outputs.OutputFile import OutputFile

from processing.algs.ftools import FToolsUtils as utils

class PointDistance(GeoAlgorithm):

    INPUT_LAYER = "INPUT_LAYER"
    INPUT_FIELD = "INPUT_FIELD"
    TARGET_LAYER = "TARGET_LAYER"
    TARGET_FIELD = "TARGET_FIELD"
    MATRIX_TYPE = "MATRIX_TYPE"
    NEAREST_POINTS = "NEAREST_POINTS"
    DISTANCE_MATRIX = "DISTANCE_MATRIX"

    MAT_TYPES = ["Linear (N*k x 3) distance matrix",
                 "Standard (N x T) distance matrix",
                 "Summary distance matrix (mean, std. dev., min, max)"
                ]

    #===========================================================================
    # def getIcon(self):
    #    return QtGui.QIcon(os.path.dirname(__file__) + "/icons/matrix.png")
    #===========================================================================

    def defineCharacteristics(self):
        self.name = "Distance matrix"
        self.group = "Vector analysis tools"

        self.addParameter(ParameterVector(self.INPUT_LAYER, "Input point layer", [ParameterVector.VECTOR_TYPE_POINT]))
        self.addParameter(ParameterTableField(self.INPUT_FIELD, "Input unique ID field", self.INPUT_LAYER, ParameterTableField.DATA_TYPE_ANY))
        self.addParameter(ParameterVector(self.TARGET_LAYER, "Target point layer", ParameterVector.VECTOR_TYPE_POINT))
        self.addParameter(ParameterTableField(self.TARGET_FIELD, "Target unique ID field", self.TARGET_LAYER, ParameterTableField.DATA_TYPE_ANY))
        self.addParameter(ParameterSelection(self.MATRIX_TYPE, "Output matrix type", self.MAT_TYPES, 0))
        self.addParameter(ParameterNumber(self.NEAREST_POINTS, "Use only the nearest (k) target points", 0, 9999, 0))

        self.addOutput(OutputFile(self.DISTANCE_MATRIX, "Distance matrix"))

    def processAlgorithm(self, progress):
        inLayer = QGisLayers.getObjectFromUri(self.getParameterValue(self.INPUT_LAYER))
        inField = self.getParameterValue(self.INPUT_FIELD)
        targetLayer = QGisLayers.getObjectFromUri(self.getParameterValue(self.TARGET_LAYER))
        targetField = self.getParameterValue(self.TARGET_FIELD)
        matType = self.getParameterValue(self.MATRIX_TYPE)
        nPoints = self.getParameterValue(self.NEAREST_POINTS)

        outputFile = self.getOutputValue(self.DISTANCE_MATRIX)

        if nPoints < 1:
            nPoints = len(QGisLayers.features(targetLayer))

        # prepare CSV file writer
        csvFile = open(outputFile, "wb")
        self.writer = UnicodeWriter(csvFile)

        if matType == 0:   # Linear distance matrix
            self.linearMatrix(inLayer, inField, targetLayer, targetField, matType, nPoints, progress)
        elif matType == 1: # Standard distance matrix
            self.regularMatrix(inLayer, inField, targetLayer, targetField, nPoints, progress)
        elif matType == 2: # Summary distance matrix
            self.linearMatrix(inLayer, inField, targetLayer, targetField, matType, nPoints, progress)

        csvFile.close()
        del self.writer

    def linearMatrix(self, inLayer, inField, targetLayer, targetField, matType, nPoints, progress):
        if matType == 0:
            self.writer.writerow(["InputID", "TargetID", "Distance"])
        else:
            self.writer.writerow(["InputID", "MEAN", "STDDEV", "MIN", "MAX"])

        index = utils.createSpatialIndex(targetLayer);

        inIdx = inLayer.fieldNameIndex(inField)
        inLayer.select([inIdx])
        outIdx = targetLayer.fieldNameIndex(inField)

        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        outGeom = QgsGeometry()
        distArea = QgsDistanceArea()

        features = QGisLayers.features(inLayer)
        current = 0
        total = 100.0 / float(len(features))
        for inFeat in features:
            inGeom = inFeat.geometry()
            inID = inFeat.attributes()[inIdx]
            featList = index.nearestNeighbor(inGeom.asPoint(), nPoints)
            distList = []
            vari = 0.0
            for i in featList:
                request = QgsFeatureRequest().setFilterFid(i)
                outFeat = targetLayer.getFeatures(request).next()
                outID = outFeat.attributes()[outIdx]
                outGeom = outFeat.geometry()
                dist = distArea.measureLine(inGeom.asPoint(), outGeom.asPoint())
                if matType == 0:
                    self.writer.writerow([unicode(inID), unicode(outID), unicode(dist)])
                else:
                    distList.append(float(dist))

            if matType == 2:
                mean = sum(distList) / len(distList)
                for i in distList:
                    vari += (i - mean) * (i - mean)
                vari = math.sqrt(vari / len(distList))
                self.writer.writerow([unicode(inID), unicode(mean), unicode(vari), unicode(min(distList)), unicode(max(distList))])

            current += 1
            progress.setPercentage(int(current * total))

    def regularMatrix(self, inLayer, inField, targetLayer, targetField, nPoints, progress):
        index = utils.createSpatialIndex(targetLayer)

        inIdx = inLayer.fieldNameIndex(inField)
        outIdx = targetLayer.fieldNameIndex(inField)

        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        outGeom = QgsGeometry()
        distArea = QgsDistanceArea()

        first = True
        current = 0
        features = QGisLayers.features(inLayer)
        total = 100.0 / float(len(features))

        for inFeat in features:
            inGeom = inFeat.geometry()
            inID = inFeat.attributes()[inIdx]
            if first:
                featList = index.nearestNeighbor(inGeom.asPoint(), nPoints)
                first = False
                data = ["ID"]
                for i in featList:
                    request = QgsFeatureRequest().setFilterFid(i)
                    outFeat = targetLayer.getFeatures(request).next()
                    data.append(unicode(outFeat.attributes[outIdx]))
                self.writer.writerow(data)

            data = [unicode(inID)]
            for i in featList:
                request = QgsFeatureRequest().setFilterFid(i)
                outFeat = targetLayer.getFeatures(request).next()
                outGeom = outFeat.geometry()
                dist = distArea.measureLine(inGeom.asPoint(), outGeom.asPoint())
                data.append(unicode(float(dist)))
            self.writer.writerow(data)

            current += 1
            progress.setPercentage(int(current * total))

class UnicodeWriter:
    def __init__(self, f, dialect=csv.excel, encoding="utf-8", **kwds):
        self.queue = cStringIO.StringIO()
        self.writer = csv.writer(self.queue, dialect=dialect, **kwds)
        self.stream = f
        self.encoder = codecs.getincrementalencoder(encoding)()

    def writerow(self, row):
        try:
            self.writer.writerow([s.encode("utf-8") for s in row])
        except:
            self.writer.writerow(row)
        data = self.queue.getvalue()
        data = data.decode("utf-8")
        data = self.encoder.encode(data)
        self.stream.write(data)
        self.queue.truncate(0)

    def writerows(self, rows):
        for row in rows:
            self.writerow(row)
