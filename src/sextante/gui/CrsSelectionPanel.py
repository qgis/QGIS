from PyQt4 import QtGui, QtCore
from sextante.gui.CrsSelectionDialog import CrsSelectionDialog

class CrsSelectionPanel(QtGui.QWidget):

    def __init__(self, default):
        super(CrsSelectionPanel, self).__init__(None)
        self.epsg = default
        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.text = QtGui.QLineEdit()
        self.text.setEnabled(False)
        self.text.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        self.horizontalLayout.addWidget(self.text)
        self.pushButton = QtGui.QPushButton()
        self.pushButton.setText("...")
        self.pushButton.clicked.connect(self.showSelectionDialog)
        self.horizontalLayout.addWidget(self.pushButton)
        self.setLayout(self.horizontalLayout)
        self.setText()

    def showSelectionDialog(self):
        dialog = CrsSelectionDialog()
        dialog.exec_()
        if dialog.epsg:
            self.epsg = str(dialog.epsg)
            self.setText()

    def setText(self):
        self.text.setText("EPSG:" + str(self.epsg))

    def getValue(self):
        return self.epsg
