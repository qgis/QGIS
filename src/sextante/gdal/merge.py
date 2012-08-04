from PyQt4 import QtGui
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.outputs.OutputRaster import OutputRaster
import os
from sextante.gdal.GdalUtils import GdalUtils
from sextante.core.SextanteUtils import SextanteUtils
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.parameters.ParameterMultipleInput import ParameterMultipleInput

class merge(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    PCT = "PCT"
    SEPARATE = "SEPARATE"

    def getIcon(self):
        filepath = os.path.dirname(__file__) + "/icons/merge.png"
        return QtGui.QIcon(filepath)

    def defineCharacteristics(self):
        self.name = "merge"
        self.group = "Miscellaneous"
        self.addParameter(ParameterMultipleInput(merge.INPUT, "Input layers", ParameterMultipleInput.TYPE_RASTER))
        self.addParameter(ParameterBoolean(merge.PCT, "Grab pseudocolor table from first layer", False))
        self.addParameter(ParameterBoolean(merge.SEPARATE, "Layer stack", False))
        self.addOutput(OutputRaster(merge.OUTPUT, "Output layer"))

    def processAlgorithm(self, progress):
        if SextanteUtils.isWindows():
            commands = ["cmd.exe", "/C ", "gdal_merge.bat"]
        else:
            commands = ["gdal_merge.py"]
        if self.getParameterValue(merge.SEPARATE):
            commands.append("-separate")
        if self.getParameterValue(merge.PCT):
            commands.append("-pct")
        commands.append("-of")
        out = self.getOutputValue(merge.OUTPUT)
        commands.append(GdalUtils.getFormatShortNameFromFilename(out))
        commands.append(self.getParameterValue(merge.INPUT).replace(";", " "))
        commands.append(out)

        GdalUtils.runGdal(commands, progress)
