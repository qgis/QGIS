import os
from sextante.parameters.ParameterFile import ParameterFile
from sextante.lidar.fusion.FusionUtils import FusionUtils
from sextante.outputs.OutputFile import OutputFile
from sextante.lidar.fusion.FusionAlgorithm import FusionAlgorithm

class MergeData(FusionAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"


    def defineCharacteristics(self):
        self.name = "Merge LAS Files"
        self.group = "Points"
        self.addParameter(ParameterFile(self.INPUT, "Input LAS files"))
        self.addOutput(OutputFile(self.OUTPUT, "Output merged LAS file"))

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), "MergeData.exe")]
        commands.append("/verbose")
        self.addAdvancedModifiersToCommand(commands)
        files = self.getParameterValue(self.INPUT).split(";")
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        outFile = self.getOutputValue(self.OUTPUT)
        commands.append(outFile)
        FusionUtils.runFusion(commands, progress)
