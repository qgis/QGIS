from PyQt4 import QtCore, QtGui
from sextante.gui.MultipleInputDialog import MultipleInputDialog

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class MultipleInputPanel(QtGui.QWidget):

    def __init__(self, options, parent = None):
        self.options = options
        self.selectedoptions = []
        super(MultipleInputPanel, self).__init__(parent)
        self.setObjectName(_fromUtf8("MSPanel"))
        self.resize(266, 30)
        self.pushButton = QtGui.QPushButton(self)
        self.pushButton.setGeometry(QtCore.QRect(220, 0, 40, 30))
        self.pushButton.setObjectName(_fromUtf8("pushButton"))
        self.label = QtGui.QLabel(self)
        self.label.setGeometry(QtCore.QRect(0, 0, 220, 30))
        self.label.setObjectName(_fromUtf8("label"))
        self.pushButton.setText("...")
        self.label.setText("0 elements selected")
        self.pushButton.clicked.connect(self.showSelectionDialog)
        #QtCore.QMetaObject.connectSlotsByName(Form)

    def showSelectionDialog(self):
        dlg = MultipleInputDialog(self.options, self.selectedoptions)
        dlg.exec_()
        if dlg.selected != None:
            self.selectedoptions = dlg.selectedOptions
            self.label.setText(str(len(self.selectedoptions)) + " elements selected")
