import os
from sextante.parameters.ParameterMultipleInput import ParameterMultipleInput
from sextante.grass.GrassUtils import GrassUtils
from sextante.core.GeoAlgorithm import GeoAlgorithm
from PyQt4 import QtGui
from sextante.core.SextanteUtils import SextanteUtils
import time

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
            layers = vector.split(";")
            for layer in layers:
                newfilename = self.exportVectorLayer(layer)
                vector = vector.replace(layer, newfilename)
            command += (" vector=" + vector.replace(";", ","))
        if elevation:
            layers = elevation.split(";")
            for layer in layers:
                newfilename = self.exportRasterLayer(layer)
                elevation = elevation.replace(layer, newfilename)
            command += (" elevation=" + elevation.replace(";", ","))
        if elevation is None and vector is None:
            command += " -q"
        commands.append(command)
        GrassUtils.createTempMapset();
        GrassUtils.executeGrass(commands, progress)

    def getTempFilename(self):
        filename =  "tmp" + str(time.time()).replace(".","") + str(SextanteUtils.getNumExportedLayers())
        return filename

    def exportVectorLayer(self,layer):
        destFilename = self.getTempFilename()
        command = "v.in.ogr"
        command += " min_area=-1"
        command +=" dsn=\"" + os.path.dirname(layer) + "\""
        command +=" layer=" + os.path.basename(layer)[:-4]
        command +=" output=" + destFilename;
        command +=" --overwrite -o"
        return destFilename


    def exportRasterLayer(self, layer):
        destFilename = self.getTempFilename()
        command = "r.in.gdal"
        command +=" input=\"" + layer + "\""
        command +=" band=1"
        command +=" out=" + destFilename;
        command +=" --overwrite -o"
        return destFilename

