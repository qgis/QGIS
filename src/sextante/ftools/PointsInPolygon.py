from sextante.core.GeoAlgorithm import GeoAlgorithm
import os.path
from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.outputs.OutputVector import OutputVector
from sextante.ftools import ftools_utils
from sextante.parameters.ParameterString import ParameterString
from sextante.core.SextanteLog import SextanteLog

class PointsInPolygon(GeoAlgorithm):

    POLYGONS = "POLYGONS"
    POINTS = "POINTS"
    OUTPUT = "OUTPUT"
    FIELD = "FIELD"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/sum_points.png")

    def processAlgorithm(self, progress):
        inField = self.getParameterValue(PointsInPolygon.FIELD)
        polyLayer = QGisLayers.getObjectFromUri(self.getParameterValue(PointsInPolygon.POLYGONS))
        pointLayer = QGisLayers.getObjectFromUri(self.getParameterValue(PointsInPolygon.POINTS))
        polyProvider = polyLayer.dataProvider()
        pointProvider = pointLayer.dataProvider()
        if polyProvider.crs() <> pointProvider.crs():
            SextanteLog.addToLog(SextanteLog.LOG_WARNING,
                                 "CRS warning!Warning: Input layers have non-matching CRS.\nThis may cause unexpected results.")
        allAttrs = polyProvider.attributeIndexes()
        polyProvider.select(allAttrs)
        allAttrs = pointProvider.attributeIndexes()
        pointProvider.select(allAttrs)
        fieldList = ftools_utils.getFieldList(polyLayer)
        index = polyProvider.fieldNameIndex(unicode(inField))
        if index == -1:
            index = polyProvider.fieldCount()
            field = QgsField(unicode(inField), QVariant.Double, "real", 24, 15, "point count field")
            fieldList[index] = field
        sRs = polyProvider.crs()
        writer = self.getOutputFromName(PointsInPolygon.OUTPUT).getVectorWriter(fieldList, polyProvider.geometryType(), sRs)
        inFeat = QgsFeature()
        inFeatB = QgsFeature()
        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        start = 15.00
        add = 85.00 / polyProvider.featureCount()
        spatialIndex = ftools_utils.createIndex( pointProvider )
        while polyProvider.nextFeature(inFeat):
            inGeom = inFeat.geometry()
            atMap = inFeat.attributeMap()
            outFeat.setAttributeMap(atMap)
            outFeat.setGeometry(inGeom)
            pointList = []
            count = 0
            #(check, pointList) = pointLayer.featuresInRectangle(inGeom.boundingBox(), True, True)
            #pointLayer.select(inGeom.boundingBox(), False)
            #pointList = pointLayer.selectedFeatures()
            pointList = spatialIndex.intersects(inGeom.boundingBox())
            if len(pointList) > 0: check = 0
            else: check = 1
            if check == 0:
                for i in pointList:
                    pointProvider.featureAtId( int( i ), inFeatB , True, allAttrs )
                    tmpGeom = QgsGeometry( inFeatB.geometry() )
                    if inGeom.contains(tmpGeom):
                        count = count + 1
            outFeat.setAttributeMap(atMap)
            outFeat.addAttribute(index, QVariant(count))
            writer.addFeature(outFeat)
            start = start + add
            progress.setPercentage(start)
        del writer

    def defineCharacteristics(self):
        self.name = "Count points in polygon"
        self.group = "Analysis tools"
        self.addParameter(ParameterVector(PointsInPolygon.POLYGONS, "Polygons", ParameterVector.VECTOR_TYPE_POLYGON))
        self.addParameter(ParameterVector(PointsInPolygon.POINTS, "Points", ParameterVector.VECTOR_TYPE_POINT))
        self.addParameter(ParameterString(PointsInPolygon.FIELD, "Count field name", "NUMPOINTS"))
        self.addOutput(OutputVector(PointsInPolygon.OUTPUT, "Result"))

