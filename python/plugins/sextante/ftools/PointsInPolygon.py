import os.path

from PyQt4 import QtGui
from PyQt4.QtCore import *

from qgis.core import *

from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.QGisLayers import QGisLayers
from sextante.core.SextanteLog import SextanteLog

from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterString import ParameterString

from sextante.outputs.OutputVector import OutputVector

from sextante.ftools import FToolsUtils as utils


class PointsInPolygon(GeoAlgorithm):

    POLYGONS = "POLYGONS"
    POINTS = "POINTS"
    OUTPUT = "OUTPUT"
    FIELD = "FIELD"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/sum_points.png")

    def defineCharacteristics(self):
        self.name = "Count points in polygon"
        self.group = "Analysis tools"

        self.addParameter(ParameterVector(self.POLYGONS, "Polygons", ParameterVector.VECTOR_TYPE_POLYGON))
        self.addParameter(ParameterVector(self.POINTS, "Points", ParameterVector.VECTOR_TYPE_POINT))
        self.addParameter(ParameterString(self.FIELD, "Count field name", "NUMPOINTS"))

        self.addOutput(OutputVector(self.OUTPUT, "Result"))

    def processAlgorithm(self, progress):
        polyLayer = QGisLayers.getObjectFromUri(self.getParameterValue(self.POLYGONS))
        pointLayer = QGisLayers.getObjectFromUri(self.getParameterValue(self.POINTS))
        fieldName = self.getParameterValue(self.FIELD)

        output = self.getOutputValue(self.OUTPUT)

        polyProvider = polyLayer.dataProvider()
        pointProvider = pointLayer.dataProvider()
        if polyProvider.crs() != pointProvider.crs():
            SextanteLog.addToLog(SextanteLog.LOG_WARNING,
                                 "CRS warning: Input layers have non-matching CRS. This may cause unexpected results.")

        idxCount, fieldList = utils.findOrCreateField(polyLayer, polyLayer.pendingFields(), fieldName)

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fieldList,
                     polyProvider.geometryType(), polyProvider.crs())

        spatialIndex = utils.createSpatialIndex(pointProvider)

        pointProvider.rewind()
        pointProvider.select()

        allAttrs = polyLayer.pendingAllAttributesList()
        polyLayer.select(allAttrs)

        ftPoly = QgsFeature()
        ftPoint = QgsFeature()
        outFeat = QgsFeature()
        geom = QgsGeometry()

        current = 0
        total = 100.0 / float(polyProvider.featureCount())
        hasIntersections = False

        while polyLayer.nextFeature(ftPoly):
            geom = ftPoly.geometry()
            atMap = ftPoly.attributeMap()

            count = 0
            hasIntersections = False
            points = spatialIndex.intersects(geom.boundingBox())
            if len(points) > 0:
                hasIntersections = True

            if hasIntersections:
                for i in points:
                    pointLayer.featureAtId(int(i), ftPoint, True, False)
                    tmpGeom = QgsGeometry(ftPoint.geometry())
                    if geom.contains(tmpGeom):
                        count += 1

            outFeat.setGeometry(geom)
            outFeat.setAttributeMap(atMap)
            outFeat.addAttribute(idxCount, QVariant(count))
            writer.addFeature(outFeat)

            current += 1
            progress.setPercentage(int(current * total))

        del writer
