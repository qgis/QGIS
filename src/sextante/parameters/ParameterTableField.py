from sextante.parameters.Parameter import Parameter

class ParameterTableField(Parameter):

    DATA_TYPE_NUMBER = 0
    DATA_TYPE_STRING = 1
    DATA_TYPE_ANY = -1

    def __init__(self, name="", description="", parent=None, datatype=-1):
        Parameter.__init__(self, name, description)
        self.parent = parent
        self.value = None
        self.datatype = datatype

    def getValueAsCommandLineParameter(self):
        return "\"" + str(self.value) + "\""

    def getAsScriptCode(self):
        return "##" + self.name + "=field " + str(self.parent)

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                "|" + str(self.parent) + "|" + str(self.datatype)


    def deserialize(self, s):
        tokens = s.split("|")
        if len(tokens) == 4:
            return ParameterTableField(tokens[0], tokens[1], tokens[2], int(tokens[3]))
        else:
            return ParameterTableField(tokens[0], tokens[1], tokens[2])

    def __str__(self):
        return self.name + " <" + self.__module__.split(".")[-1] +" from " + self.parent     + ">"
