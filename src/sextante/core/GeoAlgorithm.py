from sextante.outputs.Output import Output
from sextante.parameters.Parameter import Parameter
from sextante.core.QGisLayers import QGisLayers
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.parameters.ParameterVector import ParameterVector
from PyQt4 import QtGui
import os.path
from sextante.core.SextanteUtils import SextanteUtils
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.parameters.ParameterBoolean import ParameterBoolean


class GeoAlgorithm:

    def __init__(self):
        self.parameters = list()
        self.outputs = list()
        self.name = ""
        self.group = ""
        self.defineCharacteristics()
        self.providerName = ""
        self.crs = None

    #methods to overwrite when creating a custom geoalgorithm
    #=========================================================
    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/alg.png")

    def processAlgorithm(self):
        pass

    def defineCharacteristics(self):
        pass

    #=========================================================

    def execute(self, progress):
        self.setOutputCRSFromInputLayers()
        self.resolveTemporaryOutputs()
        self.processAlgorithm(progress)

    def resolveTemporaryOutputs(self):
        for out in self.outputs:
            if out.value == None:
                SextanteUtils.setTempOutput(out, self)

    def setOutputCRSFromInputLayers(self):
        for param in self.parameters:
            if isinstance(param, ParameterRaster):
                layers = QGisLayers.getRasterLayers()
            elif isinstance(param, ParameterVector):
                layers = QGisLayers.getVectorLayers()
            else:
                continue
            for layer in layers:
                if layer.source() == param.value:
                    self.crs = layer.crs()
                    return


    def addOutput(self, output):
        #TODO: check that name does not exist
        if isinstance(output, Output):
            self.outputs.append(output)

    def addParameter(self, param):
        #TODO: check that name does not exist
        if isinstance(param, Parameter):
            self.parameters.append(param)

    def setParameterValue(self, paramName, value):
        for param in self.parameters:
            if param.name == paramName:
                param.value = value

    def setOutputValue(self, outputName, value):
        for out in self.outputs:
            if out.name == outputName:
                out.value = value

    def getOutputValuesAsDictionary(self):
        d = {}
        for out in self.outputs:
            d[out.name] = out.value
        return d

    def canBeExecuted(self, layersCount):
        return True

    def __str__(self):
        s = "ALGORITHM: " + self.name + "\n"
        for param in self.parameters:
            s+=("\t" + str(param) + "\n")
        for out in self.outputs:
            s+=("\t" + str(out) + "\n")
        s+=("\n")
        return s


    def commandLineName(self):
        return self.provider.getName().lower() + ":" + self.name.lower().replace(" ", "").replace(",","")


    def getOutputFromName(self, name):
        for out in self.outputs:
            if out.name == name:
                return out

    def getParameterFromName(self, name):
        for param in self.parameters:
            if param.name == name:
                return param

    def getParameterValue(self, name):
        for param in self.parameters:
            if param.name == name:
                #===============================================================
                # if isinstance(param, ParameterNumber):
                #    return float(param.value)
                # elif isinstance(param, ParameterBoolean):
                #    return param.value == str(True)
                # else:
                #===============================================================
                return param.value
        return None

    def getOutputValue(self, name):
        for out in self.outputs:
            if out.name == name:
                return out.value
        return None


    def getAsCommand(self):
        s="Sextante.runalg(\"" + self.commandLineName() + "\","
        for param in self.parameters:
            s+=param.getValueAsCommandLineParameter() + ","
        for out in self.outputs:
            s+=out.getValueAsCommandLineParameter() + ","
        s= s[:-1] + ")"
        return s
