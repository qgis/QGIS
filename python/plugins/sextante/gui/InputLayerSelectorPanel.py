from PyQt4 import QtGui, QtCore

class InputLayerSelectorPanel(QtGui.QWidget):

    def __init__(self, options):
        super(InputLayerSelectorPanel, self).__init__(None)
        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.text = QtGui.QComboBox()
        for name, value in options:
            self.text.addItem(name, value)
        self.text.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        self.horizontalLayout.addWidget(self.text)
        self.pushButton = QtGui.QPushButton()
        self.pushButton.setText("...")
        self.pushButton.clicked.connect(self.showSelectionDialog)
        self.horizontalLayout.addWidget(self.pushButton)
        self.setLayout(self.horizontalLayout)

    def showSelectionDialog(self):
        filename = QtGui.QFileDialog.getOpenFileName(self, "All files", "", "*.*")
        if filename:
            self.text.addItem(filename, filename)
            self.text.setCurrentIndex(self.text.count() - 1)

    def getValue(self):
        return self.text.itemData(self.text.currentIndex()).toPyObject()
