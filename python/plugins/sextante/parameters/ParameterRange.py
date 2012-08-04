from sextante.parameters.Parameter import Parameter

class ParameterRange(Parameter):

    def __init__(self, name="", description="", default="0,1"):
        Parameter.__init__(self, name, description)
        self.default = default
        self.value = None

    def setValue(self, text):
        if text is None:
            self.value = self.default
            return True
        tokens = text.split(",")
        if len(tokens)!= 2:
            return False
        try:
            n1 = float(tokens[0])
            n2 = float(tokens[1])
            self.value=text
            return True
        except:
            return False

    def getValueAsCommandLineParameter(self):
        return "\"" + str(self.value) + "\""

    def deserialize(self, s):
        tokens = s.split("|")
        if len(tokens) == 3:
            return ParameterRange(tokens[0], tokens[1], tokens[2])
        else:
            return ParameterRange(tokens[0], tokens[1])