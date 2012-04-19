from sextante.outputs.OutputHTML import OutputHTML
from sextante.outputs.OutputRaster import OutputRaster
from sextante.outputs.OutputTable import OutputTable
from sextante.outputs.OutputVector import OutputVector
from sextante.outputs.OutputNumber import OutputNumber
from sextante.outputs.OutputFile import OutputFile

class OutputFactory():

    @staticmethod
    def getFromString(s):
        classes = [OutputRaster, OutputVector, OutputTable, OutputHTML, OutputNumber, OutputFile]
        for clazz in classes:
            if s.startswith(clazz().outputTypeName()):
                tokens = s[len(clazz().outputTypeName())+1:].split("|")
                if len(tokens) == 2:
                    return clazz(tokens[0], tokens[1])
                else:
                    return clazz(tokens[0], tokens[1], tokens[2]==str(True))
