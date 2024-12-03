"""
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

from qgis.core import (
    QgsProcessingAlgorithm,
    QgsProcessingOutputString,
    QgsProcessingParameterString,
)


class ConvertStringToUppercase(QgsProcessingAlgorithm):
    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def createInstance(self):
        return ConvertStringToUppercase()

    def name(self):
        return "converttouppercase"

    def displayName(self):
        return "Convert to upper"

    def shortDescription(self):
        return "Converts a string to upper case"

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterString(self.INPUT, "Input string"))

        self.addOutput(QgsProcessingOutputString(self.OUTPUT, "Output string"))

    def processAlgorithm(self, parameters, context, feedback):
        input_string = self.parameterAsString(parameters, self.INPUT, context)
        output_string = input_string.upper()
        feedback.pushInfo(f"Converted {input_string} to {output_string}")
        return {self.OUTPUT: output_string}
