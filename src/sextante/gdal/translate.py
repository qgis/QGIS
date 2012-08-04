from PyQt4 import QtGui
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.outputs.OutputRaster import OutputRaster
import os
from sextante.gdal.GdalUtils import GdalUtils

class translate(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def getIcon(self):
        filepath = os.path.dirname(__file__) + "/icons/translate.png"
        return QtGui.QIcon(filepath)

    def defineCharacteristics(self):
        self.name = "translate"
        self.group = "Conversion"
        self.addParameter(ParameterRaster(translate.INPUT, "Input layer", False))
        self.addOutput(OutputRaster(translate.OUTPUT, "Output layer"))

    def processAlgorithm(self, progress):
        commands = ["gdal_translate"]
        commands.append("-of")
        out = self.getOutputValue(translate.OUTPUT)
        commands.append(GdalUtils.getFormatShortNameFromFilename(out))
        commands.append(self.getParameterValue(translate.INPUT))
        commands.append(out)

        GdalUtils.runGdal(commands, progress)
