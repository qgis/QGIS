import os
from sextante.lidar.lastools.LasToolsUtils import LasToolsUtils
from sextante.lidar.lastools.LasToolsAlgorithm import LasToolsAlgorithm
from sextante.parameters.ParameterFile import ParameterFile
from sextante.outputs.OutputFile import OutputFile
from sextante.parameters.ParameterNumber import ParameterNumber

class lassplit(LasToolsAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    NUM_POINTS = "NUM_POINTS"

    def defineCharacteristics(self):
        self.name = "lassplit"
        self.group = "Tools"
        self.addParameter(ParameterFile(lassplit.INPUT, "Input las layer"))
        self.addParameter(ParameterNumber(lassplit.NUM_POINTS, "Point in each output file", 1, None, 1000000))
        self.addOutput(OutputFile(lassplit.OUTPUT, "Output las file basename"))
        self.addCommonParameters()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LasToolsUtils.LasToolsPath(), "bin", "lassplit.exe")]
        commands.append("-i")
        commands.append(self.getParameterValue(lassplit.INPUT))
        commands.append("-o")
        commands.append(self.getOutputValue(lassplit.OUTPUT))
        commands.append("-split")
        commands.append(self.getParameterValue(lassplit.NUM_POINTS))
        self.addCommonParameterValuesToCommand(commands)

        LasToolsUtils.runLasTools(commands, progress)
