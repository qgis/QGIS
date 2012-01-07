from parameters.Parameter import Parameter

class ParameterSelection(Parameter):

    @property
    def options(self):
        return self._options

    @options.setter
    def options(self, options):
        self._options = options



