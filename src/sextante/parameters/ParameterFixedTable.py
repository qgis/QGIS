from sextante.parameters.Parameter import Parameter

class ParameterFixedTable(Parameter):

    def __init__(self, name, description, cols, numRows, fixedNumOfRows = False):
        self.cols = cols
        self.numRows = 3
        self.fixedNumOfRows = False
        self.name = name
        self.description = description

    def setValue(self, obj):
        if isinstance(obj, str):
            self.value = obj
        else:
            tablestring = ""
            table = obj
            for i in range(len[table]):
                for j in range(len(table[0])):
                    tablestring = tablestring + table[i][j] + ","
            tablestring = tablestring[:-1]
            self.value = tablestring