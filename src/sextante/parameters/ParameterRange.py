from sextante.parameters.Parameter import Parameter

class ParameterRange(Parameter):

    def setValue(self, text):
        tokens = text.split(",")
        if len(tokens)!= 2:
            return False
        try:
            n1 = float(tokens[0])
            n2 = float(tokens[1])
            self.value=text
        except:
            return False

    def getValueAsCommandLineParameter(self):
        return "\"" + str(self.value) + "\""