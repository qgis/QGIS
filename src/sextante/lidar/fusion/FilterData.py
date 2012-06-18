import os
from sextante.parameters.ParameterFile import ParameterFile
from sextante.lidar.fusion.FusionUtils import FusionUtils
import subprocess
from sextante.outputs.OutputFile import OutputFile
from sextante.lidar.fusion.FusionAlgorithm import FusionAlgorithm
from sextante.parameters.ParameterNumber import ParameterNumber

class FilterData(FusionAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    VALUE = "VALUE"
    SHAPE = "SHAPE"
    WINDOWSIZE = "WINDOWSIZE"


    def defineCharacteristics(self):
        self.name = "Filter Data outliers"
        self.group = "Points"
        self.addParameter(ParameterFile(self.INPUT, "Input las layer"))
        self.addParameter(ParameterNumber(self.VALUE, "Standard Deviation multiplier"))
        self.addParameter(ParameterNumber(self.VALUE, "Window size", None, None, 10))
        self.addOutput(OutputFile(self.OUTPUT, "Output filtered las file"))
        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), "FilterData.exe")]
        commands.append("/verbose")
        self.addAdvancedModifiersToCommand(commands)
        commands.append("outlier")
        commands.append(str(self.getParameterValue(self.VALUE)))
        commands.append(str(self.getParameterValue(self.WINDOWSIZE)))
        outFile = self.getOutputValue(self.OUTPUT) + ".lda"
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
