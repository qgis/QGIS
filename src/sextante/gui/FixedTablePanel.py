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
        for i in range(param.numRows):
            self.table.append(list())
            for j in range(len(param.cols)):
                self.table[i].append("0")
        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.label = QtGui.QLabel()
        self.label.setText("Fixed table " + str(len(param.cols)) + " X " + str(param.numRows))
        self.label.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        self.horizontalLayout.addWidget(self.label)
        self.pushButton = QtGui.QPushButton()
        self.pushButton.setText("...")
        self.pushButton.clicked.connect(self.showFixedTableDialog)
        self.horizontalLayout.addWidget(self.pushButton)
        self.setLayout(self.horizontalLayout)

    def showFixedTableDialog(self):
        dlg = FixedTableDialog(self.param, self.table)
        dlg.exec_()
        if dlg.rettable != None:
            self.table = dlg.rettable
