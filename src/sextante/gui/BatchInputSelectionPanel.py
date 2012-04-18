from PyQt4 import QtGui, QtCore
from sextante.parameters.ParameterMultipleInput import ParameterMultipleInput


class BatchInputSelectionPanel(QtGui.QWidget):

    def __init__(self, param, row, col, batchDialog, parent = None):
        super(BatchInputSelectionPanel, self).__init__(parent)
        self.param = param
        self.batchDialog = batchDialog
        self.table = batchDialog.table
        self.row = row
        self.col = col
        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.text = QtGui.QLineEdit()
        self.text.setText("")
        self.text.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        self.horizontalLayout.addWidget(self.text)
        self.pushButton = QtGui.QPushButton()
        self.pushButton.setText("...")
        self.pushButton.clicked.connect(self.showSelectionDialog)
        self.horizontalLayout.addWidget(self.pushButton)
        self.setLayout(self.horizontalLayout)

    def showSelectionDialog(self):
        ret = QtGui.QFileDialog.getOpenFileNames(self, "Open file", QtCore.QString(), "All files (*.*)")
        if ret:
            files = list(ret)
            if len(files) == 1:
                self.text.setText(str(files[0]))
            else:
                if isinstance(self.param, ParameterMultipleInput):
                    self.text.setText(";".join(str(f) for f in files))
                else:
                    rowdif = len(files) - (self.table.rowCount() - self.row)
                    for i in range(rowdif):
                        self.batchDialog.addRow()
                    for i in range(len(files)):
                        self.table.cellWidget(i+self.row, self.col).setText(files[i])


    def setText(self, text):
        return self.text.setText(text)


    def getText(self):
        return self.text.text()
