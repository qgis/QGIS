from sextante.core.GeoAlgorithm import GeoAlgorithm
import os.path
from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputVector import OutputVector
from sextante.ftools import ftools_utils
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.parameters.ParameterTableField import ParameterTableField

class LinesIntersection(GeoAlgorithm):

    INPUT1 = "INPUT1"
    INPUT2 = "INPUT2"
    OUTPUT = "OUTPUT"
    FIELD1 = "FIELD1"
    FIELD2 = "FIELD2"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/intersections.png")

    def processAlgorithm(self, progress):
        settings = QSettings()
        systemEncoding = settings.value( "/UI/encoding", "System" ).toString()
        output = self.getOutputValue(LinesIntersection.OUTPUT)
        layer1 = QGisLayers.getObjectFromUri(self.getParameterValue(LinesIntersection.INPUT1))
        layer2 = QGisLayers.getObjectFromUri(self.getParameterValue(LinesIntersection.INPUT2))
        field1 = self.getParameterValue(LinesIntersection.FIELD1)
        field2 = self.getParameterValue(LinesIntersection.FIELD2)
        provider1 = layer1.dataProvider()
        provider2 = layer2.dataProvider()
        allAttrs = provider1.attributeIndexes()
        provider1.select(allAttrs)
        allAttrs = provider2.attributeIndexes()
        provider2.select(allAttrs)
        fieldList = ftools_utils.getFieldList(layer1)
        index1 = provider1.fieldNameIndex(field1)
        field1 = fieldList[index1]
        field1.setName(unicode(field1.name()) + "_1")
        fieldList = ftools_utils.getFieldList(layer2)
        index2 = provider2.fieldNameIndex(field2)
        field2 = fieldList[index2]
        field2.setName(unicode(field2.name()) + "_2")
        fieldList = {0:field1, 1:field2}
        sRs = provider1.crs()
        check = QFile(output)
        if check.exists():
            if not QgsVectorFileWriter.deleteShapeFile(output):
                raise GeoAlgorithmExecutionException("Could not delete existing output file")
        writer = QgsVectorFileWriter(output, systemEncoding, fieldList, QGis.WKBPoint, sRs)
        #writer = QgsVectorFileWriter(outPath, "UTF-8", fieldList, QGis.WKBPoint, sRs)
        inFeat = QgsFeature()
        inFeatB = QgsFeature()
        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        tempGeom = QgsGeometry()
        start = 15.00
        add = 85.00 / layer1.featureCount()
        index = ftools_utils.createIndex( provider2 )
        while provider1.nextFeature(inFeat):
            inGeom = inFeat.geometry()
            lineList = []
            #(check, lineList) = layer2.featuresInRectangle(inGeom.boundingBox(), True, True)
            # Below is a work-around for featuresInRectangle
            # Which appears to have been removed for QGIS version 1.0
            #layer2.select(inGeom.boundingBox(), False)
            #lineList = layer2.selectedFeatures()
            lineList = index.intersects( inGeom.boundingBox() )
            if len(lineList) > 0: check = 0
            else: check = 1
            if check == 0:
                for i in lineList:
                    provider2.featureAtId( int( i ), inFeatB , True, allAttrs )
                    tmpGeom = QgsGeometry( inFeatB.geometry() )
                    #tempGeom = i.geometry()
                    tempList = []
                    atMap1 = inFeat.attributeMap()
                    atMap2 = inFeatB.attributeMap()
                    if inGeom.intersects(tmpGeom):
                        tempGeom = inGeom.intersection(tmpGeom)
                        if tempGeom.type() == QGis.Point:
                            if tempGeom.isMultipart():
                                tempList = tempGeom.asMultiPoint()
                            else:
                                tempList.append(tempGeom.asPoint())
                            for j in tempList:
                                outFeat.setGeometry(tempGeom.fromPoint(j))
                                outFeat.addAttribute(0, atMap1[index1])
                                outFeat.addAttribute(1, atMap2[index2])
                                writer.addFeature(outFeat)
            start = start + add
            progress.setPercentage(start)
        del writer


    def defineCharacteristics(self):
        self.name = "Line intersections"
        self.group = "Analysis tools"
        self.addParameter(ParameterVector(LinesIntersection.INPUT1, "Input layer", ParameterVector.VECTOR_TYPE_LINE))
        self.addParameter(ParameterVector(LinesIntersection.INPUT2, "Intersect layer", ParameterVector.VECTOR_TYPE_LINE))
        self.addParameter(ParameterTableField(LinesIntersection.FIELD1, "Input unique ID field", LinesIntersection.INPUT1))
        self.addParameter(ParameterTableField(LinesIntersection.FIELD2, "Intersect unique ID field", LinesIntersection.INPUT2))
        self.addOutput(OutputVector(LinesIntersection.OUTPUT, "Output layer"))
    #=========================================================
