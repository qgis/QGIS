from sextante.parameters.Parameter import Parameter

class ParameterString(Parameter):

    def __init__(self, name="", description="", default=""):
        Parameter.__init__(self, name, description)
        self.default = default
        self.value = None

    def setValue(self, obj):
        if obj is None:
            self.value = self.default
            return True
        self.value = str(obj)
        return True

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