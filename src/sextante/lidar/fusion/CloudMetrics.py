import os
from sextante.parameters.ParameterFile import ParameterFile
from sextante.outputs.OutputTable import OutputTable
from sextante.lidar.fusion.FusionUtils import FusionUtils
from sextante.lidar.fusion.FusionAlgorithm import FusionAlgorithm

class CloudMetrics(FusionAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def defineCharacteristics(self):
        self.name = "Cloud Metrics"
        self.group = "Points"
        self.addParameter(ParameterFile(self.INPUT, "Input las layer"))
        self.addOutput(OutputTable(self.OUTPUT, "Output file with tabular metric information"))
        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), "CloudMetrics.exe")]
        commands.append("/verbose")
        self.addAdvancedModifiersToCommand(commands)
        files = self.getParameterValue(self.INPUT).split(";")
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        commands.append(self.getOutputValue(self.OUTPUT))
        #self.addCommonParameterValuesToCommand(commands)

        FusionUtils.runFusion(commands, progress)
