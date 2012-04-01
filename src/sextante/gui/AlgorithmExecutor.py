from PyQt4.QtGui import *
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException

class AlgorithmExecutor:

    @staticmethod
    def runalg(alg, progress):
        '''executes a given algorithm, showing its progress in the progress object passed along.
        Return true if everything went OK, false if the algorithm was canceled or there was
        any problem and could not be completed'''
        try:
            alg.execute(progress)
            return not alg.canceled
        except GeoAlgorithmExecutionException, e :
            QMessageBox.critical(None, "Error", e.msg)
            return False
        finally:
            progress.setFinished()

class SilentProgress():

    def setText(self, text):
        pass

    def setPercentage(self, i):
        pass

    def setFinished(self):
        pass