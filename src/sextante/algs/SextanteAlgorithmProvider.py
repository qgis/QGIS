from sextante.core.AlgorithmProvider import AlgorithmProvider
from sextante.algs.AddTableField import AddTableField
from PyQt4 import QtGui
import os
from sextante.algs.FieldsCalculator import FieldsCalculator
from sextante.algs.SaveSelectedFeatures import SaveSelectedFeatures
from sextante.algs.Explode import Explode
from sextante.algs.AutoincrementalField import AutoincrementalField

class SextanteAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.alglist = [AddTableField(), FieldsCalculator(), SaveSelectedFeatures(),
                        AutoincrementalField(), Explode()]

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)


    def unload(self):
        AlgorithmProvider.unload(self)


    def getName(self):
        return "sextante"

    def getDescription(self):
        return "SEXTANTE geoalgorithms"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/toolbox.png")

    def _loadAlgorithms(self):
        self.algs = self.alglist
