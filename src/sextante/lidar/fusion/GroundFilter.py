import os
from sextante.parameters.ParameterFile import ParameterFile
from sextante.lidar.fusion.FusionUtils import FusionUtils
import subprocess
from sextante.outputs.OutputFile import OutputFile
from sextante.lidar.fusion.FusionAlgorithm import FusionAlgorithm
from sextante.parameters.ParameterNumber import ParameterNumber

class GroundFilter(FusionAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    CELLSIZE = "CELLSIZE"


    def defineCharacteristics(self):
        self.name = "Ground Filter"
        self.group = "Points"
        self.addParameter(ParameterFile(self.INPUT, "Input las layer"))
        self.addParameter(ParameterNumber(self.CELLSIZE, "Cellsize for intermediate surfaces", 0, None, 10))
        self.addOutput(OutputFile(self.OUTPUT, "Output ground las file"))
        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), "GroundFilter.exe")]
        commands.append("/verbose")
        self.addAdvancedModifiersToCommand(commands)
        outFile = self.getOutputValue(self.OUTPUT) + ".lda"
        commands.append(str(self.getParameterValue(self.CELLSIZE)))
        commands.append(outFile)
        files = self.getParameterValue(self.INPUT).split(";")
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        FusionUtils.runFusion(commands, progress)
        commands = [os.path.join(FusionUtils.FusionPath(), "LDA2LAS.exe")]
        commands.append(outFile)
        commands.append(self.getOutputValue(self.OUTPUT))
        p = subprocess.Popen(commands, shell=True)
        p.wait()
