from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputRaster import OutputRaster
from sextante.outputs.OutputVector import OutputVector
from sextante.outputs.OutputTable import OutputTable
from sextante.core.SextanteResults import SextanteResults
from sextante.gui.ResultsDialog import ResultsDialog
from sextante.gui.RenderingStyles import RenderingStyles
from sextante.outputs.OutputHTML import OutputHTML
class SextantePostprocessing:

    @staticmethod
    def handleAlgorithmResults(alg):
        showResults = False;
        for out in alg.outputs:
            if out.hidden:
                continue
            if isinstance(out, (OutputRaster, OutputVector)):
                QGisLayers.load(out.value, out.description, alg.crs, RenderingStyles.getStyle(alg.commandLineName(),out.name))
            elif isinstance(out, OutputTable):
                pass #TODO*****
            elif isinstance(out, OutputHTML):
                SextanteResults.addResult(out.description, out.value)
                showResults = True
        if showResults:
            dlg = ResultsDialog()
            dlg.exec_()
