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
from sextante.script.ScriptUtils import ScriptUtils
import os
from sextante.core.SextanteUtils import SextanteUtils
from sextante.parameters.ParameterSelection import ParameterSelection

class ScriptAlgorithm(GeoAlgorithm):

    def __init__(self, descriptionfile):
        GeoAlgorithm.__init__(self)
        self.descriptionFile = descriptionfile
        self.defineCharacteristicsFromFile()
        self.providerName = "script:"


    def defineCharacteristicsFromFile(self):
        self.script=""
        self.name = self.descriptionFile[:self.descriptionFile.rfind(".")]
        fullpath = os.path.join(ScriptUtils.scriptsFolder(), self.descriptionFile)
        self.group = "User scripts"
        lines = open(fullpath)
        line = lines.readline()
        while line != "":
            if line.startswith("##"):
                self.processParameterLine(line.strip("\n"))
            self.script += line
            line = lines.readline()
        lines.close()


    def processParameterLine(self,line):

        param = None
        out = None
        line = line.replace("#", "");
        tokens = line.split("=");
        if tokens[1].lower() == "raster":
            param = ParameterRaster(tokens[0], tokens[0], False)
        elif tokens[1].lower() == "vector":
            param = ParameterVector(tokens[0], tokens[0],ParameterVector.VECTOR_TYPE_ANY)
        elif tokens[1].lower() == "table":
            param = ParameterTable(tokens[0], tokens[0], False)
        elif tokens[1].lower() == "multiple raster":
            param = ParameterMultipleInput(tokens[0], tokens[0], ParameterMultipleInput.TYPE_RASTER)
            param.optional = False
        elif tokens[1].lower() == "multiple vector":
            param = ParameterMultipleInput(tokens[0], tokens[0], ParameterMultipleInput.TYPE_VECTOR_ANY)
            param.optional = False
        elif tokens[1].lower().startswith("selection"):
            options = tokens[1][len("selection"):].split(";")
            param = ParameterSelection(tokens[0],  tokens[0], options);
        elif tokens[1].lower() == "boolean":
            param = ParameterBoolean(tokens[0],  tokens[0])
        elif tokens[1].lower() == "number":
            default = tokens[1][len("number")+1:]
            param = ParameterNumber(tokens[0],  tokens[0], default)
        elif tokens[1].lower() == "string":
            default = tokens[1][len("string")+1:]
            param = ParameterString(tokens[0],  tokens[0], default)
        elif tokens[1].lower() == "output raster":
            out = OutputRaster()
        elif tokens[1].lower() == "output vector":
            out = OutputVector()
        elif tokens[1].lower() == "output table":
            out = OutputTable()

        if param != None:
            self.putParameter(param)
        elif out != None:
            out.name = tokens[0]
            out.description = tokens[0]
            self.putOutput(out)
        else:
            raise WrongScriptException("Could not load script:" + self.descriptionFile + ". Problem with line \"" + line + "\"")

    def processAlgorithm(self, progress):

        #resolve temporary output files
        for out in self.outputs:
            if out.channel == None:
                SextanteUtils.setTempOutput(out)

        script = "from sextante.core.Sextante import Sextante\n"
        for param in self.parameters:
            script += param.name + "=" + param.getValueAsCommandLineParameter() + "\n"

        for out in self.outputs:
            script += out.name + "=" + out.getChannelAsCommandLineParameter() + "\n"

        script+=self.script
        exec(script)


