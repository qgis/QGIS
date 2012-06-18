import os
from sextante.parameters.ParameterFile import ParameterFile
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.lidar.fusion.FusionUtils import FusionUtils
from PyQt4 import QtGui
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.outputs.OutputRaster import OutputRaster
from sextante.parameters.ParameterSelection import ParameterSelection
import subprocess
from sextante.lidar.fusion.FusionAlgorithm import FusionAlgorithm

class Cover(FusionAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    CELLSIZE = "CELLSIZE"
    HEIGHTBREAK = "HEIGHTREAK"
    GROUND = "GROUND"
    XYUNITS = "XYUNITS"
    ZUNITS = "ZUNITS"
    UNITS = ["Meter", "Feet"]

    def defineCharacteristics(self):
        self.name = "Cover"
        self.group = "Points"
        self.addParameter(ParameterFile(self.INPUT, "Input las layer"))
        self.addParameter(ParameterFile(self.GROUND, "Input ground DTM layer"))
        self.addParameter(ParameterNumber(self.CELLSIZE, "Cellsize", 0, None, 10.0))
        self.addParameter(ParameterNumber(self.HEIGHTBREAK, "Heightbreak", 0, None, 10.0))
        self.addParameter(ParameterSelection(self.XYUNITS, "XY Units", self.UNITS))
        self.addParameter(ParameterSelection(self.ZUNITS, "Z Units", self.UNITS))
        self.addOutput(OutputRaster(self.OUTPUT, "Cover"))
        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), "Cover.exe")]
        commands.append("/verbose")
        self.addAdvancedModifiersToCommand(commands)
        ground = self.getParameterValue(self.GROUND)
        if str(ground).strip() != "":
            commands.append("/ground:" + str(ground))
        outFile = self.getOutputValue(self.OUTPUT)+".dtm"
        commands.append(outFile)
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
        commands = [os.path.join(FusionUtils.FusionPath(), "DTM2TIF.exe")]
        commands.append(outFile)
        commands.append(self.getOutputValue(self.OUTPUT))
        p = subprocess.Popen(commands, shell=True)
        p.wait()
