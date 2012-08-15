from sextante.core.GeoAlgorithm import GeoAlgorithm
import os.path
from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputVector import OutputVector
from sextante.core.SextanteLog import SextanteLog
from sextante.ftools import ftools_utils
from sextante.parameters.ParameterString import ParameterString
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException

class SumLines(GeoAlgorithm):

    LINES = "LINES"
    POLYGONS = "POLYGONS"
    FIELD = "FIELD"
    OUTPUT = "OUTPUT"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/sum_lines.png")

    def processAlgorithm(self, progress):
        inField = self.getParameterValue(SumLines.FIELD)
        lineLayer = QGisLayers.getObjectFromUri(self.getParameterValue(SumLines.LINES))
        polyLayer = QGisLayers.getObjectFromUri(self.getParameterValue(SumLines.POLYGONS))
        polyProvider = polyLayer.dataProvider()
        lineProvider = lineLayer.dataProvider()
        if polyProvider.crs() <> lineProvider.crs():
            SextanteLog.addToLog(SextanteLog.LOG_WARNING,
                                 "CRS warning!Warning: Input layers have non-matching CRS.\nThis may cause unexpected results.")
        allAttrs = polyProvider.attributeIndexes()
        polyProvider.select(allAttrs)
        allAttrs = lineProvider.attributeIndexes()
        lineProvider.select(allAttrs)
        fieldList = ftools_utils.getFieldList(polyLayer)
        index = polyProvider.fieldNameIndex(unicode(inField))
        if index == -1:
            index = polyProvider.fieldCount()
            field = QgsField(unicode(inField), QVariant.Double, "real", 24, 15, self.tr("length field"))
            fieldList[index] = field
        sRs = polyProvider.crs()
        inFeat = QgsFeature()
        inFeatB = QgsFeature()
        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        outGeom = QgsGeometry()
        distArea = QgsDistanceArea()
        lineProvider.rewind()
        start = 15.00
        add = 85.00 / polyProvider.featureCount()
        writer = self.getOutputFromName(SumLines.OUTPUT).getVectorWriter(fieldList, polyProvider.geometryType(), sRs)
        spatialIndex = ftools_utils.createIndex( lineProvider )
        while polyProvider.nextFeature(inFeat):
            inGeom = QgsGeometry(inFeat.geometry())
            atMap = inFeat.attributeMap()
            lineList = []
            length = 0
            #(check, lineList) = lineLayer.featuresInRectangle(inGeom.boundingBox(), True, False)
            #lineLayer.select(inGeom.boundingBox(), False)
            #lineList = lineLayer.selectedFeatures()
            lineList = spatialIndex.intersects(inGeom.boundingBox())
            if len(lineList) > 0: check = 0
            else: check = 1
            if check == 0:
                for i in lineList:
                    lineProvider.featureAtId( int( i ), inFeatB , True, allAttrs )
                    tmpGeom = QgsGeometry( inFeatB.geometry() )
                    if inGeom.intersects(tmpGeom):
                        outGeom = inGeom.intersection(tmpGeom)
                        length = length + distArea.measure(outGeom)
            outFeat.setGeometry(inGeom)
            outFeat.setAttributeMap(atMap)
            outFeat.addAttribute(index, QVariant(length))
            writer.addFeature(outFeat)
            start = start + add
            progress.setPercentage(start)
        del writer


    def defineCharacteristics(self):
        self.name = "Sum line lengths"
        self.group = "Analysis tools"
        self.addParameter(ParameterVector(SumLines.LINES, "Lines", ParameterVector.VECTOR_TYPE_LINE))
        self.addParameter(ParameterVector(SumLines.POLYGONS, "Polygons", ParameterVector.VECTOR_TYPE_POLYGON))
        self.addParameter(ParameterString(SumLines.FIELD, "Output field name", "LENGTH"))
        self.addOutput(OutputVector(SumLines.OUTPUT, "Result"))

