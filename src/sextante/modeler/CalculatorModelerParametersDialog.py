from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtCore, QtGui
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.modeler.ModelerAlgorithm import AlgorithmAndParameter
from sextante.outputs.OutputNumber import OutputNumber

class CalculatorModelerParametersDialog(QtGui.QDialog):


    def __init__(self, alg, model):
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.alg = alg
        self.model = model
        self.setupUi()
        self.params = None

    def setupUi(self):
        self.valueItems = {}
        self.dependentItems = {}
        self.setObjectName("Parameters")
        self.resize(650, 450)
        self.buttonBox = QtGui.QDialogButtonBox()
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName("buttonBox")
        self.infoText = QtGui.QTextEdit()
        numbers = self.getNumbers();
        text = "You can refer to model values in you formula, using single-letter variables, as follows:\n\n"
        ichar = 97;
        for number in numbers:
            text += chr(ichar) + "->" + number.name() + "\n"
            ichar += 1
        self.infoText.setText(text)
        self.infoText.setEnabled(False)
        self.formulaText = QtGui.QLineEdit()
        self.formulaText.setPlaceholderText("[Enter your formula here]")
        self.setWindowTitle(self.alg.name)
        self.verticalLayout = QtGui.QVBoxLayout()
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setObjectName("hLayout")
        self.verticalLayout.addWidget(self.infoText)
        self.verticalLayout.addWidget(self.formulaText)
        self.verticalLayout.addWidget(self.buttonBox)
        self.setLayout(self.verticalLayout)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL("accepted()"), self.okPressed)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL("rejected()"), self.cancelPressed)
        QtCore.QMetaObject.connectSlotsByName(self)


    def getNumbers(self):
        numbers = []
        params = self.model.parameters
        for param in params:
            if isinstance(param, ParameterNumber):
                numbers.append(AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, param.name, "", param.description))

        i=0
        for alg in self.model.algs:
            for out in alg.outputs:
                if isinstance(out, OutputNumber):
                    numbers.append(AlgorithmAndParameter(i, out.name, alg.name, out.description))
            i+=1
        return numbers

    def setParamValues(self):
        self.params = {}
        self.values = {}
        self.outputs = {}

        name =  self.model.getSafeNameForHarcodedParameter(self.alg.getParameterFromName(self.alg.FORMULA))
        value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
        self.params[self.alg.FORMULA] = value
        formula = str(self.formulaText.text())
        self.values[name] = formula

        numbers = self.getNumbers();
        used = []
        for i in range(len(numbers)):
            if str(chr(i+97)) in formula:
                used.append(numbers[i])
        i = 0
        for variable in used:
            paramname = self.alg.NUMBER + str(i)
            self.params[paramname] = variable
            i += 1
        #we create a dummy harcoded value for all unused variable slots
        paramname = self.alg.NUMBER + str(i)
        name =  self.model.getSafeNameForHarcodedParameter(self.alg.getParameterFromName(paramname))
        value = AlgorithmAndParameter(AlgorithmAndParameter.PARENT_MODEL_ALGORITHM, name)
        self.values[name] = 0
        for i in range(len(used), self.alg.AVAILABLE_VARIABLES):
            paramname = self.alg.NUMBER + str(i)
            self.params[paramname] = value

        self.outputs[self.alg.RESULT] = None
        return True



    def okPressed(self):
        if self.setParamValues():
            self.close()
        else:
            QMessageBox.critical(self, "Unable to add algorithm", "Wrong or missing parameter values")
            self.params = None


    def cancelPressed(self):
        self.params = None
        self.close()

