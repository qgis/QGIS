from sextante.parameters.Parameter import Parameter

class ParameterExtent(Parameter):

    def __init__(self, name="", description="", default="0,1,0,1"):
        Parameter.__init__(self, name, description)
        self.default = default
        self.value = None #The value is a string in the form "xmin, xmax, ymin, y max"

    def setValue(self, text):
        if text is None:
            self.value = self.default
            return True
        tokens = text.split(",")
        if len(tokens)!= 4:
            return False
        try:
            n1 = float(tokens[0])
            n2 = float(tokens[1])
            n3 = float(tokens[2])
            n4 = float(tokens[3])
            self.value=text
            return True
        except:
            return False

    def getValueAsCommandLineParameter(self):
        return "\"" + str(self.value) + "\""

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                        "|" + str(self.default)

    def deserialize(self, s):
        tokens = s.split("|")
        return ParameterExtent(tokens[0], tokens[1], tokens[2])

