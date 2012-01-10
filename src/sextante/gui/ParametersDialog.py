
from PyQt4 import QtGui

from sextante.gui.ui_ParametersDialog import Ui_ParametersDialog

class ParametersDialog(QtGui.QDialog):
    def __init__(self, alg):
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.alg = alg
        self.ui = Ui_ParametersDialog()
        self.ui.setupUi(self)

