from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputRaster import OutputRaster
from sextante.outputs.OutputVector import OutputVector
from sextante.outputs.OutputTable import OutputTable
from sextante.core.SextanteResults import SextanteResults
from sextante.gui.ResultsDialog import ResultsDialog
class SextantePostprocessing:

    @staticmethod
    def handleAlgorithmResults(alg):
        showResults = False;
        for out in alg.outputs:
            if isinstance(out, (OutputRaster, OutputVector)):
                if isinstance(out, OutputVector):
                    if out.hidden:
                        continue
                QGisLayers.load(out.value, out.description, alg.crs)
            elif isinstance(out, OutputTable):
                pass #TODO*****
            else:
                SextanteResults.addResult(out.description, out.value)
                showResults = True
        if showResults:
            dlg = ResultsDialog()
            dlg.exec_()
