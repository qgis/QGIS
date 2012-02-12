from sextante.parameters.Parameter import Parameter

class ParameterString(Parameter):

    def __init__(self, name, description, default=""):
        self.name = name
        self.description = description
        self.default = default

    def getValueAsCommandLineParameter(self):
        return "\"" + str(self.value) + "\""