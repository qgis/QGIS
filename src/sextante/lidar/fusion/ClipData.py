import os
from sextante.parameters.ParameterFile import ParameterFile
from sextante.lidar.fusion.FusionUtils import FusionUtils
from PyQt4 import QtGui
import subprocess
from sextante.parameters.ParameterExtent import ParameterExtent
from sextante.outputs.OutputFile import OutputFile
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.lidar.fusion.FusionAlgorithm import FusionAlgorithm

class ClipData(FusionAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    EXTENT = "EXTENT"
    SHAPE = "SHAPE"


    def defineCharacteristics(self):
        self.name = "Clip Data"
        self.group = "Points"
        self.addParameter(ParameterFile(self.INPUT, "Input las layer"))
        self.addParameter(ParameterExtent(self.EXTENT, "Extent"))
        self.addParameter(ParameterSelection(self.SHAPE, "Shape", ["Rectangle","Circle"]))
        self.addOutput(OutputFile(self.OUTPUT, "Output clipped las file"))
        self.addAdvancedModifiers()

    def processAlgorithm(self, progress):
        commands = [os.path.join(FusionUtils.FusionPath(), "FilterData.exe")]
        commands.append("/verbose")
        self.addAdvancedModifiersToCommand(commands)
        commands.append("/shape:" + str(self.getParameterValue(self.SHAPE)))
        files = self.getParameterValue(self.INPUT).split(";")
        if len(files) == 1:
            commands.append(self.getParameterValue(self.INPUT))
        else:
            FusionUtils.createFileList(files)
            commands.append(FusionUtils.tempFileListFilepath())
        outFile = self.getOutputValue(self.OUTPUT) + ".lda"
        commands.append(outFile)
        extent = str(self.getParameterValue(self.EXTENT)).split(",")
        commands.append(extent[0])
        commands.append(extent[2])
        commands.append(extent[1])
        commands.append(extent[3])
        FusionUtils.runFusion(commands, progress)
        commands = [os.path.join(FusionUtils.FusionPath(), "LDA2LAS.exe")]
        commands.append(outFile)
        commands.append(self.getOutputValue(self.OUTPUT))
        p = subprocess.Popen(commands, shell=True)
        p.wait()
