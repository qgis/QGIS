from sextante.parameters.Parameter import Parameter

class ParameterDataObject(Parameter):

    def getValueAsCommandLineParameter(self):
        if self.value == None:
            return str(None)
        else:
            return "\"" + str(self.value) + "\""