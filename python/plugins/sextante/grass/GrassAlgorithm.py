import os
from qgis.core import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.parameters.ParameterTable import ParameterTable
from sextante.parameters.ParameterMultipleInput import ParameterMultipleInput
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.outputs.OutputRaster import OutputRaster
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.outputs.OutputVector import OutputVector
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.core.SextanteLog import SextanteLog
from sextante.parameters.ParameterFactory import ParameterFactory
from sextante.outputs.OutputFactory import OutputFactory
from sextante.core.SextanteConfig import SextanteConfig
from sextante.core.QGisLayers import QGisLayers
from sextante.grass.GrassUtils import GrassUtils
import time
from sextante.core.SextanteUtils import SextanteUtils
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.core.LayerExporter import LayerExporter
from sextante.core.WrongHelpFileException import WrongHelpFileException
from sextante.outputs.OutputFile import OutputFile
from sextante.parameters.ParameterExtent import ParameterExtent
from sextante.parameters.ParameterNumber import ParameterNumber

class GrassAlgorithm(GeoAlgorithm):

    GRASS_REGION_EXTENT_PARAMETER = "GRASS_REGION_PARAMETER"
    GRASS_REGION_CELLSIZE_PARAMETER = "GRASS_REGION_CELLSIZE_PARAMETER"

    def __init__(self, descriptionfile):
        GeoAlgorithm.__init__(self)
        self.descriptionFile = descriptionfile
        self.defineCharacteristicsFromFile()
        self.numExportedLayers = 0

    def getCopy(self):
        newone = GrassAlgorithm(self.descriptionFile)
        newone.provider = self.provider
        return newone

    def getIcon(self):
        return  QIcon(os.path.dirname(__file__) + "/../images/grass.png")

    def helpFile(self):
        folder = GrassUtils.grassHelpPath()
        helpfile = str(folder) + os.sep + self.grassName + ".html"
        if os.path.exists(helpfile):
            return helpfile
        else:
            raise WrongHelpFileException("Grass help folder is not correctly configured.\nPlease configure it")
        #return None

    def getParameterDescriptions(self):
        descs = {}
        try:
            helpfile = self.helpFile()
        except WrongHelpFileException:
            return descs
        if helpfile:
            try:
                infile = open(helpfile)
                lines = infile.readlines()
                for i in range(len(lines)):
                    if lines[i].startswith("<DT><b>"):
                        for param in self. parameters:
                            searchLine = "<b>" + param.name + "</b>"
                            if searchLine in lines[i]:
                                i+=1
                                descs[param.name] = lines[i][4:-6]
                                break

                infile.close()
            except Exception:
                pass
        return descs

    def defineCharacteristicsFromFile(self):
        lines = open(self.descriptionFile)
        line = lines.readline().strip("\n").strip()
        self.grassName = line
        line = lines.readline().strip("\n").strip()
        self.name = line
        line = lines.readline().strip("\n").strip()
        self.group = line
        while line != "":
            try:
                line = line.strip("\n").strip()
                if line.startswith("Parameter"):
                    self.addParameter(ParameterFactory.getFromString(line))
                elif line.startswith("*Parameter"):
                    param = ParameterFactory.getFromString(line[1:])
                    param.isAdvanced = True
                    self.addParameter(param)
                else:
                    self.addOutput(OutputFactory.getFromString(line))
                line = lines.readline().strip("\n").strip()
            except Exception,e:
                SextanteLog.addToLog(SextanteLog.LOG_ERROR, "Could not open GRASS algorithm: " + self.descriptionFile + "\n" + line)
                raise e
        lines.close()

        #=======================================================================
        # self.xmin = SextanteConfig.getSetting(GrassUtils.GRASS_REGION_XMIN)
        # self.xmax = SextanteConfig.getSetting(GrassUtils.GRASS_REGION_XMAX)
        # self.ymin = SextanteConfig.getSetting(GrassUtils.GRASS_REGION_YMIN)
        # self.ymax = SextanteConfig.getSetting(GrassUtils.GRASS_REGION_YMAX)
        # extentString = str(self.xmin) + "," + str(self.xmax) + str(self.xmin) + "," + str(self.xmax)
        # self.cellsize = SextanteConfig.getSetting(GrassUtils.GRASS_REGION_CELLSIZE)
        #=======================================================================
        self.addParameter(ParameterExtent(self.GRASS_REGION_EXTENT_PARAMETER, "GRASS region extent"))
        self.addParameter(ParameterNumber(self.GRASS_REGION_CELLSIZE_PARAMETER, "GRASS region cellsize", 0, None, 1))

