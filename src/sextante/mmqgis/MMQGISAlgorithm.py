from sextante.script.ScriptAlgorithm import ScriptAlgorithm
from PyQt4 import QtGui

class MMQGISAlgorithm(ScriptAlgorithm):

    def getIcon(self):
        filename = self.descriptionFile[:-2] + "png"
        return QtGui.QIcon(filename)