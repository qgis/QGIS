from sextante.parameters.Parameter import Parameter

class ParameterNumber(Parameter):

    def __init__(self, name, description, minValue = None, maxValue = None, default = 0):
        self.name = name
        self.description = description
        self.default = default
        self.min = minValue
        self.max = maxValue