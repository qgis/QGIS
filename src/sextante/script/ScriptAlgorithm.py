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
from sextante.parameters.ParameterDataObject import ParameterDataObject
from sextante.parameters.ParameterRange import ParameterRange

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
            param = ParameterRaster()
            param.optional = False
        elif tokens[1].lower() == "vector":
            param = ParameterRaster()
            param.optional = False
            param.shapetype = ParameterVector.VECTOR_TYPE_ANY
        elif tokens[1].lower() == "table":
            param = ParameterTable()
            param.optional = False
        elif tokens[1].lower() == "multiple raster":
            param = ParameterMultipleInput();
            param.datatype=ParameterMultipleInput.TYPE_RASTER
            param.optional = False
        elif tokens[1].lower() == "multiple vector":
            param = ParameterMultipleInput();
            param.datatype=ParameterMultipleInput.TYPE_VECTOR_ANY
            param.optional = False
        elif tokens[1].lower() == "boolean":
            param = ParameterBoolean()
        elif tokens[1].lower() == "number":
            param = ParameterNumber()
        elif tokens[1].lower() == "string":
            param = ParameterString()
        elif tokens[1].lower() == "output raster":
            out = OutputRaster()
        elif tokens[1].lower() == "output vector":
            out = OutputVector()
        elif tokens[1].lower() == "output table":
            out = OutputTable()

        if param != None:
            param.name = tokens[0]
            param.description = tokens[0]
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


