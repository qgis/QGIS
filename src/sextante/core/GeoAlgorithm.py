from sextante.outputs.Output import Output
from sextante.parameters.Parameter import Parameter

class GeoAlgorithm:

    def __init__(self):
        self.parameters = list()
        self.outputs = list()
        self.name = ""
        self.group = ""
        self.defineCharacteristics()
        self.providerName = ""

    def execute(self):
        if self.checkParameters():
            self.proccessAlgorithm()


    def checkParameters(self):
        #TODO!!!!!!
        return True

    def defineCharacteristics(self):
        pass

    def processAlgorithm(self):
        pass

    def putOutput(self, output):
        if isinstance(output, Output):
            self.outputs.append(output)

    def putParameter(self, param):
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
        s+=self._descriptionfile + "\n"
        for param in self.parameters.values():
            s+=(str(param) + "\n")
        for out in self.outputs.values():
            s+=(str(out) + "\n")
        s+=("\n")
        return s

    def commandLineName(self):
        return self.providerName + self.name.lower().replace(" ", "")


