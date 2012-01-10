
from PyQt4 import QtGui
from sextante.gui.ui_SextanteToolbox import Ui_SextanteToolbox

class SextanteToolbox(QtGui.QDialog):
    def __init__(self):
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.ui = Ui_SextanteToolbox()
        self.ui.setupUi(self)
