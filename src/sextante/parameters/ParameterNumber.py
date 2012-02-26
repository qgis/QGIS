from sextante.parameters.Parameter import Parameter

class ParameterNumber(Parameter):

    def __init__(self, name="", description="", minValue = None, maxValue = None, default = 0):
        self.name = name
        self.description = description
        self.default = default
        self.min = minValue
        self.max = maxValue
        self.value = None

    def setValue(self, n):
        try:
            self.value = float(n)
            return True
        except:
            return False

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                        "|" + str(self.min) + "|" + str(self.max)  + "|" + str(self.default)

    def deserialize(self, s):
        tokens = s.split("|")
        return ParameterNumber(tokens[0], tokens[1], float(tokens[2]), float(tokens[3]), float(tokens[4]))

    def getAsScriptCode(self):
        return "##" + self.name + "=number " + self.default