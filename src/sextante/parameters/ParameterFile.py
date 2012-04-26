from sextante.parameters.Parameter import Parameter

class ParameterFile(Parameter):

    def __init__(self, name="", description="", isFolder = False):
        Parameter.__init__(self, name, description)
        self.value = None
        self.isFolder = isFolder

    def getValueAsCommandLineParameter(self):
        return "\"" + str(self.value) + "\""

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                        "|" + str(self.isFolder)

    def deserialize(self, s):
        tokens = s.split("|")
        return ParameterFile(tokens[0], tokens[1], tokens[2] == str(True))

    def getAsScriptCode(self):
        if self.isFolder:
            return "##" + self.name + "=folder"
        else:
            return "##" + self.name + "=file"