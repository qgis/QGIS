from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputRaster import OutputRaster
from sextante.outputs.OutputVector import OutputVector
from sextante.outputs.OutputTable import OutputTable
class SextanteResults():

    results = []

    @staticmethod
    def addResult(name, result):
        SextanteResults.results.append(Result(name, result))

    @staticmethod
    def getResults():
        return SextanteResults.results

    @staticmethod
    def handleAlgorithmResults(alg):
        for out in alg.outputs:
            if isinstance(out, (OutputRaster, OutputVector)):
                QGisLayers.load(out.value, out.description, alg.crs)
            elif isinstance(out, OutputTable):
                pass #TODO*****
            else:
                SextanteResults.addResult(out.description, out.value)

class Result():

    def __init__(self, name, value):
        self.name = name
        self.value = value





