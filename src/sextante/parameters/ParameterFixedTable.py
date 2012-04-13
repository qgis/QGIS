from sextante.parameters.Parameter import Parameter
from PyQt4 import QtGui

class ParameterFixedTable(Parameter):

    def __init__(self, name="", description="", cols=["value"], numRows=3, fixedNumOfRows = False):
        self.cols = cols
        self.numRows = numRows
        self.fixedNumOfRows = fixedNumOfRows
        self.name = name
        self.description = description
        self.value = None

    def setValue(self, obj):
        ##TODO: check that it contains a correct number of elements
        if isinstance(obj, str):
            self.value = obj
        else:
            self.value = ParameterFixedTable.tableToString(obj)
        return True

    @staticmethod
    def tableToString(table):
        tablestring = ""
        for i in range(len[table]):
            for j in range(len(table[0])):
                tablestring = tablestring + table[i][j] + ","
        tablestring = tablestring[:-1]
        return tablestring

    def deserialize(self, s):
        tokens = s.split("|")
        return ParameterFixedTable(tokens[0], tokens[1], tokens[3].split(";"), int(tokens[2]), tokens[4] == str(True))

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                        "|" + str(self.numRows) + "|" + ";".join(self.cols) + "|" +  str(self.fixedNumOfRows)

