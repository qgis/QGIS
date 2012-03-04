from sextante.outputs.OutputHTML import OutputHTML
from sextante.outputs.OutputRaster import OutputRaster
from sextante.outputs.OutputTable import OutputTable
from sextante.outputs.OutputVector import OutputVector

class OutputFactory():

    @staticmethod
    def getFromString(s):
        classes = [OutputRaster, OutputVector, OutputTable, OutputHTML]
        for clazz in classes:
            if s.startswith(clazz().outputTypeName()):
                tokens = s[len(clazz().outputTypeName())+1:].split("|")
                return clazz(tokens[0], tokens[1])