from sextante.parameters.Parameter import Parameter

class ParameterFixedTable(Parameter):

    def __init__(self, name="", description="", cols=1, numRows=3, fixedNumOfRows = False):
        self.cols = cols
        self.numRows = numRows
        self.fixedNumOfRows = False
        self.name = name
        self.description = description
        self.value = None

    def setValue(self, obj):
        if isinstance(obj, str):
            self.value = obj
        else:
            self.value = ParameterFixedTable.tableToString(obj)

    @staticmethod
    def tableToString(table):
        tablestring = ""
        for i in range(len[table]):
            for j in range(len(table[0])):
                tablestring = tablestring + table[i][j] + ","
        tablestring = tablestring[:-1]
        return tablestring



