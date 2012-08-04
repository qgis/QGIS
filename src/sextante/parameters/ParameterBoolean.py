from sextante.parameters.Parameter import Parameter

class ParameterBoolean(Parameter):

    def __init__(self, name="", description="", default=True):
        Parameter.__init__(self, name, description)
        self.default = default
        self.value = None

    def setValue(self, value):
        if value is None:
            self.value = self.default
            return True
        self.value = str(value) == str(True)
        return True

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description + "|" + str(self.default)

    def deserialize(self, s):
        tokens = s.split("|")
        return ParameterBoolean (tokens[0], tokens[1], tokens[2] == str(True))

    def getAsScriptCode(self):
        return "##" + self.name + "=boolean " + str(self.default)