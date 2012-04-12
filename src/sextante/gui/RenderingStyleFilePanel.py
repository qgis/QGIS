from PyQt4 import QtGui, QtCore
import os.path
from sextante.core.SextanteConfig import SextanteConfig


class RenderingStyleFilePanel(QtGui.QWidget):

    def __init__(self):
        super(RenderingStyleFilePanel, self).__init__(None)
        self.setObjectName("RSFPanel")
        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.setObjectName("hLayout")
        self.text = QtGui.QLineEdit()
        self.text.setObjectName("label")
        self.text.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        self.horizontalLayout.addWidget(self.text)
        self.pushButton = QtGui.QPushButton()
        self.pushButton.setObjectName("pushButton")
        self.pushButton.setText("...")
        self.pushButton.clicked.connect(self.showSelectionDialog)
        self.horizontalLayout.addWidget(self.pushButton)
        self.setLayout(self.horizontalLayout)

    def showSelectionDialog(self):
        filename = QtGui.QFileDialog.getOpenFileName(self, "Select style file", "", "*.qml")
        if filename:
            self.text.setText(str(filename))

    def setText(self, text):
            self.text.setText(str(text))

    def getValue(self):
        filename = str(self.text.text())
        return filename