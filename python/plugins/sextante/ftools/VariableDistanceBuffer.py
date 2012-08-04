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
from sextante.parameters.ParameterTableField import ParameterTableField

class VariableDistanceBuffer(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    FIELD = "FIELD"
    USE_SELECTED = "USE_SELECTED"
    SEGMENTS = "SEGMENTS"
    DISSOLVE = "DISSOLVE"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/buffer.png")

    def processAlgorithm(self, progress):
        output = self.getOutputValue(VariableDistanceBuffer.OUTPUT)
        useSelection = self.getParameterValue(VariableDistanceBuffer.USE_SELECTED)
        dissolve = self.getParameterValue(VariableDistanceBuffer.DISSOLVE)
        field = self.getParameterValue(VariableDistanceBuffer.FIELD)
        segments = int(self.getParameterValue(VariableDistanceBuffer.SEGMENTS))
        layer = QGisLayers.getObjectFromUri(self.getParameterValue(VariableDistanceBuffer.INPUT))
        buff.buffering(progress, output, 0, field, useSelection, True, layer, dissolve, segments)

    def defineCharacteristics(self):
        self.name = "Variable distance buffer"
        self.group = "Geoprocessing tools"
        self.addParameter(ParameterVector(VariableDistanceBuffer.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterBoolean(VariableDistanceBuffer.USE_SELECTED, "Use selected features", False))
        self.addParameter(ParameterTableField(VariableDistanceBuffer.FIELD, "Distance field",VariableDistanceBuffer.INPUT ))
        self.addParameter(ParameterNumber(VariableDistanceBuffer.SEGMENTS, "Segments", 1, default=10 ))
        self.addParameter(ParameterBoolean(VariableDistanceBuffer.DISSOLVE, "Dissolve result", True))
        self.addOutput(OutputVector(VariableDistanceBuffer.OUTPUT, "Buffer"))

    #=========================================================
