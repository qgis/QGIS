from parameters.Parameter import Parameter

class ParameterTableField(Parameter):

    @property
    def parent(self):
        return self._parent

    @parent.setter
    def parent(self, parent):
        self._parent = parent

