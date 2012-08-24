from sextante.core.GeoAlgorithm import GeoAlgorithm
import os.path
from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputVector import OutputVector
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.ftools import Buffer as buff

class FixedDistanceBuffer(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    FIELD = "FIELD"
    USE_SELECTED = "USE_SELECTED"
    DISTANCE = "DISTANCE"
    SEGMENTS = "SEGMENTS"
    DISSOLVE = "DISSOLVE"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/buffer.png")

    def processAlgorithm(self, progress):
        vlayer = QGisLayers.getObjectFromUri(self.getParameterValue(FixedDistanceBuffer.INPUT))
        vprovider = vlayer.dataProvider()
        allAttrs = vprovider.attributeIndexes()
        vprovider.select(allAttrs)
        writer = self.getOutputFromName(FixedDistanceBuffer.OUTPUT).getVectorWriter(vprovider.fields(), QGis.WKBPolygon, vprovider.crs() )
        useSelection = self.getParameterValue(FixedDistanceBuffer.USE_SELECTED)
        distance = self.getParameterValue(FixedDistanceBuffer.DISTANCE)
        dissolve = self.getParameterValue(FixedDistanceBuffer.DISSOLVE)
        segments = int(self.getParameterValue(FixedDistanceBuffer.SEGMENTS))
        layer = QGisLayers.getObjectFromUri(self.getParameterValue(FixedDistanceBuffer.INPUT))
        buff.buffering(progress, writer, distance, None, useSelection, False, layer, dissolve, segments)

    def defineCharacteristics(self):
        self.name = "Fixed distance buffer"
        self.group = "Geoprocessing tools"
        self.addParameter(ParameterVector(FixedDistanceBuffer.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterBoolean(FixedDistanceBuffer.USE_SELECTED, "Use selected features", False))
        self.addParameter(ParameterNumber(FixedDistanceBuffer.DISTANCE, "Distance", 0, default=10 ))
        self.addParameter(ParameterNumber(FixedDistanceBuffer.SEGMENTS, "Segments", 1, default=10 ))
        self.addParameter(ParameterBoolean(FixedDistanceBuffer.DISSOLVE, "Dissolve result", True))
        self.addOutput(OutputVector(FixedDistanceBuffer.OUTPUT, "Buffer"))

    #=========================================================
