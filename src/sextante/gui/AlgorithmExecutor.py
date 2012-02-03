from PyQt4 import QtCore
from PyQt4.QtGui import *
import time
from sextante.core.SextanteUtils import SextanteUtils

class AlgorithmExecutor:

    @staticmethod
    def runalg(alg, progress):
        SextanteUtils.addToLog(SextanteUtils.LOG_ALGORITHM, alg.getAsCommand())
        alg.execute(progress)
        #=======================================================================
        # th = RunAlgorithmThread(alg, progress)
        # th.start()
        # th.wait()
        #=======================================================================

class RunAlgorithmThread(QtCore.QThread):

    def __init__(self, alg, progress):
        self.alg = alg
        self.progress = progress
        QtCore.QThread.__init__(self)

    def run(self):
        for i in range(2):
            time.sleep(3)
            self.progress.addText(str(i))
            self.progress.setPercentage(i*50)
        #self.alg.execute(self.progress)

class SilentProgress():

    def addText(self, text):
        pass

    def setPercentage(self, i):
        pass