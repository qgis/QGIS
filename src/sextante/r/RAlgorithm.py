import os
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtGui, QtCore
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
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.parameters.ParameterTableField import ParameterTableField
from sextante.outputs.OutputHTML import OutputHTML
from sextante.r.RUtils import RUtils
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.core.SextanteLog import SextanteLog
from sextante.core.SextanteUtils import SextanteUtils
import subprocess
from sextante.parameters.ParameterExtent import ParameterExtent
from sextante.parameters.ParameterFile import ParameterFile
from sextante.outputs.OutputFile import OutputFile
from sextante.gui.Help2Html import Help2Html

class RAlgorithm(GeoAlgorithm):

    R_CONSOLE_OUTPUT = "R_CONSOLE_OUTPUT"
    RPLOTS = "RPLOTS"

    def getCopy(self):
        newone = RAlgorithm(self.descriptionFile)
        newone.provider = self.provider
        return newone

    def __init__(self, descriptionFile, script=None):
        GeoAlgorithm.__init__(self)
        self.script = script
        self.descriptionFile = descriptionFile
        if script is not None:
            self.defineCharacteristicsFromScript()
        if descriptionFile is not None:
            self.defineCharacteristicsFromFile()

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/r.png")

    def defineCharacteristicsFromScript(self):
        lines = self.script.split("\n")
        self.silentOutputs = []
        self.name = "[Unnamed algorithm]"
        self.group = "User R scripts"
        for line in lines:
            if line.startswith("##"):
                try:
                    self.processParameterLine(line.strip("\n"))
                except:
                    pass

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
        elif tokens[1].lower().strip().startswith("boolean"):
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
        elif tokens[1].lower().strip() == "extent":
            param = ParameterExtent(tokens[0],  desc)
        elif tokens[1].lower().strip() == "file":
            param = ParameterFile(tokens[0],  desc, False)
        elif tokens[1].lower().strip() == "folder":
            param = ParameterFile(tokens[0],  desc, True)
        elif tokens[1].lower().strip().startswith("string"):
            default = tokens[1].strip()[len("string")+1:]
            param = ParameterString(tokens[0],  desc, default)
        elif tokens[1].lower().strip().startswith("output raster"):
            out = OutputRaster()
        elif tokens[1].lower().strip().startswith("output vector"):
            out = OutputVector()
        elif tokens[1].lower().strip().startswith("output table"):
            out = OutputTable()
        elif tokens[1].lower().strip().startswith("output file"):
            out = OutputFile()

        if param != None:
            self.addParameter(param)
        elif out != None:
            out.name = tokens[0]
            out.description = tokens[0]
            self.addOutput(out)
        else:
            raise WrongScriptException("Could not load R script:" + self.descriptionFile + ".\n Problem with line \"" + line + "\"")

    def processAlgorithm(self, progress):
        if SextanteUtils.isWindows():
            path = RUtils.RFolder()
            if path == "":
                raise GeoAlgorithmExecutionException("R folder is not configured.\nPlease configure it before running R scripts.")
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
                value = value.replace("\\", "/")
                commands.append(param.name + " = " + "readGDAL(\"" + value + "\")")
            if isinstance(param, ParameterVector):
                value = param.getSafeExportedLayer()
                value = value.replace("\\", "/")
                filename = os.path.basename(value)
                filename = filename[:-4]
                commands.append(param.name + " = readOGR(\"" + value + "\",layer=\"" + filename + "\")")
            if isinstance(param, ParameterTable):
                value = param.value
                if not value.lower().endswith("csv"):
                    raise GeoAlgorithmExecutionException("Unsupported input file format.\n" + value)
                commands.append(param.name + " <- read.csv(\"" + value + "\", head=TRUE, sep=\",\")")
            if isinstance(param, (ParameterTableField, ParameterString)):
                commands.append(param.name + "=\"" + param.value + "\"")
            if isinstance(param, (ParameterNumber, ParameterSelection)):
                commands.append(param.name + "=" + str(param.value))
            if isinstance(param, ParameterBoolean):
                if param.value:
                    commands.append(param.name + "=TRUE")
                else:
                    commands.append(param.name + "=FALSE")
            if isinstance(param, ParameterMultipleInput):
                iLayer = 0;
                if param.datatype == ParameterMultipleInput.TYPE_RASTER:
                    layers = param.value.split(";")
                    for layer in layers:
                        if not layer.lower().endswith("asc") and not layer.lower().endswith("tif"):
                            raise GeoAlgorithmExecutionException("Unsupported input file format.\n" + layer)
                        layer = layer.replace("\\", "/")
                        commands.append("tempvar" + str(iLayer)+ " = " + "readGDAL(\"" + layer + "\"")
                        iLayer+=1
                else:
                    exported = param.getSafeExportedLayers()
                    layers = exported.split(";")
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
                for layer in layers:
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

    def helpFile(self):
        helpfile = self.descriptionFile + ".help"
        if os.path.exists(helpfile):
            h2h = Help2Html()
            return h2h.getHtmlFile(self, helpfile)
        else:
            return None

    def checkBeforeOpeningParametersDialog(self):
        if SextanteUtils.isWindows():
            path = RUtils.RFolder()
            if path == "":
                return "R folder is not configured.\nPlease configure it before running R scripts."
        else:
            R_INSTALLED = "R_INSTALLED"
            settings = QSettings()
            if settings.contains(R_INSTALLED):
                return
            command = ["R --version"]
            proc = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE,stderr=subprocess.STDOUT, universal_newlines=True).stdout
            for line in iter(proc.readline, ""):
                if "R version" in line:
                    settings.setValue(R_INSTALLED, True)
                    return
            return "It seems that R is not correctly installed in your system.\nPlease install it before running R Scripts."

