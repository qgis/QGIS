from parameters.Parameter import Parameter

class ParameterDataObject(Parameter):

    @property
    def optional(self):
        return self._optional

    @optional.setter
    def optional(self, optional):
        self._optional = optional

