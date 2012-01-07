from parameters.ParameterDataObject import ParameterDataObject

class ParameterMultipleInput(ParameterDataObject):

    TYPE_VECTOR_POINT = 0
    TYPE_VECTOR_LINE = 1
    TYPE_VECTOR_POLYGON = 2
    TYPE_VECTOR_ANY = 3
    TYPE_RASTER = 4
    TYPE_TABLE = 5


    @property
    def datatype(self):
        return self._type

    @datatype.setter
    def datatype(self, shapetype):
        self._type = type