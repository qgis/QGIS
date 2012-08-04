from sextante.parameters.Parameter import Parameter

class ParameterSelection(Parameter):

    def __init__(self, name="", description="", options=[], default = 0):
        Parameter.__init__(self, name, description)
        self.options = options
        self.value = None
        self.default = default

    def setValue(self, n):
        if n is None:
            self.value = self.default
            return True
        try:
            n = int(n)
            self.value = n
            return True
        except:
            return False

    def getAsScriptCode(self):
        return "##" + self.name + "=selection " + ";".join(self.options)

    def deserialize(self, s):
        tokens = s.split("|")
        if len(tokens) == 4:
            return ParameterSelection(tokens[0], tokens[1], tokens[2].split(";"), int(tokens[3]))
        else:
            return ParameterSelection(tokens[0], tokens[1], tokens[2].split(";"))

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                        "|" + ";".join(self.options)