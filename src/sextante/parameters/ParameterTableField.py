from sextante.parameters.Parameter import Parameter

class ParameterTableField(Parameter):

    def __init__(self, name, description, parent):
        self.name = name
        self.description = description
        self.parent = parent

    def __str__(self):
        return "\"" + str(self.value) + "\""