from sextante.outputs.Output import Output
from sextante.parameters.Parameter import Parameter

class GeoAlgorithm:

    def __init__(self):
        self.parameters = {}
        self.outputs = {}
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
#===============================================================================
#
#    @property
#    def outputs(self):
#        return self._outputs
#
#    @property
#    def parameters(self):
#        return self._parameters
#
#    @property
#    def group(self):
#        return self._group
#
#    @group.setter
#    def group(self, g):
#        self._group = g
#
#    @property
#    def name(self):
#        return self._name
#
#    @name.setter
#    def name(self, name):
#        self._name = name
#===============================================================================

    def putOutput(self, output):
        if isinstance(output, Output):
            self.outputs[output.name] = output


    def putParameter(self, param):
        if isinstance(param, Parameter):
            self.parameters[param.name] = param

    def setParameterValue(self, paramName, value):
        param = self.parameters[paramName]
        if param != None:
            param.value = value

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


