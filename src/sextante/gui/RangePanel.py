from PyQt4 import QtGui

class RangePanel(QtGui.QWidget):

    def __init__(self, param):
        super(RangePanel, self).__init__(None)
        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.labelmin = QtGui.QLabel()
        self.labelmin.setText("Min")
        self.textmin = QtGui.QLineEdit()
        self.textmin.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        self.labelmax = QtGui.QLabel()
        self.labelmax.setText("Max")
        self.textmax = QtGui.QLineEdit()
        self.textmin.setText(param.default.split(",")[0])
        self.textmax.setText(param.default.split(",")[1])
        self.textmax.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        self.horizontalLayout.addWidget(self.labelmin)
        self.horizontalLayout.addWidget(self.textmin)
        self.horizontalLayout.addWidget(self.labelmax)
        self.horizontalLayout.addWidget(self.textmax)
        self.setLayout(self.horizontalLayout)

    def getValue(self):
        return self.textmin.text() + "," + self.textmax.text()