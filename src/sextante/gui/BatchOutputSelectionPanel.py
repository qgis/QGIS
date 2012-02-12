from PyQt4 import QtGui, QtCore
from sextante.gui.AutofillDialog import  AutofillDialog
from sextante.parameters.ParameterMultipleInput import ParameterMultipleInput
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.parameters.ParameterTable import ParameterTable
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.parameters.ParameterSelection import ParameterSelection
from sextante.parameters.ParameterFixedTable import ParameterFixedTable
import os.path


class BatchOutputSelectionPanel(QtGui.QWidget):

    def __init__(self, alg, row, col, batchDialog, parent = None):
        super(BatchOutputSelectionPanel, self).__init__(parent)
        self.alg = alg
        self.row = row
        self.col = col
        self.batchDialog = batchDialog
        self.table = batchDialog.table
        self.setObjectName("OSPanel")
        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.setObjectName("hLayout")
        self.text = QtGui.QLineEdit()
        self.text.setObjectName("label")
        self.text.setText("")
        self.text.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        self.horizontalLayout.addWidget(self.text)
        self.pushButton = QtGui.QPushButton()
        self.pushButton.setObjectName("pushButton")
        self.pushButton.setText("...")
        self.pushButton.clicked.connect(self.showSelectionDialog)
        self.horizontalLayout.addWidget(self.pushButton)
        self.setLayout(self.horizontalLayout)

    def showSelectionDialog(self):
        filename = QtGui.QFileDialog.getSaveFileName(self, "Save file", QtCore.QString(), "All files (*.*)")
        if filename:
            filename = str(filename)
            dlg = AutofillDialog(self.alg)
            dlg.exec_()
            if dlg.mode != None:
                try:
                    if dlg.mode == AutofillDialog.DO_NOT_AUTOFILL:
                        self.text.setChannel(filename)
                    elif dlg.mode == AutofillDialog.FILL_WITH_NUMBERS:
                        n = self.table.rowCount() - self.row
                        for i in range(n):
                            name = filename[:filename.rfind(".")] + str(i+1) + filename[filename.rfind("."):]
                            self.table.cellWidget(i + self.row, self.col).setChannel(name)
                    elif dlg.mode == AutofillDialog.FILL_WITH_PARAMETER:
                        n = self.table.rowCount() - self.row
                        for i in range(n):
                            widget = self.table.cellWidget(i+self.row, dlg.param)
                            param = self.alg.parameters[dlg.param]
                            if isinstance(param, (ParameterRaster, ParameterVector, ParameterTable, ParameterMultipleInput)):
                                s = str(widget.getText())
                                s = os.path.basename(s)
                                s= s[:s.rfind(".")]
                            elif isinstance(param, ParameterBoolean):
                                s = str(widget.currentIndex() == 0)
                            elif isinstance(param, ParameterSelection):
                                s = str(widget.currentText())
                            elif isinstance(param, ParameterFixedTable):
                                s = str(widget.table)
                            else:
                                s = str(widget.text())
                            name = filename[:filename.rfind(".")] + s + filename[filename.rfind("."):]
                            self.table.cellWidget(i + self.row, self.col).setChannel(name)
                except:
                    pass
    def setChannel(self, text):
        return self.text.setText(text)

    def getChannel(self):
        return str(self.text.text())

