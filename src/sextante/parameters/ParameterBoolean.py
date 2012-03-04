from sextante.parameters.Parameter import Parameter

class ParameterBoolean(Parameter):

    def __init__(self, name="", description="", default="True"):
        self.name = name
        self.description = description
        self.default = default
        self.value = None

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description + "|" + str(self.default)

    def deserialize(self, s):
        tokens = s.split("|")
        return ParameterBoolean (tokens[0], tokens[1], tokens[2] == str(True))

    def getAsScriptCode(self):
        return "##" + self.name + "=boolean " + self.default