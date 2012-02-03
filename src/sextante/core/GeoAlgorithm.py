from sextante.outputs.Output import Output
from sextante.parameters.Parameter import Parameter
from sextante.core.QGisLayers import QGisLayers
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.parameters.ParameterVector import ParameterVector


class GeoAlgorithm:

    def __init__(self):
        self.parameters = list()
        self.outputs = list()
        self.name = ""
        self.group = ""
        self.defineCharacteristics()
        self.providerName = ""
        self.crs = None

    def execute(self, progress):
        self.setOutputCRSFromInputLayers()
        self.processAlgorithm(progress)

    def setOutputCRSFromInputLayers(self):
        for param in self.parameters:
            if isinstance(param, ParameterRaster):
                layers = QGisLayers.getRasterLayers()
            elif isinstance(param, ParameterVector):
                layers = QGisLayers.getVectorLayers()
            else:
                continue
            for layer in layers:
                if layer.dataProvider().dataSourceUri() == param.value:
                    self.crs = layer.crs()
                    return


    def defineCharacteristics(self):
        pass

    def processAlgorithm(self):
        pass

    def putOutput(self, output):
        #TODO: check that name does not exist
        if isinstance(output, Output):
            self.outputs.append(output)

    def putParameter(self, param):
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

    def canBeExecuted(self, layersCount):
        return True

    def __str__(self):
        s = "ALGORITHM: " + self.name + "\n"
        #s+=self._descriptionFile + "\n"
        for param in self.parameters:
            s+=(str(param) + "\n")
        for out in self.outputs:
            s+=(str(out) + "\n")
        s+=("\n")
        return s


    def commandLineName(self):
        return self.providerName + self.name.lower().replace(" ", "")


    def getOuputsChannelsAsMap(self):
        retmap = {}
        for out in self.outputs:
            retmap[out.name] = out.channel
        return retmap

    def getAsCommand(self):
        s="Sextante.runalg(\"" + self.commandLineName() + "\","
        for param in self.parameters:
            s+=param.getValueAsCommandLineParameter() + ","
        for out in self.outputs:
            s+=out.getChannelAsCommandLineParameter() + ","
        s= s[:-1] + ")"
        return s
