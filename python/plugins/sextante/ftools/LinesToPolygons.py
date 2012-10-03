import os.path

from PyQt4 import QtGui
from PyQt4.QtCore import *

from qgis.core import *

from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.QGisLayers import QGisLayers

from sextante.parameters.ParameterVector import ParameterVector

from sextante.outputs.OutputVector import OutputVector

class LinesToPolygons(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/to_lines.png")

    def defineCharacteristics(self):
        self.name = "Lines to polygons"
        self.group = "Geometry tools"

        self.addParameter(ParameterVector(self.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_LINE))
        self.addOutput(OutputVector(self.OUTPUT, "Output layer"))

    def processAlgorithm(self, progress):
        settings = QSettings()
        encoding = settings.value("/UI/encoding", "System").toString()

        layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.INPUT))
        output = self.getOutputValue(self.OUTPUT)

        provider = layer.dataProvider()
        layer.select(layer.pendingAllAttributesList())

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(layer.pendingFields(),
                     QGis.WKBPolygon, provider.crs())

        inFeat = QgsFeature()
        outFeat = QgsFeature()
        inGeom = QgsGeometry()

        current = 0
        total = 100.0 / float(provider.featureCount())

        while layer.nextFeature(inFeat):
            outGeomList = []
            multi = False

            if inFeat.geometry().isMultipart():
                outGeomList = inFeat.geometry().asMultiPolyline()
                multi = True
            else:
                outGeomList.append(inFeat.geometry().asPolyline())

            polyGeom = self.removeBadLines(outGeomList)
            if len(polyGeom) <> 0:
                outFeat.setGeometry(QgsGeometry.fromPolygon(polyGeom))
                atMap = inFeat.attributeMap()
                outFeat.setAttributeMap(atMap)
                writer.addFeature(outFeat)

            current += 1
            progress.setPercentage(int(current * total))

        del writer

    def removeBadLines(self, lines):
        geom = []
        if len(lines) == 1:
            if len(lines[0]) > 2:
                geom = lines
            else:
                geom = []
        else:
            geom = [elem for elem in lines if len(elem) > 2]
        return geom
