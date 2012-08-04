import os
from sextante.lidar.lastools.LasToolsUtils import LasToolsUtils
from sextante.lidar.lastools.LasToolsAlgorithm import LasToolsAlgorithm
from sextante.parameters.ParameterFile import ParameterFile
from sextante.outputs.OutputFile import OutputFile

class lasclassify(LasToolsAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def defineCharacteristics(self):
        self.name = "lasclassify"
        self.group = "Tools"
        self.addParameter(ParameterFile(lasclassify.INPUT, "Input las layer"))
        self.addOutput(OutputFile(lasclassify.OUTPUT, "Output classified las file"))
        self.addCommonParameters()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LasToolsUtils.LasToolsPath(), "bin", "lasclassify.exe")]
        commands.append("-i")
        commands.append(self.getParameterValue(lasclassify.INPUT))
        commands.append("-o")
        commands.append(self.getOutputValue(lasclassify.OUTPUT))
        self.addCommonParameterValuesToCommand(commands)

        LasToolsUtils.runLasTools(commands, progress)
