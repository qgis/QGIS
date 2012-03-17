from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.parameters.ParameterTable import ParameterTable
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterMultipleInput import ParameterMultipleInput
from sextante.script.WrongScriptException import WrongScriptException
from sextante.outputs.OutputTable import OutputTable
from sextante.outputs.OutputVector import OutputVector
from sextante.outputs.OutputRaster import OutputRaster
from sextante.parameters.ParameterString import ParameterString
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.parameters.ParameterBoolean import ParameterBoolean
import os
from sextante.parameters.ParameterSelection import ParameterSelection
from PyQt4 import QtGui
from sextante.parameters.ParameterTableField import ParameterTableField
from sextante.outputs.OutputHTML import OutputHTML
from sextante.r.RUtils import RUtils
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.core.SextanteLog import SextanteLog

class RAlgorithm(GeoAlgorithm):

    R_CONSOLE_OUTPUT = "R_CONSOLE_OUTPUT"
    RPLOTS = "RPLOTS"

    def __deepcopy__(self,memo):
        newone = RAlgorithm(self.descriptionFile)
        newone.provider = self.provider
        return newone

    def __init__(self, descriptionfile):
        GeoAlgorithm.__init__(self)
        self.descriptionFile = descriptionfile
        self.defineCharacteristicsFromFile()

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/script.png")

    def defineCharacteristicsFromFile(self):
        self.script = ""
        self.commands=[]
        self.showPlots = False
        self.showConsoleOutput = False
        self.verboseCommands = []
        filename = os.path.basename(self.descriptionFile)
        self.name = filename[:filename.rfind(".")].replace("_", " ")
        self.group = "User R scripts"
        lines = open(self.descriptionFile)
        line = lines.readline().strip("\n")
        while line != "":
            if line.startswith("##"):
                try:
                    self.processParameterLine(line)
                except Exception:
                    raise WrongScriptException("Could not load R script:" + self.descriptionFile + ".\n Problem with line \"" + line + "\"")
            elif line.startswith(">"):
                self.commands.append(line[1:])
                self.verboseCommands.append(line[1:])
                if not self.showConsoleOutput:
                    self.addOutput(OutputHTML(RAlgorithm.R_CONSOLE_OUTPUT, "R Console Output"))
                self.showConsoleOutput = True
            else:
                self.commands.append(line)
            self.script += line + "\n"
            line = lines.readline().strip("\n")
        lines.close()

    def getVerboseCommands(self):
        return self.verboseCommands

    def createDescriptiveName(self, s):
        return s.replace("_", " ")

    def processParameterLine(self,line):
        param = None
        out = None
        line = line.replace("#", "");
        if line.lower().strip().startswith("showplots"):
            self.showPlots = True
            self.addOutput(OutputHTML(RAlgorithm.RPLOTS, "R Plots"));
            return
        tokens = line.split("=");
        desc = self.createDescriptiveName(tokens[0])
        if tokens[1].lower().strip() == "group":
            self.group = tokens[0]
            return
        if tokens[1].lower().strip() == "raster":
            param = ParameterRaster(tokens[0], desc, False)
        elif tokens[1].lower().strip() == "vector":
            param = ParameterVector(tokens[0],  desc,ParameterVector.VECTOR_TYPE_ANY)
        elif tokens[1].lower().strip() == "table":
            param = ParameterTable(tokens[0], desc, False)
        elif tokens[1].lower().strip() == "multiple raster":
            param = ParameterMultipleInput(tokens[0], desc, ParameterMultipleInput.TYPE_RASTER)
            param.optional = False
        elif tokens[1].lower().strip() == "multiple vector":
            param = ParameterMultipleInput(tokens[0], desc, ParameterMultipleInput.TYPE_VECTOR_ANY)
            param.optional = False
        elif tokens[1].lower().strip().startswith("selection"):
            options = tokens[1].strip()[len("selection"):].split(";")
            param = ParameterSelection(tokens[0],  desc, options);
        elif tokens[1].lower().strip() == "boolean":
            default = tokens[1].strip()[len("boolean")+1:]
            param = ParameterBoolean(tokens[0],  desc, default)
        elif tokens[1].lower().strip().startswith("number"):
            try:
                default = float(tokens[1].strip()[len("number")+1:])
                param = ParameterNumber(tokens[0],  desc, default=default)
            except:
                raise WrongScriptException("Could not load R script:" + self.descriptionFile + ".\n Problem with line \"" + line + "\"")
        elif tokens[1].lower().strip().startswith("field"):
            field = tokens[1].strip()[len("field")+1:]
            found = False
            for p in self.parameters:
                if p.name == field:
                    found = True
                    break
            if found:
                param = ParameterTableField(tokens[0],  tokens[0], field)
        elif tokens[1].lower().strip().startswith("string"):
            default = tokens[1].strip()[len("string")+1:]
            param = ParameterString(tokens[0],  desc, default)
        elif tokens[1].lower().strip().startswith("output raster"):
            out = OutputRaster()
            if tokens[1].strip().endswith("*"):
                self.silentOutputs.append(tokens[0])
        elif tokens[1].lower().strip().startswith("output vector"):
            out = OutputVector()
            if tokens[1].strip().endswith("*"):
                self.silentOutputs.append(tokens[0])
        elif tokens[1].lower().strip().startswith("output table"):
            out = OutputTable()
            if tokens[1].strip().endswith("*"):
                self.silentOutputs.append(tokens[0])

        if param != None:
            self.addParameter(param)
        elif out != None:
            out.name = tokens[0]
            out.description = tokens[0]
            self.addOutput(out)
        else:
            raise WrongScriptException("Could not load R script:" + self.descriptionFile + ".\n Problem with line \"" + line + "\"")

    def processAlgorithm(self, progress):
        path = RUtils.RFolder()
        if path == "":
            raise GeoAlgorithmExecutionException("R folder is not configured.\nPlease configure it before running R script.")
        loglines = []
        loglines.append("R execution commands")
        loglines += self.getFullSetOfRCommands()
        SextanteLog.addToLog(SextanteLog.LOG_INFO, loglines)
        RUtils.executeRAlgorithm(self)
        if self.showPlots:
            htmlfilename = self.getOutputValue(RAlgorithm.RPLOTS)
            f = open(htmlfilename, "w")
            f.write("<img src=\"" + self.plotsFilename + "\"/>")
            f.close()
        if self.showConsoleOutput:
            htmlfilename = self.getOutputValue(RAlgorithm.R_CONSOLE_OUTPUT)
            f = open(htmlfilename)
            f.write(RUtils.getConsoleOutput())
            f.close()

    def getFullSetOfRCommands(self):
        commands = []
        commands += self.getImportCommands()
        commands += self.getRCommands()
        commands += self.getExportCommands()

        return commands

    def getExportCommands(self):
        commands = []
        for out in self.outputs:
            if isinstance(out, OutputRaster):
                value = out.value
                if not value.endswith("tif"):
                    value = value + ".tif"
                value = value.replace("\\", "/")
                commands.append("writeGDAL(" + out.name + ",\"" + value + "\")")
            if isinstance(out, OutputVector):
                value = out.value
                if not value.endswith("shp"):
                    value = value + ".shp"
                value = value.replace("\\", "/")
                filename = os.path.basename(value)
                filename = filename[:-4]
                commands.append("writeOGR(" + out.name + ",\"" + value + "\",\""
                            + filename + "\", driver=\"ESRI Shapefile\")");

        if self.showPlots:
            commands.append("dev.off()");

        return commands


    def getImportCommands(self):
        commands = []
        commands.append("library(\"rgdal\")");
        for param in self.parameters:
            if isinstance(param, ParameterRaster):
                value = param.value
                if not value.lower().endswith("asc") and not value.lower().endswith("tif"):
                  raise GeoAlgorithmExecutionException("Unsupported input file format.\n" + value)
                value = value.replace("\\", "/")
                commands.append(param.name + " = " + "readGDAL(\"" + value + "\"")
            if isinstance(param, ParameterVector):
                value = param.value
                if not value.lower().endswith("shp"):
                  raise GeoAlgorithmExecutionException("Unsupported input file format.\n" + value)
                value = value.replace("\\", "/")
                filename = os.path.basename(value)
                filename = filename[:-4]
                commands.append(param.name + " = " + "readOGR(\"" + value + "\",layer=\"" + filename + "\")")
            if isinstance(param, (ParameterTableField, ParameterString)):
                commands.append(param.name + "=\"" + param.value + "\"")
            if isinstance(param, ParameterNumber):
                commands.append(param.name + "=" + str(param.value))
            if isinstance(param, ParameterBoolean):
                if param.value:
                    commands.append(param.name + "=TRUE")
                else:
                    commands.append(param.name + "=FALSE")
            if isinstance(param, ParameterMultipleInput):
                layers = param.value.split(";")
                iLayer = 0;
                if param.datatype == ParameterMultipleInput.TYPE_RASTER:
                    for layer in layers:
                        if not layer.lower().endswith("asc") and not layer.lower().endswith("tif"):
                            raise GeoAlgorithmExecutionException("Unsupported input file format.\n" + layer)
                        layer = layer.replace("\\", "/")
                        commands.append("tempvar" + str(iLayer)+ " = " + "readGDAL(\"" + layer + "\"")
                        iLayer+=1
                else:
                    for layer in layers:
                        if not layer.lower().endswith("shp"):
                            raise GeoAlgorithmExecutionException("Unsupported input file format.\n" + layer)
                        layer = layer.replace("\\", "/")
                        filename = os.path.basename(layer)
                        filename = filename[:-4]
                        commands.append("tempvar" + str(iLayer) + " = " + "readOGR(\"" + layer + "\",layer=\"" + filename + "\")")
                        iLayer+=1
                s = ""
                s += param.name
                s += (" = c(")
                iLayer = 0
                for layer in list:
                    if iLayer != 0:
                        s +=","
                    s += "tempvar" + str(iLayer)
                    iLayer += 1
                s+=")\n"
                commands.append(s)

        if self.showPlots:
            htmlfilename = self.getOutputValue(RAlgorithm.RPLOTS)
            self.plotsFilename = htmlfilename +".png"
            self.plotsFilename = self.plotsFilename.replace("\\", "/");
            commands.append("png(\"" + self.plotsFilename + "\")");

        return commands


    def getRCommands(self):
        return self.commands


