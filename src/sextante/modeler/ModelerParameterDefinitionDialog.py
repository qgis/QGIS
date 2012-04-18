from PyQt4 import QtCore, QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.parameters.ParameterTable import ParameterTable
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterMultipleInput import ParameterMultipleInput
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.parameters.ParameterString import ParameterString
from sextante.parameters.ParameterTableField import ParameterTableField


class ModelerParameterDefinitionDialog(QtGui.QDialog):

    PARAMETER_NUMBER="Number"
    PARAMETER_RASTER="Raster Layer"
    PARAMETER_TABLE="Table"
    PARAMETER_VECTOR="Vector layer"
    PARAMETER_STRING="String"
    PARAMETER_BOOLEAN="Boolean"
    PARAMETER_TABLE_FIELD="Table field"
    #TO ADD
    PARAMETER_MULTIPLE="Multiple input"
    PARAMETER_FIXED_TABLE="Fixed table"

    paramTypes = [PARAMETER_BOOLEAN,PARAMETER_NUMBER, PARAMETER_RASTER,
                  PARAMETER_STRING, PARAMETER_VECTOR, PARAMETER_TABLE, PARAMETER_TABLE_FIELD]

    def __init__(self, alg, paramType):
        self.alg = alg;
        self.paramType = paramType
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.setupUi()
        self.param = None

    def setupUi(self):
        self.setWindowTitle("Parameter definition")

        self.verticalLayout = QtGui.QVBoxLayout(self)
        self.verticalLayout.setSpacing(40)
        self.verticalLayout.setMargin(20)

        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.label = QtGui.QLabel("Parameter name")
        self.horizontalLayout.addWidget(self.label)
        self.nameTextBox = QtGui.QLineEdit()
        self.horizontalLayout.addWidget(self.nameTextBox)
        self.verticalLayout.addLayout(self.horizontalLayout)

        self.horizontalLayout2 = QtGui.QHBoxLayout(self)
        self.horizontalLayout2.setSpacing(2)
        self.horizontalLayout2.setMargin(0)
        self.horizontalLayout3 = QtGui.QHBoxLayout(self)
        self.horizontalLayout3.setSpacing(2)
        self.horizontalLayout3.setMargin(0)

        if self.paramType == ModelerParameterDefinitionDialog.PARAMETER_BOOLEAN:
            self.horizontalLayout2.addWidget(QtGui.QLabel("Default value"))
            self.yesNoCombo = QtGui.QComboBox()
            self.yesNoCombo.addItem("Yes")
            self.yesNoCombo.addItem("No")
            self.horizontalLayout2.addWidget(self.yesNoCombo)
            self.verticalLayout.addLayout(self.horizontalLayout2)
        if self.paramType == ModelerParameterDefinitionDialog.PARAMETER_TABLE_FIELD:
            self.horizontalLayout2.addWidget(QtGui.QLabel("Parent layer"))
            self.parentCombo = QtGui.QComboBox()
            for param in self.alg.parameters:
                if isinstance(param, (ParameterVector, ParameterTable)):
                    self.parentCombo.addItem(param.description, param.name)
            self.horizontalLayout2.addWidget(self.parentCombo)
            self.verticalLayout.addLayout(self.horizontalLayout2)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_RASTER:
            self.horizontalLayout2.addWidget(QtGui.QLabel("Mandatory"))
            self.yesNoCombo = QtGui.QComboBox()
            self.yesNoCombo.addItem("Yes")
            self.yesNoCombo.addItem("No")
            self.horizontalLayout2.addWidget(self.yesNoCombo)
            self.verticalLayout.addLayout(self.horizontalLayout2)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_TABLE:
            self.horizontalLayout2.addWidget(QtGui.QLabel("Mandatory"))
            self.yesNoCombo = QtGui.QComboBox()
            self.yesNoCombo.addItem("Yes")
            self.yesNoCombo.addItem("No")
            self.horizontalLayout2.addWidget(self.yesNoCombo)
            self.verticalLayout.addLayout(self.horizontalLayout2)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_VECTOR:
            self.horizontalLayout2.addWidget(QtGui.QLabel("Mandatory"))
            self.yesNoCombo = QtGui.QComboBox()
            self.yesNoCombo.addItem("Yes")
            self.yesNoCombo.addItem("No")
            self.horizontalLayout2.addWidget(self.yesNoCombo)
            self.horizontalLayout3.addWidget(QtGui.QLabel("Shape type"))
            self.shapetypeCombo = QtGui.QComboBox()
            self.shapetypeCombo.addItem("Any")
            self.shapetypeCombo.addItem("Point")
            self.shapetypeCombo.addItem("Line")
            self.shapetypeCombo.addItem("Polygon")
            self.horizontalLayout3.addWidget(self.shapetypeCombo)
            self.verticalLayout.addLayout(self.horizontalLayout3)
            self.verticalLayout.addLayout(self.horizontalLayout2)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_MULTIPLE:
            self.horizontalLayout2.addWidget(QtGui.QLabel("Mandatory"))
            self.yesNoCombo = QtGui.QComboBox()
            self.yesNoCombo.addItem("Yes")
            self.yesNoCombo.addItem("No")
            self.horizontalLayout2.addWidget(self.yesNoCombo)
            self.horizontalLayout3.addWidget(QtGui.QLabel("Data type"))
            self.datatypeCombo = QtGui.QComboBox()
            self.datatypeCombo.addItem("Vector (any)")
            self.datatypeCombo.addItem("Vector (point)")
            self.datatypeCombo.addItem("Vector (line)")
            self.datatypeCombo.addItem("Vector (polygon)")
            self.datatypeCombo.addItem("Raster")
            self.datatypeCombo.addItem("Table")
            self.horizontalLayout3.addWidget(self.datatypeCombo)
            self.verticalLayout.addLayout(self.horizontalLayout3)
            self.verticalLayout.addLayout(self.horizontalLayout2)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_NUMBER:
            self.horizontalLayout2.addWidget(QtGui.QLabel("Min/Max values"))
            self.minTextBox = QtGui.QLineEdit()
            self.maxTextBox = QtGui.QLineEdit()
            self.horizontalLayout2.addWidget(self.minTextBox)
            self.horizontalLayout2.addWidget(self.maxTextBox)
            self.verticalLayout.addLayout(self.horizontalLayout2)
            self.horizontalLayout3.addWidget(QtGui.QLabel("Default value"))
            self.defaultTextBox = QtGui.QLineEdit()
            self.defaultTextBox.setText("0")
            self.horizontalLayout3.addWidget(self.defaultTextBox)
            self.verticalLayout.addLayout(self.horizontalLayout3)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_STRING:
            self.horizontalLayout2.addWidget(QtGui.QLabel("Default value"))
            self.defaultTextBox = QtGui.QLineEdit()
            self.horizontalLayout2.addWidget(self.defaultTextBox)
            self.verticalLayout.addLayout(self.horizontalLayout2)

        self.buttonBox = QtGui.QDialogButtonBox(self)
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName("buttonBox")
        QObject.connect(self.buttonBox, QtCore.SIGNAL("accepted()"), self.okPressed)
        QObject.connect(self.buttonBox, QtCore.SIGNAL("rejected()"), self.cancelPressed)
        self.verticalLayout.addWidget(self.buttonBox)

        self.setLayout(self.verticalLayout)


    def okPressed(self):
        description = str(self.nameTextBox.text())
        if description.strip() == "":
            QMessageBox.critical(self, "Unable to define parameter", "Invalid parameter name")
            return
        name = self.paramType.upper().replace(" ","") + "_" + description.upper().replace(" ","")
        if self.paramType == ModelerParameterDefinitionDialog.PARAMETER_BOOLEAN:
            self.param = ParameterBoolean(name, description, self.yesNoCombo.currentIndex() == 0)
        if self.paramType == ModelerParameterDefinitionDialog.PARAMETER_TABLE_FIELD:
            if self.parentCombo.currentIndex() < 0:
                QMessageBox.critical(self, "Unable to define parameter", "Wrong or missing parameter values")
                return
            parent = self.parentCombo.itemData(self.parentCombo.currentIndex()).toPyObject()
            self.param = ParameterTableField(name, description, parent)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_RASTER:
            self.param = ParameterRaster(name, description, self.yesNoCombo.currentIndex() == 1)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_TABLE:
            self.param = ParameterTable(name, description, self.yesNoCombo.currentIndex() == 1)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_VECTOR:
            self.param = ParameterVector(name, description, self.shapetypeCombo.currentIndex()-1, self.yesNoCombo.currentIndex() == 1)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_MULTIPLE:
            self.param = ParameterMultipleInput(name, description, self.datatypeCombo.currentIndex()-1, self.yesNoCombo.currentIndex() == 1)
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_NUMBER:
            try:
                vmin = str(self.minTextBox.text()).strip()
                if vmin == "":
                    vmin = None
                else:
                    vmin = float(vmin)
                vmax = str(self.maxTextBox.text()).strip()
                if vmax == "":
                    vmax = None
                else:
                    vmax = float(vmax)
                self.param = ParameterNumber(name, description, vmin, vmax, float(str(self.defaultTextBox.text())))
            except:
               QMessageBox.critical(self, "Unable to define parameter", "Wrong or missing parameter values")
               return
        elif self.paramType == ModelerParameterDefinitionDialog.PARAMETER_STRING:
            self.param = ParameterString(name, description, str(self.defaultTextBox.text()))

        self.close()

    def cancelPressed(self):
        self.param = None
        self.close()

