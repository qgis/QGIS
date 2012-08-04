from sextante.script.ScriptAlgorithm import ScriptAlgorithm
from PyQt4 import QtGui
import os

class GdalAlgorithm(ScriptAlgorithm):
    '''Just a ScriptAlgorithm that automatically takes its icon
    filename for the script filename'''

    def getIcon(self):
        filename = os.path.basename(self.descriptionFile[:-2] + "png")
        filepath = os.path.dirname(__file__) + "/icons/" + filename
        return QtGui.QIcon(filepath)