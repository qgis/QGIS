from PyQt4 import QtGui
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.outputs.OutputRaster import OutputRaster
import os
from sextante.gdal.GdalUtils import GdalUtils

class nearblack(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    NEAR = "NEAR"
    WHITE = "WHITE"

    def getIcon(self):
        filepath = os.path.dirname(__file__) + "/icons/nearblack.png"
        return QtGui.QIcon(filepath)

    def defineCharacteristics(self):
        self.name = "nearblack"
        self.group = "Analysis"
        self.addParameter(ParameterRaster(nearblack.INPUT, "Input layer", False))
        self.addParameter(ParameterNumber(nearblack.NEAR, "How far from black (white)", 0, None, 15))
        self.addParameter(ParameterBoolean(nearblack.WHITE, "Search for nearly white pixels instead of nearly black", False))
        self.addOutput(OutputRaster(nearblack.OUTPUT, "Output layer"))

    def processAlgorithm(self, progress):
        commands = ["nearblack"]
        commands.append("-o")
        commands.append(self.getOutputValue(nearblack.OUTPUT))
        commands.append("-near")
        commands.append(str(self.getParameterValue(nearblack.NEAR)))
        if self.getParameterValue(nearblack.WHITE):
            commands.append("-white")
        commands.append(self.getParameterValue(nearblack.INPUT))
        GdalUtils.runGdal(commands, progress)
