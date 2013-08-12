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

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.parameters.ParameterString import ParameterString
from processing.parameters.ParameterNumber import ParameterNumber
from processing.modeler.CalculatorModelerParametersDialog import CalculatorModelerParametersDialog
from processing.outputs.OutputNumber import OutputNumber
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException

class CalculatorModelerAlgorithm(GeoAlgorithm):

    FORMULA = "FORMULA"
    NUMBER = "NUMBER"
    RESULT = "RESULT"
    AVAILABLE_VARIABLES = 10

    def defineCharacteristics(self):
        self.showInModeler = True
        self.showInToolbox = False
        self.name = "Calculator"
        self.group = "Modeler-only tools"
        self.addParameter(ParameterString(self.FORMULA, "Formula", ""))
        for i in range(self.AVAILABLE_VARIABLES):
            self.addParameter(ParameterNumber(CalculatorModelerAlgorithm.NUMBER + str(i), "dummy"))
        self.addOutput(OutputNumber(self.RESULT, "Result"))

    def processAlgorithm(self, progress):
        formula = self.getParameterValue(self.FORMULA)
        for i in range(self.AVAILABLE_VARIABLES):
            name = CalculatorModelerAlgorithm.NUMBER + str(i)
            num = self.getParameterValue(name)
            formula = formula.replace(chr(97+i), str(num))
        try:
            result = eval(formula)
            self.setOutputValue(self.RESULT, result)
        except:
            raise GeoAlgorithmExecutionException("Wrong formula: " + formula)


    def getCustomModelerParametersDialog(self, modelAlg, algIndex = None):
        return CalculatorModelerParametersDialog(self, modelAlg, algIndex)
