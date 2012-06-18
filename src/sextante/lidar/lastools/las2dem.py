import os
from PyQt4 import QtGui
from sextante.parameters.ParameterString import ParameterString
from sextante.lidar.lastools.LasToolsUtils import LasToolsUtils
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.outputs.OutputRaster import OutputRaster
from sextante.lidar.lastools.LasToolsAlgorithm import LasToolsAlgorithm
from sextante.parameters.ParameterFile import ParameterFile

class las2dem(LasToolsAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    INTENSITY = "INTENSITY"

    def defineCharacteristics(self):
        self.name = "las2dem"
        self.group = "Tools"
        self.addParameter(ParameterFile(las2dem.INPUT, "Input las layer"))
        self.addParameter(ParameterBoolean(las2dem.INTENSITY, "Use intensity instead of elevation", False))
        self.addOutput(OutputRaster(las2dem.OUTPUT, "Output dem layer"))
        self.addCommonParameters()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LasToolsUtils.LasToolsPath(), "bin", "las2dem.exe")]
        commands.append("-i")
        commands.append(self.getParameterValue(las2dem.INPUT))
        commands.append("-o")
        commands.append(self.getOutputValue(las2dem.OUTPUT))
        if self.getParameterValue(las2dem.INTENSITY):
            commands.append("-intensity")
        self.addCommonParameterValuesToCommand(commands)

        LasToolsUtils.runLasTools(commands, progress)
