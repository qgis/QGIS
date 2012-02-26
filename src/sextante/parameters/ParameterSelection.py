from sextante.parameters.Parameter import Parameter

class ParameterSelection(Parameter):

    def __init__(self, name="", description="", options=[]):
        self.name = name
        self.description = description
        self.options = options
        self.value = None

    def getAsScriptCode(self):
        return "##" + self.name + "=selection " + ";".join(self.options)