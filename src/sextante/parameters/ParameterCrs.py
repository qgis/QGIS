from sextante.parameters.Parameter import Parameter

class ParameterCrs(Parameter):

    def __init__(self, name="", description="", default = "4326"):
        '''The value is the EPSG code of the CRS'''
        Parameter.__init__(self, name, description)
        self.value = None
        self.default = default

    def setValue(self, value):
        if value is None:
            self.value = self.default
            return True
        self.value = str(value)
        return True

    def getValueAsCommandLineParameter(self):
        return "\"" + str(self.value) + "\""

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                        "|" + str(self.default)

    def deserialize(self, s):
        tokens = s.split("|")
        return ParameterCrs(tokens[0], tokens[1], tokens[2])

    def getAsScriptCode(self):
        return "##" + self.name + "=crs " + str(self.default)
