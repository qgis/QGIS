import os
from sextante.lidar.lastools.LasToolsUtils import LasToolsUtils
from sextante.lidar.lastools.LasToolsAlgorithm import LasToolsAlgorithm
from sextante.parameters.ParameterFile import ParameterFile
from sextante.outputs.OutputHTML import OutputHTML

class lasinfo(LasToolsAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def defineCharacteristics(self):
        self.name = "lasinfo"
        self.group = "Tools"
        self.addParameter(ParameterFile(lasinfo.INPUT, "Input las layer"))
        self.addOutput(OutputHTML(lasinfo.OUTPUT, "Output info file"))

    def processAlgorithm(self, progress):
        commands = [os.path.join(LasToolsUtils.LasToolsPath(), "bin", "lasinfo.exe")]
        commands.append("-i")
        commands.append(self.getParameterValue(lasinfo.INPUT))
        commands.append("-o")
        commands.append(self.getOutputValue(lasinfo.OUTPUT) + ".txt")

        LasToolsUtils.runLasTools(commands, progress)
        fin = open (self.getOutputValue(lasinfo.OUTPUT) + ".txt")
        fout = open (self.getOutputValue(lasinfo.OUTPUT), "w")
        lines = fin.readlines()
        for line in lines:
            fout.write(line + "<br>")
        fin.close()
        fout.close()

