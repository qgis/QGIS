from parameters.ParameterDataObject import ParameterDataObject

class ParameterVector(ParameterDataObject):

    VECTOR_TYPE_POINT = 0
    VECTOR_TYPE_LINE = 1
    VECTOR_TYPE_POLYGON = 2
    VECTOR_TYPE_ANY = 3


    @property
    def shapetype(self):
        return self._type

    @shapetype.setter
    def shapetype(self, shapetype):
        self._type = type
