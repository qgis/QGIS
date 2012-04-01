from PyQt4 import QtGui
from sextante.gui.NumberInputDialog import NumberInputDialog

class NumberInputPanel(QtGui.QWidget):

    def __init__(self, number, parent = None):
        super(NumberInputPanel, self).__init__(parent)
        self.setObjectName("NIPanel")
        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.setObjectName("hLayout")
        self.text = QtGui.QLineEdit()
        self.text.setObjectName("linedit")
        self.text.setText(str(number))
        self.text.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        self.horizontalLayout.addWidget(self.text)
        self.pushButton = QtGui.QPushButton()
        self.pushButton.setObjectName("pushButton")
        self.pushButton.setText("...")
        self.pushButton.clicked.connect(self.showNumberInputDialog)
        self.horizontalLayout.addWidget(self.pushButton)
        self.setLayout(self.horizontalLayout)

    def showNumberInputDialog(self):
        pass
        dlg = NumberInputDialog()
        dlg.exec_()
        if dlg.value != None:
            self.text.setText(str(dlg.value))

    def getValue(self):
        return self.text.text()