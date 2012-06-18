import os
from sextante.parameters.ParameterFile import ParameterFile
from sextante.lidar.fusion.FusionUtils import FusionUtils
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.lidar.fusion.FusionAlgorithm import FusionAlgorithm
from sextante.outputs.OutputFile import OutputFile

class GridSurfaceCreate(FusionAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    CELLSIZE = "CELLSIZE"
    XYUNITS = "XYUNITS"
    ZUNITS = "ZUNITS"
    UNITS = ["Meter", "Feet"]

    def defineCharacteristics(self):
        self.name = "Create Grid Surface"
        self.group = "Surface"
        self.addParameter(ParameterFile(self.INPUT, "Input las layer"))
        self.addParameter(ParameterNumber(self.CELLSIZE, "Cellsize", 0, None, 10.0))
        self.addParameter(ParameterSelection(self.XYUNITS, "XY Units", self.UNITS))
        self.addParameter(ParameterSelection(self.ZUNITS, "Z Units", self.UNITS))
        self.addOutput(OutputFile(self.OUTPUT, "PLANS DTM surface"))
        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), "GridSurfaceCreate.exe")]
        commands.append("/verbose")
        self.addAdvancedModifiersToCommand(commands)
        commands.append(self.getOutputValue(self.OUTPUT))
        commands.append(str(self.getParameterValue(self.CELLSIZE)))
        commands.append(self.UNITS[self.getParameterValue(self.XYUNITS)][0])
        commands.append(self.UNITS[self.getParameterValue(self.ZUNITS)][0])
        commands.append("0")
        commands.append("0")
        commands.append("0")
        commands.append("0")
        files = self.getParameterValue(self.INPUT).split(";")
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        FusionUtils.runFusion(commands, progress)
