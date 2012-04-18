from PyQt4 import QtGui
from sextante.gui.NumberInputDialog import NumberInputDialog

class NumberInputPanel(QtGui.QWidget):

    def __init__(self, number, isInteger):
        super(NumberInputPanel, self).__init__(None)
        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.isInteger = isInteger
        if isInteger:
            self.spin = QtGui.QSpinBox()
            self.spin.setMaximum(100000)
            self.spin.setMinimum(-100000)
            self.spin.setValue(number)
            self.horizontalLayout.addWidget(self.spin)
            self.setLayout(self.horizontalLayout)
        else:
            self.text = QtGui.QLineEdit()
            self.text.setText(str(number))
            self.text.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
            self.horizontalLayout.addWidget(self.text)
            self.pushButton = QtGui.QPushButton()
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
        if self.isInteger:
            return self.spin.value()
        else:
            return self.text.text()