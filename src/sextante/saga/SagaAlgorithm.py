import os
from qgis.core import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.saga.SagaBlackList import SagaBlackList
from sextante.saga.UnwrappableSagaAlgorithmException import UnwrappableSagaAlgorithmException
from sextante.parameters.ParameterTable import ParameterTable
from sextante.parameters.ParameterFixedTable import ParameterFixedTable
from sextante.parameters.ParameterTableField import ParameterTableField
from sextante.outputs.OutputTable import OutputTable
from sextante.parameters.ParameterMultipleInput import ParameterMultipleInput
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.outputs.OutputRaster import OutputRaster
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.parameters.ParameterString import ParameterString
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.core.SextanteUtils import SextanteUtils

from sextante.outputs.OutputVector import OutputVector
from sextante.saga.SagaUtils import SagaUtils
import time
from sextante.saga.SagaGroupNameDecorator import SagaGroupNameDecorator
from sextante.parameters.ParameterRange import ParameterRange
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.core.SextanteLog import SextanteLog
from sextante.parameters.ParameterFactory import ParameterFactory
from sextante.outputs.OutputFactory import OutputFactory
from sextante.core.SextanteConfig import SextanteConfig
from sextante.core.QGisLayers import QGisLayers
from PyQt4 import QtGui

class SagaAlgorithm(GeoAlgorithm):

    def __init__(self, descriptionfile):
        GeoAlgorithm.__init__(self)
        self._descriptionFile = descriptionfile
        self.defineCharacteristicsFromFile()
        self.numExportedLayers = 0
        self.resample = False #True if the user should define a grid system in
                              #case several non-matching raster layers are used as input

    def getIcon(self):
        return  QIcon(os.path.dirname(__file__) + "/../images/saga.png")

    def defineCharacteristicsFromFile(self):
        lines = open(self._descriptionFile)
        line = lines.readline().strip("\n").strip()
        self.name = line
        line = lines.readline().strip("\n").strip()
        self.undecoratedGroup = line
        self.group = SagaGroupNameDecorator.getDecoratedName(self.undecoratedGroup)
        while line != "":
            line = line.strip("\n").strip()
            if line.startswith("Parameter"):
                self.addParameter(ParameterFactory.getFromString(line))
            elif line.startswith("Resample"):
                self.resample = True
            else:
                self.addOutput(OutputFactory.getFromString(line))
            line = lines.readline().strip("\n").strip()
        lines.close()


    def defineCharacteristicsFromFileSagaFormat(self):
        lines = open(self._descriptionFile)
        line = lines.readline()
        while line != "":
            line = line.strip("\n").strip()
            readLine = True;
            if line.startswith("library name"):
                self.undecoratedGroup = line.split("\t")[1]
                self.group = SagaGroupNameDecorator.getDecoratedName(self.undecoratedGroup)
            if line.startswith("module name"):
                self.name = line.split("\t")[1]
            if line.startswith("-"):
                try:
                    paramName = line[1:line.index(":")]
                    paramDescription = line[line.index(">") + 1:].strip()
                except Exception: #boolean params have a different syntax
                    paramName = line[1:line.index("\t")]
                    paramDescription = line[line.index("\t") + 1:].strip()
                if SagaBlackList.isBlackListed(self.name, self.undecoratedGroup):
                    raise UnwrappableSagaAlgorithmException()
                line = lines.readline().lower()
                if "data object" in line:
                    output = OutputRaster()
                    output.name = paramName
                    output.description = paramDescription
                    self.addOutput(output)
                elif "file" in line:
                    param = ParameterString(paramName, paramDescription)
                    self.addParameter(param)
                elif "table" in line:
                    #print(line)
                    if "input" in line:
                        param = ParameterTable(paramName, paramDescription, ("optional" in line))
                        lastParentParameterName = paramName;
                        self.addParameter(param)
                    elif "static" in line:
                        print (self.name)
                        line = lines.readline().strip("\n").strip()
                        numCols = int(line.split(" ")[0])
                        colNames = [];
                        for i in range(numCols):
                            line = lines.readline().strip("\n").strip()
                            colNames.append(line.split("]")[1])
                        param = ParameterFixedTable(paramName, paramDescription, colNames, 3, False)
                        self.addParameter(param)
                    elif "field" in line:
                        if lastParentParameterName == None:
                            raise UnwrappableSagaAlgorithmException();
                        param = ParameterTableField(paramName, paramDescription, lastParentParameterName)
                        self.addParameter(param)
                    else:
                        output = OutputTable()
                        output.name = paramName
                        output.description = paramDescription
                        self.addOutput(output)
                elif "grid" in line:
                    if "input" in line:
                        if "list" in line:
                            param = ParameterMultipleInput(paramName, paramDescription, ParameterMultipleInput.TYPE_RASTER,("optional" in line))
                            self.addParameter(param)
                        else:
                            param = ParameterRaster(paramName, paramDescription,("optional" in line))
                            self.addParameter(param)
                    else:
                        output = OutputRaster()
                        output.name = paramName
                        output.description = paramDescription
                        self.addOutput(output)
                elif "shapes" in line:
                    if "input" in line:
                        if "list" in line:
                            param = ParameterMultipleInput(paramName, paramDescription, ParameterMultipleInput.TYPE_VECTOR_ANY,("optional" in line))
                            self.addParameter(param)
                        else:
                            param = ParameterVector(paramName, paramDescription,ParameterVector.VECTOR_TYPE_ANY,("optional" in line))
                            lastParentParameterName = paramName;
                            self.addParameter(param)
                    else:
                        output = OutputVector()
                        output.name = paramName
                        output.description = paramDescription
                        self.addOutput(output)
                elif "floating" in line or "integer" in line or "degree" in line:
                    param = ParameterNumber(paramName, paramDescription,)
                    self.addParameter(param)
                elif "boolean" in line:
                    param = ParameterBoolean(paramName,paramDescription)
                    self.addParameter(param)
                elif "text" in line:
                    param = ParameterString(paramName, paramDescription)
                    self.addParameter(param)
                elif "range" in line:
                    param = ParameterRange(paramName, paramDescription)
                    self.addParameter(param)
                elif "choice" in line:
                    line = lines.readline()
                    line = lines.readline().strip("\n").strip()
                    options = list()
                    while line != "" and not (line.startswith("-")):
                        options.append(line);
                        line = lines.readline().strip("\n").strip()
                    param = ParameterSelection(paramName, paramDescription, options)
                    self.addParameter(param)
                    if line == "":
                        break
                    else:
                        readLine = False
            if readLine:
                line = lines.readline()
        lines.close()


    def calculateResamplingExtent(self):
        auto = SextanteConfig.getSetting(SagaUtils.SAGA_AUTO_RESAMPLING)
        if auto:
            first = True;
            for param in self.parameters:
                if isinstance(param, ParameterRaster):
                    if isinstance(param.value, QgsRasterLayer):
                        value = param.value
                    else:
                        value = QGisLayers.getObjectFromUri(param.value)
                    if first:
                        self.xmin = value.extent().xMinimum()
                        self.xmax = value.extent().xMaximum()
                        self.ymin = value.extent().yMinimum()
                        self.ymax = value.extent().yMaximum()
                        self.cellsize = (value.extent().xMaximum() - value.extent().xMinimum())/value.getRasterXDim()
                        first = False
                    else:
                        self.xmin = min(self.xmin, value.extent().xMinimum())
                        self.xmax = max(self.xmax, value.extent().xMaximum())
                        self.ymin = min(self.ymin, value.extent().yMinimum())
                        self.ymax = max(self.ymax, value.extent().yMaximum())
                        self.cellsize = max(self.cellsize, (value.extent().xMaximum() - value.extent().xMinimum())/value.getRasterXDim())
        else:
            self.xmin = SextanteConfig.getSetting(SagaUtils.SAGA_RESAMPLING_REGION_XMIN)
            self.xmax = SextanteConfig.getSetting(SagaUtils.SAGA_RESAMPLING_REGION_XMAX)
            self.ymin = SextanteConfig.getSetting(SagaUtils.SAGA_RESAMPLING_REGION_YMIN)
            self.ymax = SextanteConfig.getSetting(SagaUtils.SAGA_RESAMPLING_REGION_YMAX)
            self.cellsize = SextanteConfig.getSetting(SagaUtils.SAGA_RESAMPLING_REGION_CELLSIZE)



    def processAlgorithm(self, progress):
        path = SagaUtils.sagaPath()
        if path == "":
            raise GeoAlgorithmExecutionException("SAGA folder is not configured.\nPlease configure it before running SAGA algorithms.")
        useSelection = SextanteConfig.getSetting(SagaUtils.SAGA_USE_SELECTED)
        commands = list()
        self.exportedLayers = {}
        self.numExportedLayers = 0;

        #1: Export rasters to sgrd and vectors to shp
        #   Tables must be in dbf format. We check that.
        if self.resample:
            self.calculateResamplingExtent()
        for param in self.parameters:
            if isinstance(param, ParameterRaster):
                if param.value == None:
                    continue
                #===============================================================
                # if isinstance(param.value, QgsRasterLayer):
                #    value = str(param.value.dataProvider().dataSourceUri())
                # else:
                #===============================================================
                value = param.value
                if not value.endswith("sgrd"):
                    commands.append(self.exportRasterLayer(value))
                if self.resample:
                    commands.append(self.resampleRasterLayer(value));
            if isinstance(param, ParameterVector):
                if param.value == None:
                    continue
                #===============================================================
                # if isinstance(param.value, QgsVectorLayer):
                #    value = str(param.value.dataProvider().dataSourceUri())
                # else:
                #===============================================================
                value = param.value
                if (not value.endswith("shp")) or useSelection:
                    self.exportVectorLayer(value)
                    #raise GeoAlgorithmExecutionException("Unsupported file format")
            if isinstance(param, ParameterTable):
                if param.value == None:
                    continue
                #===============================================================
                # if isinstance(param.value, QgsVectorLayer):
                #    value = str(param.value.dataProvider().dataSourceUri())
                # else:
                #===============================================================
                value = param.value
                if value.endswith("shp"):
                    value = value[:-3] + "dbf"
                if not value.endswith("dbf"):
                    raise GeoAlgorithmExecutionException("Unsupported file format")
            if isinstance(param, ParameterMultipleInput):
                if param.value == None:
                    continue
                layers = param.value.split(";")
                if layers == None or len(layers) == 0:
                    continue
                if param.datatype == ParameterMultipleInput.TYPE_RASTER:
                    for layer in layers:
                        if not layer.endswith("sgrd"):
                            commands.append(self.exportRasterLayer(layer))
                        if self.resample:
                            commands.append(self.resampleRasterLayer(layer));
                elif param.datatype == ParameterMultipleInput.TYPE_VECTOR_ANY:
                    for layer in layers:
                        if not layer.endswith("shp"):
                            raise GeoAlgorithmExecutionException("Unsupported file format")

        #2: set parameters and outputs
        command = self.undecoratedGroup  + " \"" + self.name + "\""
        for param in self.parameters:
            if param.value == None:
                continue
            if isinstance(param, (ParameterRaster, ParameterVector)):
                value = param.value
                if value in self.exportedLayers.keys():
                    command+=(" -" + param.name + " " + self.exportedLayers[value])
                else:
                    command+=(" -" + param.name + " " + value)
            elif isinstance(param, ParameterMultipleInput):
                s = param.value
                for layer in self.exportedLayers.keys():
                    if layer in self.exportedLayers.keys():
                        s = s.replace(layer, self.exportedLayers[layer])
                command+=(" -" + param.name + " " + s);
            elif isinstance(param, ParameterBoolean):
                if param.value:
                    command+=(" -" + param.name);
            else:
                command+=(" -" + param.name + " " + str(param.value));


        for out in self.outputs:
            if isinstance(out, OutputRaster):
                filename = out.value
                if not filename.endswith(".asc") and not filename.endswith(".tif"):
                    filename += ".tif"
                    out.value = filename
                filename = SextanteUtils.tempFolder() + os.sep + os.path.basename(filename) + ".sgrd"
                command+=(" -" + out.name + " " + filename);
            if isinstance(out, OutputVector):
                filename = out.value
                if not filename.endswith(".shp"):
                    filename += ".shp"
                    out.value = filename
                command+=(" -" + out.name + " " + filename);
            if isinstance(out, OutputTable):
                filename = out.value
                if not filename.endswith(".dbf"):
                    filename += ".dbf"
                    out.value = filename
                command+=(" -" + out.name + " " + filename);

        commands.append(command)

      #3:Export resulting raster layers
        for out in self.outputs:
            if isinstance(out, OutputRaster):
                filename = out.value
                filename2 = SextanteUtils.tempFolder() + os.sep + os.path.basename(filename) + ".sgrd"
                if filename.endswith("asc"):
                    commands.append("io_grid 0 -GRID " + filename2 + " -FORMAT 1 -FILE " + filename);
                else:
                    commands.append("io_gdal 1 -GRIDS " + filename2 + " -FORMAT 1 -TYPE 0 -FILE " + filename);

        #4 Run SAGA
        SagaUtils.createSagaBatchJobFileFromSagaCommands(commands)
        loglines = []
        loglines.append("SAGA execution commands")
        for line in commands:
            loglines.append(line)
        SextanteLog.addToLog(SextanteLog.LOG_INFO, loglines)
        SagaUtils.executeSaga(progress);


    def resampleRasterLayer(self,layer):
        if layer in self.exportedLayers.keys:
            inputFilename = self.exportedLayers[layer]
        else:
            inputFilename = layer
        destFilename = self.getTempFilename()
        self.exportedLayers[layer]= destFilename
        s = "grid_tools \"Resampling\" -INPUT " + inputFilename + "-TARGET 0 -SCALE_UP_METHOD 4 -SCALE_DOWN_METHOD 4 -USER_XMIN " +\
                self.xmin + " -USER_XMAX " + self.xmax + " -USER_YMIN " + self.ymin + " -USER_YMAX "  + self.ymax +\
                " -USER_SIZE " + str(self.cellsize) + " -USER_GRID " + destFilename
        return s


    def getObjectFromUri(self, uri):
        layers = QGisLayers.getVectorLayers()
        for layer in layers:
            if layer.source() == uri:
                return layer
        return None

    def exportVectorLayer(self, filename):
        layer = self.getObjectFromUri(filename)
        if layer:
            settings = QSettings()
            systemEncoding = settings.value( "/UI/encoding", "System" ).toString()
            output = self.getTempFilename("shp")
            provider = layer.dataProvider()
            allAttrs = provider.attributeIndexes()
            provider.select( allAttrs )
            useSelection = SextanteConfig.getSetting(SagaUtils.SAGA_USE_SELECTED)
            if useSelection and layer.selectedFeatureCount() != 0:
                writer = QgsVectorFileWriter( output, systemEncoding,provider.fields(), provider.geometryType(), provider.crs() )
                selection = layer.selectedFeatures()
                for feat in selection:
                    writer.addFeature(feat)
                del writer
                self.exportedLayers[filename]=output
            else:
                if (not filename.endswith("shp")):
                    writer = QgsVectorFileWriter( output, systemEncoding,provider.fields(), provider.geometryType(), provider.crs() )
                    feat = QgsFeature()
                    while provider.nextFeature(feat):
                        writer.addFeature(feat)
                    del writer
                    self.exportedLayers[filename]=output
        else:
            if (not filename.endswith("shp")):
                raise GeoAlgorithmExecutionException("Unsupported file format")

    def exportRasterLayer(self, layer):
        #=======================================================================
        # if not layer.lower().endswith("tif") and not layer.lower().endswith("asc"):
        #    raise GeoAlgorithmExecutionException("Unsupported input file format: " + layer)
        #=======================================================================
        #ext = os.path.splitext(layer)[1][1:].strip()
        destFilename = self.getTempFilename("sgrd")
        self.exportedLayers[layer]= destFilename
        #=======================================================================
        # if ext.lower() == "tif":
        #    return "io_grid_image 1 -OUT_GRID " + destFilename + " -FILE " + layer + " -METHOD 0"
        # else:
        #=======================================================================
        return "io_gdal 0 -GRIDS " + destFilename + " -FILES " + layer


    def getTempFilename(self, ext):
        self.numExportedLayers+=1
        path = SextanteUtils.tempFolder()
        filename = path + os.sep + str(time.time()) + str(SextanteUtils.NUM_EXPORTED) + "." + ext
        SextanteUtils.NUM_EXPORTED +=1

        return filename


