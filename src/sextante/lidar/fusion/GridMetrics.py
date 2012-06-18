import os
from sextante.parameters.ParameterFile import ParameterFile
from sextante.outputs.OutputTable import OutputTable
from sextante.lidar.fusion.FusionUtils import FusionUtils
from sextante.lidar.fusion.FusionAlgorithm import FusionAlgorithm
from sextante.parameters.ParameterNumber import ParameterNumber

class GridMetrics(FusionAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    GROUND = "GROUND"
    HEIGHT = "HEIGHT"
    CELLSIZE = "CELLSIZE"

    def defineCharacteristics(self):
        self.name = "Grid Metrics"
        self.group = "Points"
        self.addParameter(ParameterFile(self.INPUT, "Input las layer"))
        self.addParameter(ParameterFile(self.GROUND, "Input ground DTM layer"))
        self.addParameter(ParameterNumber(self.HEIGHT, "Height break"))
        self.addParameter(ParameterNumber(self.CELLSIZE, "Cellsize"))
        self.addOutput(OutputTable(self.OUTPUT, "Output table with grid metrics"))
        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), "GridMetrics.exe")]
        commands.append("/verbose")
        self.addAdvancedModifiersToCommand(commands)
        commands.append(self.getParameterValue(self.GROUND))
        commands.append(str(self.getParameterValue(self.HEIGHT)))
        commands.append(str(self.getParameterValue(self.CELLSIZE)))
        commands.append(self.getOutputValue(self.OUTPUT))
        files = self.getParameterValue(self.INPUT).split(";")
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())

        FusionUtils.runFusion(commands, progress)
