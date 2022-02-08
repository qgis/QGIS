# -*- coding: utf-8 -*-

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

from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (QgsProcessing,
                       QgsFeatureSink,
                       QgsProcessingException,
                       QgsProcessingAlgorithm,
                       QgsProcessingParameterString,
                       QgsProcessingOutputString)
from qgis import processing


class AlgWithComplexParamNames(QgsProcessingAlgorithm):

    # Constants used to refer to parameters and outputs. They will be
    # used when calling the algorithm from another algorithm, or when
    # calling from the QGIS console.

    INPUT = 'INPUT with many complex chars.123 a'
    INPUT2 = 'another% complex# NaMe'
    OUTPUT = 'OUTPUT'

    def createInstance(self):
        return AlgWithComplexParamNames()

    def name(self):
        return 'complex .name$'

    def initAlgorithm(self, config=None):
        self.addParameter(
            QgsProcessingParameterString(
                self.INPUT,
                'Input string 1'
            )
        )
        self.addParameter(
            QgsProcessingParameterString(
                self.INPUT2,
                'Input string 2'
            )
        )

        self.addOutput(
            QgsProcessingOutputString(
                self.OUTPUT,
                'Output string'
            )
        )

    def processAlgorithm(self, parameters, context, feedback):
        string1 = self.parameterAsString(
            parameters,
            self.INPUT,
            context
        )
        string2 = self.parameterAsString(
            parameters,
            self.INPUT2,
            context
        )

        return {self.OUTPUT: string1.lower() + ':' + string2.lower()}
