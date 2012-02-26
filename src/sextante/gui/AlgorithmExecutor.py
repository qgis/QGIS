from PyQt4.QtGui import *
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException

class AlgorithmExecutor:

    @staticmethod
    def runalg(alg, progress):
        try:
            alg.execute(progress)
        except GeoAlgorithmExecutionException, e :
            QMessageBox.critical(None, "Error", e.msg)
        finally:
            progress.setFinished()


    @staticmethod
    def runbatch(algs, progress):
        try:
            for alg in algs:
                progress.addText(alg.getAsCommand())
                AlgorithmExecutor.runalg(alg, SilentProgress())
                progress.addText("Execution OK!")
        except GeoAlgorithmExecutionException, e :
            QMessageBox.critical(self, "Error",e.msg)
            progress.addText("Execution Failed")
        finally:
            progress.setFinished()

class SilentProgress():

    def setText(self, text):
        pass

    def setPercentage(self, i):
        pass

    def setFinished(self):
        pass