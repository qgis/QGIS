from sextante.parameters.Parameter import Parameter

class ParameterFixedTable(Parameter):

    def __init__(self, name="", description="", cols=["value"], numRows=3, fixedNumOfRows = False):
        Parameter.__init__(self, name, description)
        self.cols = cols
        self.numRows = numRows
        self.fixedNumOfRows = fixedNumOfRows
        self.value = None

    def setValue(self, obj):
        ##TODO: check that it contains a correct number of elements
        if isinstance(obj, str):
            self.value = obj
        else:
            self.value = ParameterFixedTable.tableToString(obj)
        return True

    def getValueAsCommandLineParameter(self):
        return "\"" + str(self.value) + "\""

    @staticmethod
    def tableToString(table):
        tablestring = ""
        for i in range(len(table)):
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

