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
from sextante.outputs.OutputDataObject import OutputDataObject
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

class SagaAlgorithm(GeoAlgorithm):

    def __init__(self, descriptionfile):
        GeoAlgorithm.__init__(self)
        self._descriptionFile = descriptionfile
        self.defineCharacteristicsFromFile()
        self.numExportedLayers = 0
        self.providerName = "saga:"

    def defineCharacteristicsFromFile(self):

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
                if "data object" in line or "file" in line:
                    if "output" in line:
                        output = OutputDataObject()
                        output.name = paramName
                        output.description = paramDescription
                    else:
                        raise UnwrappableSagaAlgorithmException()
                if "table" in line:
                    #print(line)
                    if "input" in line:
                        param = ParameterTable()
                        param.name = paramName
                        param.description = paramDescription
                        param.optional = ("optional" in line)
                        lastParentParameterName = paramName;
                        self.putParameter(param)
                    elif "static" in line:
                        print (self.name)
                        #line = lines.readline()
                        line = lines.readline().strip("\n").strip()
                        numCols = int(line.split(" ")[0])
                        colNames = [];
                        for i in range(numCols):
                            line = lines.readline().strip("\n").strip()
                            colNames.append(line.split("]")[1])
                        param = ParameterFixedTable()
                        param.name = paramName
                        param.description = paramDescription
                        param.cols = colNames
                        param.numRows = 3
                        param.fixedNumOfRows = False
                        self.putParameter(param)
                        self.ok = True
                    elif "field" in line:
                        if lastParentParameterName == None:
                            raise UnwrappableSagaAlgorithmException();
                        param = ParameterTableField()
                        param.name = paramName
                        param.description = paramDescription
                        param.parent = lastParentParameterName
                        self.putParameter(param)
                    else:
                        output = OutputTable()
                        output.name = paramName
                        output.description = paramDescription
                elif "grid" in line:
                    if "input" in line:
                        if "list" in line:
                            param = ParameterMultipleInput()
                            param.name = paramName
                            param.description = paramDescription
                            param.optional = ("optional" in line)
                            param.datatype=ParameterMultipleInput.TYPE_RASTER
                            self.putParameter(param)
                        else:
                            param = ParameterRaster()
                            param.name = paramName
                            param.description = paramDescription
                            param.optional = ("optional" in line)
                            self.putParameter(param)
                    else:
                        output = OutputRaster()
                        output.name = paramName
                        output.description = paramDescription
                        self.putOutput(output)
                elif "shapes" in line:
                    if "input" in line:
                        if "list" in line:
                            param = ParameterMultipleInput()
                            param.name = paramName
                            param.description = paramDescription
                            param.optional = ("optional" in line)
                            param.datatype=ParameterMultipleInput.TYPE_VECTOR_ANY
                            self.putParameter(param)
                        else:
                            param = ParameterVector()
                            param.name = paramName
                            param.description = paramDescription
                            param.optional = ("optional" in line)
                            param.shapetype = ParameterVector.VECTOR_TYPE_ANY
                            lastParentParameterName = paramName;
                            self.putParameter(param)
                    else:
                        output = OutputVector()
                        output.name = paramName
                        output.description = paramDescription
                        self.putOutput(output)
                elif "floating" in line or "integer" in line:
                    param = ParameterNumber()
                    param.name = paramName
                    param.description = paramDescription
                    self.putParameter(param)
                elif "boolean" in line:
                    param = ParameterBoolean()
                    param.name = paramName
                    param.description = paramDescription
                    self.putParameter(param)
                elif "text" in line:
                    param = ParameterString()
                    param.name = paramName
                    param.description = paramDescription
                    self.putParameter(param)
                elif "range" in line:
                    param = ParameterRange()
                    param.name = paramName
                    param.description = paramDescription
                    self.putParameter(param)
                elif "choice" in line:
                    line = lines.readline()
                    line = lines.readline().strip("\n").strip()
                    options = list()
                    while line != "" and not (line.startswith("-")):
                        options.append(line);
                        line = lines.readline().strip("\n").strip()
                    param = ParameterSelection()
                    param.name = paramName
                    param.description = paramDescription
                    param.options = options
                    self.putParameter(param)
                    if line == "":
                        break
                    else:
                        readLine = False
            if readLine:
                line = lines.readline()
        lines.close()

    def processAlgorithm(self, progress):

        commands = list()
        self.exportedLayers = {}
        self.numExportedLayers = 0;

        #resolve temporary output files
        for out in self.outputs:
            if out.channel == None:
                SextanteUtils.setTempOutput(out)

      #1: Export rasters to sgrd. only ASC and TIF are supported.
      #   Vector layers must be in shapefile format and tables in dbf format. We check that.
        for param in self.parameters:
            if isinstance(param, ParameterRaster):
                if param.value == None:
                    continue;
                if isinstance(param.value, QgsRasterLayer):
                    value = str(param.value.dataProvider().dataSourceUri())
                else:
                    value = param.value
                if not value.endswith("sgrd"):
                    commands.append(self.exportRasterLayer(value));
            if isinstance(param, ParameterVector):
                if param.value == None:
                    continue;
                if isinstance(param.value, QgsVectorLayer):
                    value = str(param.value.dataProvider().dataSourceUri())
                else:
                    value = param.value
                if not value.endswith("shp"):
                    raise GeoAlgorithmExecutionException("Unsupported file format")
            if isinstance(param, ParameterTable):
                if param.value == None:
                    continue;
                if isinstance(param.value, QgsVectorLayer):
                    value = str(param.value.dataProvider().dataSourceUri())
                else:
                    value = param.value
                if value.endswith("shp"):
                    value = value[:-3] + "dbf"
                if not value.endswith("dbf"):
                    raise GeoAlgorithmExecutionException("Unsupported file format")
            if isinstance(param, ParameterMultipleInput):
                layers = param.value
                if layers == None or len(layers) == 0:
                    continue
                if param.datatype == ParameterMultipleInput.TYPE_RASTER:
                    for layer in layers:
                        if isinstance(layer, QgsRasterLayer):
                            layer = str(layer.dataProvider().dataSourceUri())
                        commands.append(self.exportRasterLayer(layer))
                elif param.datatype == ParameterMultipleInput.TYPE_VECTOR_ANY:
                    for layer in layers:
                        if isinstance(layer, QgsVectorLayer):
                            layer = str(layer.dataProvider().dataSourceUri())
                        if not layer.endswith("shp"):
                            raise GeoAlgorithmExecutionException("Unsupported file format")

        #2: set parameters and outputs
        command = self.undecoratedGroup  + " \"" + self.name + "\""
        for param in self.parameters:
            if isinstance(param, ParameterRaster):
                value = param.value
                if value in self.exportedLayers.keys():
                    command+=(" -" + param.name + " " + self.exportedLayers[value])
                else:
                    command+=(" -" + param.name + " " + value)
            elif isinstance(param, ParameterMultipleInput):
                s = param.value
                for layer in self.exportedLayers.keys():
                    if layer in self.exportedLayers.keys():
                        s = s.replace(layer, self.exportedlayer[layer])
                command+=(" -" + param.name + " " + s);
            elif isinstance(param, ParameterBoolean):
                if (bool(param.value)):
                    command+=(" -" + param.name);
            else:
                command+=(" -" + param.name + " " + param.value);


        for out in self.outputs:
            if isinstance(out, OutputRaster):
                filename = out.channel
                if not filename.endswith(".asc") and not filename.endswith(".tif"):
                    filename += ".tif"
                    out.channel = filename
                filename = SextanteUtils.tempFolder() + os.sep + os.path.basename(filename) + ".sgrd"
                command+=(" -" + out.name + " " + filename);
            if isinstance(out, OutputVector):
                filename = out.channel
                if not filename.endswith(".shp"):
                    filename += ".shp"
                    out.channel = filename
                command+=(" -" + out.name + " " + filename);
            if isinstance(out, OutputTable):
                filename = out.channel
                if not filename.endswith(".dbf"):
                    filename += ".dbf"
                    out.channel = filename
                command+=(" -" + out.name + " " + filename);

        commands.append(command)

      #3:Export resulting raster layers
        for out in self.outputs:
            if isinstance(out, OutputRaster):
                filename = out.channel
                filename2 = SextanteUtils.tempFolder() + os.sep + os.path.basename(filename) + ".sgrd"
                if filename.endswith("asc"):
                    commands.append("io_grid 0 -GRID " + filename2 + " -FORMAT 1 -FILE " + filename);
                else:
                    commands.append("io_gdal 1 -GRIDS " + filename2 + " -FORMAT 1 -FILE " + filename);

      #4 Run SAGA

        SagaUtils.createSagaBatchJobFileFromSagaCommands(commands)
        SagaUtils.executeSaga(self, progress);


    def exportRasterLayer(self,layer):

        if not layer.lower().endswith("tif") and not layer.lower().endswith("asc"):
            raise GeoAlgorithmExecutionException("Unsupported input file format")
        ext = os.path.splitext(layer)[1][1:].strip()
        destFilename = self.getTempFilename()
        self.exportedLayers[layer]= destFilename
        if ext.lower() == "tif":
            return "io_grid_image 1 -OUT_GRID " + destFilename + " -FILE " + layer + " -METHOD 0"
        else:
            return "io_grid 1 -GRID " + destFilename + " -FILE " + layer


    def getTempFilename(self):
        self.numExportedLayers+=1
        path = SextanteUtils.tempFolder()
        filename = path + os.sep + str(time.time()) + str(SextanteUtils.NUM_EXPORTED) + ".sgrd"
        SextanteUtils.NUM_EXPORTED +=1

        return filename


