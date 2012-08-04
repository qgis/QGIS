import os
from PyQt4 import QtGui
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.parameters.ParameterString import ParameterString
from sextante.outputs.OutputVector import OutputVector
from sextante.lidar.lastools.LasToolsUtils import LasToolsUtils
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.lidar.lastools.LasToolsAlgorithm import LasToolsAlgorithm
from sextante.parameters.ParameterFile import ParameterFile

class lasboundary(LasToolsAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    CONCAVITY = "CONCAVITY"
    DISJOINT = "DISJOINT"
    HOLES = "HOLES"

    def defineCharacteristics(self):
        self.name = "lasboundary"
        self.group = "Tools"
        self.addParameter(ParameterFile(lasboundary.INPUT, "Input las layer"))
        self.addParameter(ParameterNumber(lasboundary.CONCAVITY, "Concavity threshold", 0, None, 50.0))
        self.addParameter(ParameterBoolean(lasboundary.HOLES, "Compute also interior holes", False))
        self.addParameter(ParameterBoolean(lasboundary.DISJOINT, "Compute disjoint hull", False))
        self.addOutput(OutputVector(lasboundary.OUTPUT, "Output boundary layer"))
        self.addCommonParameters()

    def processAlgorithm(self, progress):
        commands = [os.path.join(LasToolsUtils.LasToolsPath(), "bin", "lasboundary.exe")]
        commands.append("-i")
        commands.append(self.getParameterValue(lasboundary.INPUT))
        commands.append("-o")
        commands.append(self.getOutputValue(lasboundary.OUTPUT))
        commands.append("-concavity")
        commands.append(str(self.getParameterValue(lasboundary.CONCAVITY)))
        if self.getParameterValue(lasboundary.HOLES):
            commands.append("-holes")
        if self.getParameterValue(lasboundary.DISJOINT):
            commands.append("-disjoint")
        self.addCommonParameterValuesToCommand(commands)

        LasToolsUtils.runLasTools(commands, progress)
