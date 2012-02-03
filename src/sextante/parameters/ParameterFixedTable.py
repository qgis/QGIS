from sextante.parameters.Parameter import Parameter

class ParameterFixedTable(Parameter):


    def setValue(self, obj):
        tablestring = ""
        table = obj
        for i in range(len[table]):
            for j in range(len(table[0])):
                tablestring = tablestring + table[i][j] + ","
        tablestring = tablestring[:-1]
        self.value = tablestring