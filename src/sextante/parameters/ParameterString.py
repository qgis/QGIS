from sextante.parameters.Parameter import Parameter

class ParameterString(Parameter):

    def __init__(self, name="", description="", default=""):
        self.name = name
        self.description = description
        self.default = default
        self.value = None

    def getValueAsCommandLineParameter(self):
        return "\"" + str(self.value) + "\""

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                        "|" + str(self.default)

    def deserialize(self, s):
        tokens = s.split("|")
        return ParameterString(tokens[0], tokens[1], tokens[2])

    def getAsScriptCode(self):
        return "##" + self.name + "=string " + self.default