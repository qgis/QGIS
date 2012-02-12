from PyQt4 import QtCore, QtGui
from sextante.gui.MultipleInputDialog import MultipleInputDialog

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class MultipleInputPanel(QtGui.QWidget):

    def __init__(self, options, parent = None):
        super(MultipleInputPanel, self).__init__(parent)
        self.options = options
        self.selectedoptions = []
        self.setObjectName(_fromUtf8("MSPanel"))
        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.setObjectName(_fromUtf8("hLayout"))
        self.label = QtGui.QLabel()
        self.label.setObjectName(_fromUtf8("label"))
        self.label.setText("0 elements selected")
        self.label.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        self.horizontalLayout.addWidget(self.label)
        self.pushButton = QtGui.QPushButton()
        self.pushButton.setObjectName(_fromUtf8("pushButton"))
        self.pushButton.setText("...")
        self.pushButton.clicked.connect(self.showSelectionDialog)
        self.horizontalLayout.addWidget(self.pushButton)
        self.setLayout(self.horizontalLayout)

    def showSelectionDialog(self):
        dlg = MultipleInputDialog(self.options, self.selectedoptions)
        dlg.exec_()
        if dlg.selectedoptions != None:
            self.selectedoptions = dlg.selectedoptions
            self.label.setText(str(len(self.selectedoptions)) + " elements selected")
