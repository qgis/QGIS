from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputRaster import OutputRaster
from sextante.outputs.OutputVector import OutputVector
from sextante.outputs.OutputTable import OutputTable
from sextante.core.SextanteResults import SextanteResults
from sextante.gui.ResultsDialog import ResultsDialog
from sextante.gui.RenderingStyles import RenderingStyles
from sextante.outputs.OutputHTML import OutputHTML
from PyQt4.QtGui import *
from sextante.core.SextanteConfig import SextanteConfig
import os
class SextantePostprocessing:

    @staticmethod
    def handleAlgorithmResults(alg, showResults = True):
        htmlResults = False;
        for out in alg.outputs:
            if out.hidden or not out.open:
                continue
            if isinstance(out, (OutputRaster, OutputVector, OutputTable)):
                try:
                    if SextanteConfig.getSetting(SextanteConfig.USE_FILENAME_AS_LAYER_NAME):
                        name = os.path.basename(out.value)
                    else:
                        name = out.description
                    QGisLayers.load(out.value, name, alg.crs, RenderingStyles.getStyle(alg.commandLineName(),out.name))
                except Exception, e:
                    QMessageBox.critical(None, "Error", str(e))
            elif isinstance(out, OutputHTML):
                SextanteResults.addResult(out.description, out.value)
                htmlResults = True
        if showResults and htmlResults:
            QApplication.restoreOverrideCursor()
            dlg = ResultsDialog()
            dlg.exec_()
