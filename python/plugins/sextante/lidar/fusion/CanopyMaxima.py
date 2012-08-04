import os
from sextante.parameters.ParameterFile import ParameterFile
from sextante.outputs.OutputTable import OutputTable
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.lidar.fusion.FusionUtils import FusionUtils
from PyQt4 import QtGui
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.lidar.fusion.FusionAlgorithm import FusionAlgorithm

class CanopyMaxima(FusionAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    THRESHOLD = "THRESHOLD"
    GROUND = "GROUND"


    def defineCharacteristics(self):
        self.name = "Canopy Maxima"
        self.group = "Points"
        self.addParameter(ParameterFile(self.INPUT, "Input las layer"))
        self.addParameter(ParameterFile(self.GROUND, "Input ground DTM layer [optional, leave blank if not using it]"))
        self.addParameter(ParameterNumber(self.THRESHOLD, "Minimum threshold", 0, None, 10.0))
        self.addOutput(OutputTable(self.OUTPUT, "Output file with maxima"))
        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), "CanopyMaxima.exe")]
        commands.append("/verbose")
        self.addAdvancedModifiersToCommand(commands)
        ground = self.getParameterValue(self.GROUND)
        if str(ground).strip() != "":
            commands.append("/ground:" + str(ground))
        commands.append("/threshold:" + str(self.getParameterValue(self.THRESHOLD)))
        files = self.getParameterValue(self.INPUT).split(";")
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        commands.append(self.getOutputValue(self.OUTPUT))

        FusionUtils.runFusion(commands, progress)
