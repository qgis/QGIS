from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.parameters.ParameterString import ParameterString
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.modeler.CalculatorModelerParametersDialog import CalculatorModelerParametersDialog
from sextante.outputs.OutputNumber import OutputNumber
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException

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
