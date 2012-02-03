from sextante.parameters.Parameter import Parameter

class ParameterString(Parameter):

    def __str__(self):
        return "\"" + str(self.value) + "\""