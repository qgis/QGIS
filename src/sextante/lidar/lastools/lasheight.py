import os
from sextante.lidar.lastools.LasToolsUtils import LasToolsUtils
from sextante.lidar.lastools.LasToolsAlgorithm import LasToolsAlgorithm
from sextante.parameters.ParameterFile import ParameterFile
from sextante.outputs.OutputFile import OutputFile

class lasheight(LasToolsAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def defineCharacteristics(self):
        self.name = "lasheight"
        self.group = "Tools"
        self.addParameter(ParameterFile(lasheight.INPUT, "Input las layer"))
        self.addOutput(OutputFile(lasheight.OUTPUT, "Output height las file"))
        self.addCommonParameters()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LasToolsUtils.LasToolsPath(), "bin", "lasheight.exe")]
        commands.append("-i")
        commands.append(self.getParameterValue(lasheight.INPUT))
        commands.append("-o")
        commands.append(self.getOutputValue(lasheight.OUTPUT))
        self.addCommonParameterValuesToCommand(commands)

        LasToolsUtils.runLasTools(commands, progress)
