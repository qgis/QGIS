from PyQt4 import QtCore, QtGui
from sextante.core.QGisLayers import QGisLayers

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s



class Ui_ParametersDialog(object):

    SAVE_TO_TEMP_FILE = "[Save to temporary file]"

    def setupUi(self, Dialog):
        self.alg = Dialog. alg
        self.valueItems = {}
        Dialog.setObjectName(_fromUtf8("Parameters"))
        Dialog.resize(650, 450)
        self.buttonBox = QtGui.QDialogButtonBox(Dialog)
        self.buttonBox.setGeometry(QtCore.QRect(110, 400, 441, 32))
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName(_fromUtf8("buttonBox"))
        self.tableWidget = QtGui.QTableWidget(Dialog)
        self.tableWidget.setGeometry(QtCore.QRect(5, 5, 640, 350))
        self.tableWidget.setSelectionMode(QtGui.QAbstractItemView.NoSelection)
        self.tableWidget.setColumnCount(2)
        self.tableWidget.setColumnWidth(0,300)
        self.tableWidget.setColumnWidth(1,300)
        self.tableWidget.setHorizontalHeaderItem(0, QtGui.QTableWidgetItem("Parameter"))
        self.tableWidget.setHorizontalHeaderItem(1, QtGui.QTableWidgetItem("Value"))
        self.tableWidget.setObjectName(_fromUtf8("tableWidget"))
        self.tableWidget.verticalHeader().setVisible(False)

        self.setTableContent()

        self.retranslateUi(Dialog)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("accepted()")), self.accept)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL(_fromUtf8("rejected()")), self.reject)
        QtCore.QMetaObject.connectSlotsByName(Dialog)


    def getItemFromParameter(self, param):

        item = QtGui.QComboBox()
        item.addItems(QGisLayers.getLayers())
        #item = QtGui.QLineEdit()
        #item.setFlags(QtCore.Qt.ItemIsEnabled|QtCore.Qt.ItemIsEditable)
        return item


    def setTableContent(self):
        params = self.alg.parameters.values()
        outputs = self.alg.outputs.values()
        numParams = len(self.alg.parameters.values())
        numOutputs = len(self.alg.outputs.values())
        self.tableWidget.setRowCount(numParams + numOutputs)

        i=0
        for param in params:
            item = QtGui.QTableWidgetItem(param.description)
            item.setFlags(QtCore.Qt.ItemIsEnabled)
            self.tableWidget.setItem(i,0, item)
            item = self.getItemFromParameter(param)
            self.valueItems[param.name] = item
            self.tableWidget.setCellWidget(i,1, item)
            self.tableWidget.setRowHeight(i,22)
            i+=1

        for output in outputs:
            item = QtGui.QTableWidgetItem(output.description)
            item.setFlags(QtCore.Qt.ItemIsEnabled)
            self.tableWidget.setItem(i,0, item)
            item = QtGui.QLineEdit()
            item.setText(self.SAVE_TO_TEMP_FILE)
            self.valueItems[output.name] = item
            #item.setFlags(QtCore.Qt.ItemIsEnabled|QtCore.Qt.ItemIsEditable)
            self.tableWidget.setCellWidget(i,1, item)
            self.tableWidget.setRowHeight(i,22)
            i+=1


    def accept(self):
        pass

    def reject(self):
        pass

    def retranslateUi(self, Dialog):
        Dialog.setWindowTitle(self.alg.name)



