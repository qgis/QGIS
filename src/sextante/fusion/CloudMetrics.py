import os
from sextante.parameters.ParameterFile import ParameterFile
from sextante.outputs.OutputTable import OutputTable
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.fusion.FusionUtils import FusionUtils
from PyQt4 import QtGui

class CloudMetrics(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/tool.png")

    def defineCharacteristics(self):
        self.name = "Cloud Metrics"
        self.group = "Tools"
        self.addParameter(ParameterFile(self.INPUT, "Input las layer"))
        self.addOutput(OutputTable(self.OUTPUT, "Output file with tabular metric information"))
        #self.addCommonParameters()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), "CloudMetrics.exe")]
        files = self.getParameterValue(self.INPUT).split(";")
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        commands.append(self.getOutputValue(self.OUTPUT))
        #self.addCommonParameterValuesToCommand(commands)

        FusionUtils.runFusion(commands, progress)
