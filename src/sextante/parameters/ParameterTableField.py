from sextante.parameters.Parameter import Parameter

class ParameterTableField(Parameter):

    def __init__(self, name="", description="", parent=None):
        Parameter.__init__(self, name, description)
        self.parent = parent
        self.value = None

    def getValueAsCommandLineParameter(self):
        return "\"" + str(self.value) + "\""

    def getAsScriptCode(self):
        return "##" + self.name + "=field " + str(self.parent)

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                        "|" + str(self.parent)

    def deserialize(self, s):
        tokens = s.split("|")
        return ParameterTableField(tokens[0], tokens[1], tokens[2])

    def __str__(self):
        return self.name + " <" + self.__module__.split(".")[-1] +" from " + self.value + ">"