#===============================================================================
#    def calculateRegion(self):
#        auto = SextanteConfig.getSetting(GrassUtils.GRASS_AUTO_REGION)
#        if auto:
#            try:
#                self.cellsize = SextanteConfig.getSetting(GrassUtils.GRASS_REGION_CELLSIZE)
#            except Exception:
#                self.cellsize = 0;
#            first = True;
#            for param in self.parameters:
#                if param.value:
#                    if isinstance(param, (ParameterRaster, ParameterVector)):
#                        if isinstance(param.value, (QgsRasterLayer, QgsVectorLayer)):
#                            layer = param.value
#                        else:
#                            layer = QGisLayers.getObjectFromUri(param.value)
#                        self.addToRegion(layer, first)
#                        first = False
#                    elif isinstance(param, ParameterMultipleInput):
#                        layers = param.value.split(";")
#                        for layername in layers:
#                            layer = QGisLayers.getObjectFromUri(layername, first)
#                            self.addToRegion(layer, first)
#                            first = False
#            if self.cellsize == 0:
#                self.cellsize = 1
#        else:
#            self.xmin = SextanteConfig.getSetting(GrassUtils.GRASS_REGION_XMIN)
#            self.xmax = SextanteConfig.getSetting(GrassUtils.GRASS_REGION_XMAX)
#            self.ymin = SextanteConfig.getSetting(GrassUtils.GRASS_REGION_YMIN)
#            self.ymax = SextanteConfig.getSetting(GrassUtils.GRASS_REGION_YMAX)
#            self.cellsize = SextanteConfig.getSetting(GrassUtils.GRASS_REGION_CELLSIZE)
#
#
#    def addToRegion(self, layer, first):
#        if first:
#            self.xmin = layer.extent().xMinimum()
#            self.xmax = layer.extent().xMaximum()
#            self.ymin = layer.extent().yMinimum()
#            self.ymax = layer.extent().yMaximum()
#            if isinstance(layer, QgsRasterLayer):
#                self.cellsize = (layer.extent().xMaximum() - layer.extent().xMinimum())/layer.width()
#        else:
#            self.xmin = min(self.xmin, layer.extent().xMinimum())
#            self.xmax = max(self.xmax, layer.extent().xMaximum())
#            self.ymin = min(self.ymin, layer.extent().yMinimum())
#            self.ymax = max(self.ymax, layer.extent().yMaximum())
#            if isinstance(layer, QgsRasterLayer):
#                self.cellsize = max(self.cellsize, (layer.extent().xMaximum() - layer.extent().xMinimum())/layer.width())
#===============================================================================

    def processAlgorithm(self, progress):
        if SextanteUtils.isWindows():
            path = GrassUtils.grassPath()
            if path == "":
                raise GeoAlgorithmExecutionException("GRASS folder is not configured.\nPlease configure it before running GRASS algorithms.")

        commands = []
        self.exportedLayers = {}

        #self.calculateRegion()
        region = str(self.getParameterValue(self.GRASS_REGION_EXTENT_PARAMETER))
        regionCoords = region.split(",")
        GrassUtils.createTempMapset();

        command = "g.region"
        command += " n=" + str(regionCoords[3])
        command +=" s=" + str(regionCoords[2])
        command +=" e=" + str(regionCoords[1])
        command +=" w=" + str(regionCoords[0])
        command +=" res=" + str(self.getParameterValue(self.GRASS_REGION_CELLSIZE_PARAMETER));
        commands.append(command)

        #1: Export layer to grass mapset
        for param in self.parameters:
            if isinstance(param, ParameterRaster):
                if param.value == None:
                    continue
                value = param.value
                commands.append(self.exportRasterLayer(value))
            if isinstance(param, ParameterVector):
                if param.value == None:
                    continue
                value = param.value
                commands.append(self.exportVectorLayer(value))
            if isinstance(param, ParameterTable):
                pass
            if isinstance(param, ParameterMultipleInput):
                if param.value == None:
                    continue
                layers = param.value.split(";")
                if layers == None or len(layers) == 0:
                    continue
                if param.datatype == ParameterMultipleInput.TYPE_RASTER:
                    for layer in layers:
                        commands.append(self.exportRasterLayer(layer))
                elif param.datatype == ParameterMultipleInput.TYPE_VECTOR_ANY:
                    for layer in layers:
                        commands.append(self.exportVectorLayer(layer))

        #2: set parameters and outputs
        command = self.grassName
        for param in self.parameters:
            if param.value == None:
                continue
            if param.name == self.GRASS_REGION_CELLSIZE_PARAMETER or param.name == self.GRASS_REGION_EXTENT_PARAMETER:
                continue
            if isinstance(param, (ParameterRaster, ParameterVector)):
                value = param.value
                if value in self.exportedLayers.keys():
                    command+=(" " + param.name + "=" + self.exportedLayers[value])
                else:
                    command+=(" " + param.name + "=" + value)
            elif isinstance(param, ParameterMultipleInput):
                s = param.value
                for layer in self.exportedLayers.keys():
                    s = s.replace(layer, self.exportedLayers[layer])
                s = s.replace(";",",")
                command+=(" " + param.name + "=" + s);
            elif isinstance(param, ParameterBoolean):
                if param.value:
                    command += (" " + param.name)
            elif isinstance(param, ParameterSelection):
                idx = int(param.value)
                command+=(" " + param.name + "=" + str(param.options[idx]));
            else:
                command+=(" " + param.name + "=" + str(param.value));

        for out in self.outputs:
            if isinstance(out, OutputFile):
                command+=(" " + out.name + "=\"" + out.value + "\"");
            else:
                command+=(" " + out.name + "=" + out.name);

        command += " --overwrite"
        commands.append(command)

        #3:Export resulting layers to a format that qgis can read
        for out in self.outputs:
            if isinstance(out, OutputRaster):
                filename = out.value
                #Raster layer output: adjust region to layer before exporting
                commands.append("g.region rast=" + out.name)
                command = "r.out.gdal -c createopt=\"TFW=YES,COMPRESS=LZW\""
                command += " input="
                command += out.name
                command += " output=\"" + filename + "\""
                commands.append(command)
            if isinstance(out, OutputVector):
                command = "v.out.ogr -ce input=" + out.name
                command += " dsn=\"" + os.path.dirname(out.value) + "\""
                command += " format=ESRI_Shapefile"
                command += " olayer=" + os.path.basename(out.value)[:-4]
                command += " type=auto"
                commands.append(command)

        #4 Run GRASS
        loglines = []
        loglines.append("GRASS execution commands")
        for line in commands:
            progress.setCommand(line)
            loglines.append(line)
        if SextanteConfig.getSetting(GrassUtils.GRASS_LOG_COMMANDS):
            SextanteLog.addToLog(SextanteLog.LOG_INFO, loglines)
        GrassUtils.executeGrass(commands, progress);


    def exportVectorLayer(self, orgFilename):
        #only export to an intermediate shp if the layer is not file-based.
        #We assume that almost all file formats will be supported by ogr
        #We also export if there is a selection
        if not os.path.exists(orgFilename):
            layer = QGisLayers.getObjectFromUri(orgFilename, False)
            if layer:
                filename = LayerExporter.exportVectorLayer(layer)
        else:
            layer = QGisLayers.getObjectFromUri(orgFilename, False)
            if layer:
                useSelection = SextanteConfig.getSetting(SextanteConfig.USE_SELECTED)
                if useSelection and layer.selectedFeatureCount() != 0:
                    filename = LayerExporter.exportVectorLayer(layer)
                else:
                    filename = orgFilename
            else:
                filename = orgFilename
        destFilename = self.getTempFilename()
        self.exportedLayers[orgFilename]= destFilename
        command = "v.in.ogr"
        command += " min_area=-1"
        command +=" dsn=\"" + os.path.dirname(filename) + "\""
        command +=" layer=" + os.path.basename(filename)[:-4]
        command +=" output=" + destFilename;
        command +=" --overwrite -o"
        return command


    def exportRasterLayer(self, layer):
        destFilename = self.getTempFilename()
        self.exportedLayers[layer]= destFilename
        command = "r.in.gdal"
        command +=" input=\"" + layer + "\""
        command +=" band=1"
        command +=" out=" + destFilename;
        command +=" --overwrite -o"
        return command


    def getTempFilename(self):
        filename =  "tmp" + str(time.time()).replace(".","") + str(SextanteUtils.getNumExportedLayers())
        return filename

    def commandLineName(self):
        return "grass:" + self.name[:self.name.find(" ")]

#===============================================================================
#    def checkBeforeOpeningParametersDialog(self):
#        for param in self.parameters:
#            if isinstance(param, (ParameterRaster, ParameterVector)):
#                return None
#            if isinstance(param, ParameterMultipleInput):
#                if not param.optional:
#                    return None
#
#        if SextanteConfig.getSetting(GrassUtils.GRASS_AUTO_REGION):
#            return "This algorithm cannot be run with the 'auto-region' setting\nPlease set a GRASS region before running it"
#        else:
#            return None
#===============================================================================
