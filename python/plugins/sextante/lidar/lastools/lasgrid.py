import os
from PyQt4 import QtGui
from sextante.parameters.ParameterString import ParameterString
from sextante.lidar.lastools.LasToolsUtils import LasToolsUtils
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.outputs.OutputRaster import OutputRaster
from sextante.lidar.lastools.LasToolsAlgorithm import LasToolsAlgorithm
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.parameters.ParameterFile import ParameterFile

class lasgrid(LasToolsAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    INTENSITY = "INTENSITY"
    METHOD = "METHOD"
    METHODS = ["-average", "-lowest", "-highest", "-stddev"]

    def defineCharacteristics(self):
        self.name = "lasgrid"
        self.group = "Tools"
        self.addParameter(ParameterFile(lasgrid.INPUT, "Input las layer"))
        self.addParameter(ParameterBoolean(lasgrid.INTENSITY, "Use intensity instead of elevation", False))
        self.addParameter(ParameterSelection(lasgrid.METHOD, "Method", lasgrid.METHODS))
        self.addOutput(OutputRaster(lasgrid.OUTPUT, "Output grid layer"))
        self.addCommonParameters()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LasToolsUtils.LasToolsPath(), "bin", "lasgrid.exe")]
        commands.append("-i")
        commands.append(self.getParameterValue(lasgrid.INPUT))
        commands.append("-o")
        commands.append(self.getOutputValue(lasgrid.OUTPUT))
        if self.getParameterValue(lasgrid.INTENSITY):
            commands.append("-intensity")
        commands.append(lasgrid.METHODS[self.getParameterValue(lasgrid.METHOD)])
        self.addCommonParameterValuesToCommand(commands)

        LasToolsUtils.runLasTools(commands, progress)
