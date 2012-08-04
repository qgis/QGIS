from sextante.core.GeoAlgorithm import GeoAlgorithm
import os.path
from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.ftools import ftools_utils
from sextante.parameters.ParameterTableField import ParameterTableField
from sextante.outputs.OutputVector import OutputVector

class MeanCoords(GeoAlgorithm):

    POINTS = "POINTS"
    WEIGHT = "WEIGHT"
    OUTPUT = "OUTPUT"
    UID = "UID"
    WEIGHT = "WEIGHT"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/mean.png")

    def processAlgorithm(self, progress):
        settings = QSettings()
        systemEncoding = settings.value( "/UI/encoding", "System" ).toString()
        output = self.getOutputValue(MeanCoords.OUTPUT)
        vlayer = QGisLayers.getObjectFromUri(self.getParameterValue(MeanCoords.POINTS))
        weightField = self.getParameterValue(MeanCoords.WEIGHT)
        uniqueField = self.getParameterValue(MeanCoords.UID)
        provider = vlayer.dataProvider()
        weightIndex = provider.fieldNameIndex(weightField)
        uniqueIndex = provider.fieldNameIndex(uniqueField)
        feat = QgsFeature()
        allAttrs = provider.attributeIndexes()
        provider.select(allAttrs)
        sRs = provider.crs()
        if uniqueIndex <> -1:
            uniqueValues = ftools_utils.getUniqueValues(provider, int( uniqueIndex ) )
            single = False
        else:
            uniqueValues = [QVariant(1)]
            single = True
        fieldList = { 0 : QgsField("MEAN_X", QVariant.Double), 1 : QgsField("MEAN_Y", QVariant.Double), 2 : QgsField("UID", QVariant.String)  }
        writer = QgsVectorFileWriter(output, systemEncoding, fieldList, QGis.WKBPoint, sRs)
        outfeat = QgsFeature()
        points = []
        weights = []
        nFeat = provider.featureCount() * len(uniqueValues)
        nElement = 0
        for j in uniqueValues:
            provider.rewind()
            provider.select(allAttrs)
            cx = 0.00
            cy = 0.00
            points = []
            weights = []
            while provider.nextFeature(feat):
                nElement += 1
                progress.setPercentage(nElement/nFeat * 100)
                if single:
                    check = j.toString().trimmed()
                else:
                    check = feat.attributeMap()[uniqueIndex].toString().trimmed()
                if check == j.toString().trimmed():
                    cx = 0.00
                    cy = 0.00
                    if weightIndex == -1:
                        weight = 1.00
                    else:
                        try:
                            weight = float(feat.attributeMap()[weightIndex].toDouble()[0])
                        except:
                            weight = 1.00
                    geom = QgsGeometry(feat.geometry())
                    geom = ftools_utils.extractPoints(geom)
                    for i in geom:
                        cx += i.x()
                        cy += i.y()
                    points.append(QgsPoint((cx / len(geom)), (cy / len(geom))))
                    weights.append(weight)
            sumWeight = sum(weights)
            cx = 0.00
            cy = 0.00
            item = 0
            for item, i in enumerate(points):
                cx += i.x() * weights[item]
                cy += i.y() * weights[item]
            cx = cx / sumWeight
            cy = cy / sumWeight
            meanPoint = QgsPoint(cx, cy)
            outfeat.setGeometry(QgsGeometry.fromPoint(meanPoint))
            outfeat.addAttribute(0, QVariant(cx))
            outfeat.addAttribute(1, QVariant(cy))
            outfeat.addAttribute(2, QVariant(j))
            writer.addFeature(outfeat)
            if single:
                break
        del writer



    def defineCharacteristics(self):
        self.name = "Mean coordinate(s)"
        self.group = "Analysis tools"
        self.addParameter(ParameterVector(MeanCoords.POINTS, "Input layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterTableField(MeanCoords.WEIGHT, "Weight field", MeanCoords.POINTS))
        self.addParameter(ParameterTableField(MeanCoords.UID, "Unique ID field", MeanCoords.POINTS))
        self.addOutput(OutputVector(MeanCoords.OUTPUT, "Result"))