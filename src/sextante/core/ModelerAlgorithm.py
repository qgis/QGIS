from sextante.core.GeoAlgorithm import GeoAlgorithm

class ModelerAlgorithm(GeoAlgorithm):

    algs = []
    algParameters = []
    paramValues = {}

    def __init__(self):
        GeoAlgorithm.__init__(self)
        self.providerName = "model:"

    def openModel(self, filename):
        pass

    def addAlgorithm(self, algName, parametersMap):
        self.algs.append(algName)
        self.parameters.append(parametersMap)

    def addParameterValue(self, value):
        name = "HARD_CODE_PARAM_VALUE" + str(len(self.paramValues))
        self.paramValues[name] = value
        return name

    def __str__(self):
        s=""
        for i in range(len(self.algs)):
            s+="ALGORITHM:" + self.algs[i]+"\n"
            for param in self.algParameters[i]:
                s+=str(param.value)
        for param in self.parameters:
            s += "PARAMETER:" + param.serialize() + "\n"
        for key in self.paramValues.keys():
            s += "VALUE:" + key + "=" + str(self.paramValues[key]) + "\n"

        return s