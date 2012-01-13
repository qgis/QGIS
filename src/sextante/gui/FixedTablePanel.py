from PyQt4 import QtCore, QtGui
from sextante.gui.FixedTableDialog import FixedTableDialog

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class FixedTablePanel(QtGui.QWidget):

    def __init__(self, param, parent = None):
        super(FixedTablePanel, self).__init__(parent)
        self.param = param
        self.table = []
        self.setObjectName(_fromUtf8("MSPanel"))
        self.contents = QtGui.QWidget(self)
        self.contents.setObjectName(_fromUtf8("contents"))
        self.horizontalLayout = QtGui.QHBoxLayout(self.contents)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.setObjectName(_fromUtf8("hLayout"))
        self.label = QtGui.QLabel(self.contents)
        self.label.setObjectName(_fromUtf8("label"))
        self.label.setText("Fixed table " + str(len(param.cols)) + " X " + str(param.numRows))
        self.horizontalLayout.addWidget(self.label)
        self.pushButton = QtGui.QPushButton(self.contents)
        self.pushButton.setObjectName(_fromUtf8("pushButton"))
        self.pushButton.setText("...")
        self.pushButton.clicked.connect(self.showFixedTableDialog)
        self.horizontalLayout.addWidget(self.pushButton)

    def showFixedTableDialog(self):
        dlg = FixedTableDialog(self.param, self.table)
        dlg.exec_()
        if dlg.table != None:
            self.table = dlg.table
