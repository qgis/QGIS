import os
from sextante.parameters.ParameterMultipleInput import ParameterMultipleInput
from sextante.grass.GrassUtils import GrassUtils
from sextante.core.GeoAlgorithm import GeoAlgorithm
from PyQt4 import QtGui

class nviz(GeoAlgorithm):

    ELEVATION = "ELEVATION"
    VECTOR = "VECTOR"

    def getIcon(self):
        return  QtGui.QIcon(os.path.dirname(__file__) + "/../images/grass.png")

    def defineCharacteristics(self):
        self.name = "nviz"
        self.group = "Visualization(NVIZ)"
        self.addParameter(ParameterMultipleInput(nviz.ELEVATION, "Elevation layers", ParameterMultipleInput.TYPE_RASTER, True))
        self.addParameter(ParameterMultipleInput(nviz.VECTOR, "Vector layers", ParameterMultipleInput.TYPE_VECTOR_ANY, True))

    def processAlgorithm(self, progress):
        commands = []
        command = "nviz"
        vector = self.getParameterValue(self.VECTOR);
        elevation = self.getParameterValue(self.ELEVATION);
        if vector:
            command += (" vector=" + vector.replace(";", ","))
        if elevation:
            command += (" elevation=" + elevation.replace(";", ","))
        if elevation is None and vector is None:
            command += " -q"
        commands.append(command)
        GrassUtils.createTempMapset();
        GrassUtils.executeGrass(commands, progress)
