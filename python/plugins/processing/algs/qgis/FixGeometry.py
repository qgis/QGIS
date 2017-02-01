# -*- coding: utf-8 -*-

"""
***************************************************************************
    FixGeometry.py
    -----------------------
    Date                 : January 2017
    Copyright            : (C) 2017 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'January 2017'
__copyright__ = '(C) 2017, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.tools import dataobjects, vector


class FixGeometry(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Fix geometries')
        self.group, self.i18n_group = self.trAlgorithm('Vector geometry tools')

        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input Layer')))
        self.addOutput(OutputVector(self.OUTPUT,
                                    self.tr('Layer with fixed geometries')))

    def processAlgorithm(self, feedback):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT))

        writer = self.getOutputFromName(
            self.OUTPUT).getVectorWriter(
                layer.fields(),
                layer.wkbType(),
                layer.crs())

        features = vector.features(layer)
        if len(features) == 0:
            raise GeoAlgorithmExecutionException(self.tr('There are no features in the input layer'))

        total = 100.0 / len(features)
        for current, inputFeature in enumerate(features):
            outputFeature = inputFeature
            if inputFeature.geometry():
                outputGeometry = inputFeature.geometry().makeValid()
                if not outputGeometry:
                    ProcessingLog.addToLog(ProcessingLog.LOG_WARNING,
                                           'makeValid failed for feature {}'.format(inputFeature.id()))
                outputFeature.setGeometry(outputGeometry)

            writer.addFeature(outputFeature)
            feedback.setProgress(int(current * total))

        del writer
