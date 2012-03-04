from sextante.parameters.Parameter import Parameter
from PyQt4 import QtGui

class ParameterSelection(Parameter):

    def __init__(self, name="", description="", options=[]):
        self.name = name
        self.description = description
        self.options = options
        self.value = None

    def getAsScriptCode(self):
        return "##" + self.name + "=selection " + ";".join(self.options)

    def deserialize(self, s):
        tokens = s.split("|")
        return ParameterSelection(tokens[0], tokens[1], tokens[2].split(";"))

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                        "|" + ";".join(self.options)