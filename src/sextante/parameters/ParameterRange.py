from sextante.parameters.Parameter import Parameter

class ParameterRange(Parameter):

    def __init__(self, name="", description="", default="0,1"):
        self.name = name
        self.description = description
        self.default = default
        self.value = None

    def setValue(self, text):
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