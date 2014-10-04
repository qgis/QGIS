# -*- coding: utf-8 -*-

"""
***************************************************************************
    CalculatorModelerAlgorithm.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from PyQt4 import QtCore, QtGui
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import \
        GeoAlgorithmExecutionException
from processing.core.parameters import ParameterString
from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputNumber
from processing.modeler.ModelerParametersDialog import ModelerParametersDialog
from processing.modeler.ModelerAlgorithm import Algorithm


FORMULA = 'FORMULA'
NUMBER = 'NUMBER'
RESULT = 'RESULT'
AVAILABLE_VARIABLES = 10

class CalculatorModelerAlgorithm(GeoAlgorithm):

    def defineCharacteristics(self):
        self.showInModeler = True
        self.showInToolbox = False
        self.name = self.tr('Calculator', 'CalculatorModelerAlgorithm')
        self.group = self.tr('Modeler-only tools', 'CalculatorModelerAlgorithm')
        self.addParameter(ParameterString(FORMULA,
            self.tr('Formula', 'CalculatorModelerAlgorithm'), ''))
        for i in range(AVAILABLE_VARIABLES):
            self.addParameter(ParameterNumber(NUMBER
                              + str(i), 'dummy'))
        self.addOutput(OutputNumber(RESULT,
            self.tr('Result', 'CalculatorModelerAlgorithm')))

    def processAlgorithm(self, progress):
        formula = self.getParameterValue(FORMULA)
        for i in range(AVAILABLE_VARIABLES):
            name = NUMBER + str(i)
            num = self.getParameterValue(name)
            formula = formula.replace(chr(97 + i), str(num))
        try:
            result = eval(formula)
            self.setOutputValue(RESULT, result)
        except:
            raise GeoAlgorithmExecutionException(
                self.tr('Wrong formula: %s', 'CalculatorModelerAlgorithm') % formula)

    def getCustomModelerParametersDialog(self, modelAlg, algIndex=None):
        return CalculatorModelerParametersDialog(self, modelAlg, algIndex)


class CalculatorModelerParametersDialog(ModelerParametersDialog):

    def setupUi(self):
        self.valueItems = {}
        self.dependentItems = {}
        self.resize(650, 450)
        self.buttonBox = QtGui.QDialogButtonBox()
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel
                | QtGui.QDialogButtonBox.Ok)
        self.infoText = QtGui.QTextEdit()
        numbers = self.getAvailableValuesOfType(ParameterNumber, OutputNumber)
        text = self.tr('You can refer to model values in your formula, using '
            'single-letter variables, as follows:\n', 'CalculatorModelerParametersDialog')
        ichar = 97
        if numbers:
            for number in numbers:
                text += chr(ichar) + '->' + self.resolveValueDescription(number) + '\n'
                ichar += 1
        else:
            text += self.tr('\n - No numerical variables are available.', 'CalculatorModelerParametersDialog')
        self.infoText.setText(text)
        self.infoText.setEnabled(False)
        self.formulaText = QtGui.QLineEdit()
        if hasattr(self.formulaText, 'setPlaceholderText'):
            self.formulaText.setPlaceholderText(self.tr('[Enter your formula here]', 'CalculatorModelerParametersDialog'))
        self.setWindowTitle(self.tr('Calculator', 'CalculatorModelerParametersDialog'))
        self.verticalLayout = QtGui.QVBoxLayout()
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.addWidget(self.infoText)
        self.verticalLayout.addWidget(self.formulaText)
        self.verticalLayout.addWidget(self.buttonBox)
        self.setLayout(self.verticalLayout)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL('accepted()'),
                               self.okPressed)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL('rejected()'),
                               self.cancelPressed)
        QtCore.QMetaObject.connectSlotsByName(self)

    def createAlgorithm(self):
        alg = Algorithm('modelertools:calculator')
        alg.setName(self.model)
        alg.description = self.tr('Calculator', 'CalculatorModelerParametersDialog')

        formula = self.formulaText.text()
        alg.params[FORMULA] = formula

        for i in xrange(AVAILABLE_VARIABLES):
            paramname = NUMBER + str(i)
            alg.params[paramname] = None

        numbers = self.getAvailableValuesOfType(ParameterNumber, OutputNumber)
        used = []
        for i in range(len(numbers)):
            if str(chr(i + 97)) in formula:
                used.append(numbers[i])

        for i, variable in enumerate(used):
            paramname = NUMBER + str(i)
            alg.params[paramname] = variable

        #TODO check formula is correct
        return alg
