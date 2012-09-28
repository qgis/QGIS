import os.path
import csv
import math
import codecs
import cStringIO

from PyQt4 import QtGui
from qgis.core import *

from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.QGisLayers import QGisLayers
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException

from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.parameters.ParameterTableField import ParameterTableField

from sextante.outputs.OutputFile import OutputFile

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

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/matrix.png")

    def defineCharacteristics(self):
        self.name = "Distance matrix"
        self.group = "Analysis tools"

        self.addParameter(ParameterVector(PointDistance.INPUT_LAYER, "Input point layer", ParameterVector.VECTOR_TYPE_POINT))
        self.addParameter(ParameterTableField(PointDistance.INPUT_FIELD, "Input unique ID field", PointDistance.INPUT_LAYER, ParameterTableField.DATA_TYPE_ANY))
        self.addParameter(ParameterVector(PointDistance.TARGET_LAYER, "Target point layer", ParameterVector.VECTOR_TYPE_POINT))
        self.addParameter(ParameterTableField(PointDistance.TARGET_FIELD, "Target unique ID field", PointDistance.TARGET_LAYER, ParameterTableField.DATA_TYPE_ANY))
        self.addParameter(ParameterSelection(PointDistance.MATRIX_TYPE, "Output matrix type", PointDistance.MAT_TYPES, 0))
        self.addParameter(ParameterNumber(PointDistance.NEAREST_POINTS, "Use only the nearest (k) target points", 0, 9999, 0))

        self.addOutput(OutputFile(PointDistance.DISTANCE_MATRIX, "Distance matrix"))

    def processAlgorithm(self, progress):
        inLayer = QGisLayers.getObjectFromUri(self.getParameterValue(PointDistance.INPUT_LAYER))
        inField = self.getParameterValue(PointDistance.INPUT_FIELD)
        targetLayer = QGisLayers.getObjectFromUri(self.getParameterValue(PointDistance.TARGET_LAYER))
        targetField = self.getParameterValue(PointDistance.TARGET_FIELD)
        matType = self.getParameterValue(PointDistance.MATRIX_TYPE)
        nPoints = self.getParameterValue(PointDistance.NEAREST_POINTS)

        outputFile = self.getOutputValue(PointDistance.DISTANCE_MATRIX)

        if nPoints < 1:
            nPoints = targetLayer.featureCount()

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

        inProvider = inLayer.dataProvider()
        targetProvider = targetLayer.dataProvider()
        targetProvider.select()

        index = QgsSpatialIndex()
        inFeat = QgsFeature()
        while targetProvider.nextFeature(inFeat):
            index.insertFeature(inFeat)

        inIdx = inLayer.fieldNameIndex(inField)
        inProvider.select([inIdx])
        outIdx = targetLayer.fieldNameIndex(inField)
        targetProvider.rewind()
        targetProvider.select([outIdx])

        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        outGeom = QgsGeometry()
        distArea = QgsDistanceArea()

        current = 0
        count = inLayer.featureCount()

        while inProvider.nextFeature(inFeat):
            inGeom = inFeat.geometry()
            inID = inFeat.attributeMap()[inIdx].toString()
            featList = index.nearestNeighbor(inGeom.asPoint(), nPoints)
            distList = []
            vari = 0.0
            for i in featList:
                targetProvider.featureAtId(i, outFeat, True, [outIdx])
                outID = outFeat.attributeMap()[outIdx].toString()
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
            progress.setPercentage(int(current/count * 100))

    def regularMatrix(self, inLayer, inField, targetLayer, targetField, nPoints, progress):
        inProvider = inLayer.dataProvider()
        targetProvider = targetLayer.dataProvider()
        targetProvider.select()

        index = QgsSpatialIndex()
        inFeat = QgsFeature()
        while targetProvider.nextFeature(inFeat):
            index.insertFeature(inFeat)

        inIdx = inLayer.fieldNameIndex(inField)
        inProvider.select([inIdx])
        outIdx = targetLayer.fieldNameIndex(inField)
        targetProvider.rewind()
        targetProvider.select([outIdx])

        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        outGeom = QgsGeometry()
        distArea = QgsDistanceArea()

        first = True
        current = 0
        count = inLayer.featureCount()

        while inProvider.nextFeature(inFeat):
            inGeom = inFeat.geometry()
            inID = inFeat.attributeMap()[inIdx].toString()
            if first:
                featList = index.nearestNeighbor(inGeom.asPoint(), nPoints)
                first = False
                data = ["ID"]
                for i in featList:
                    targetProvider.featureAtId(i, outFeat, True, [outIdx])
                    data.append(unicode(outFeat.attributeMap()[outIdx].toString()))
                self.writer.writerow(data)

            data = [unicode(inID)]
            for i in featList:
                targetProvider.featureAtId(i, outFeat, True)
                outGeom = outFeat.geometry()
                dist = distArea.measureLine(inGeom.asPoint(), outGeom.asPoint())
                data.append(unicode(float(dist)))
            self.writer.writerow(data)

            current += 1
            progress.setPercentage(int(current/count * 100))

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
