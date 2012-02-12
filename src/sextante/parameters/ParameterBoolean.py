from sextante.parameters.Parameter import Parameter

class ParameterBoolean(Parameter):

    def __init__(self, name, description, default=True):
        self.name = name
        self.description = description
        self.default = default
