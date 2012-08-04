import os
from PyQt4 import QtGui
from sextante.lidar.lastools.LasToolsUtils import LasToolsUtils
from sextante.lidar.lastools.LasToolsAlgorithm import LasToolsAlgorithm
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.parameters.ParameterFile import ParameterFile
from sextante.outputs.OutputFile import OutputFile

class lasground(LasToolsAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    INTENSITY = "INTENSITY"
    METHOD = "METHOD"
    METHODS = ["terrain", "town", "city"]

    def defineCharacteristics(self):
        self.name = "lasground"
        self.group = "Tools"
        self.addParameter(ParameterFile(lasground.INPUT, "Input las layer"))
        self.addParameter(ParameterSelection(lasground.METHOD, "Method", lasground.METHODS))
        self.addOutput(OutputFile(lasground.OUTPUT, "Output ground las file"))
        self.addCommonParameters()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LasToolsUtils.LasToolsPath(), "bin", "lasground.exe")]
        commands.append("-i")
        commands.append(self.getParameterValue(lasground.INPUT))
        commands.append("-o")
        commands.append(self.getOutputValue(lasground.OUTPUT))
        method = self.getParameterValue(lasground.METHOD)
        if method != 0:
            commands.append("-" + lasground.METHODS[method])
        self.addCommonParameterValuesToCommand(commands)

        LasToolsUtils.runLasTools(commands, progress)
