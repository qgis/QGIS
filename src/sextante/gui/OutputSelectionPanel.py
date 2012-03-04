from PyQt4 import QtGui, QtCore
import os.path
from sextante.core.SextanteConfig import SextanteConfig


class OutputSelectionPanel(QtGui.QWidget):

    SAVE_TO_TEMP_FILE = "[Save to temporary file]"

    def __init__(self, output, alg):
        self.output = output
        self.alg = alg
        super(OutputSelectionPanel, self).__init__(None)
        self.setObjectName("OSPanel")
        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.setObjectName("hLayout")
        self.text = QtGui.QLineEdit()
        self.text.setObjectName("label")
        self.text.setText(OutputSelectionPanel.SAVE_TO_TEMP_FILE)
        self.text.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        self.horizontalLayout.addWidget(self.text)
        self.pushButton = QtGui.QPushButton()
        self.pushButton.setObjectName("pushButton")
        self.pushButton.setText("...")
        self.pushButton.clicked.connect(self.showSelectionDialog)
        self.horizontalLayout.addWidget(self.pushButton)
        self.setLayout(self.horizontalLayout)

    def showSelectionDialog(self):
        filefilter = self.output.getFileFilter(self.alg)
        filename = QtGui.QFileDialog.getSaveFileName(self, "Save file", QtCore.QString(), filefilter)
        if filename:
            self.text.setText(str(filename))

    def getValue(self):
        filename = str(self.text.text())
        if filename.strip() == "" or filename == OutputSelectionPanel.SAVE_TO_TEMP_FILE:
            return None
        else:
            if not os.path.isabs(filename):
                filename = SextanteConfig.getSetting(SextanteConfig.OUTPUT_FOLDER)
            return filename
