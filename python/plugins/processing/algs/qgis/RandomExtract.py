# -*- coding: utf-8 -*-

"""
***************************************************************************
    RandomExtract.py
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
from builtins import range

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import random

from qgis.core import (QgsFeatureSink,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingOutputVectorLayer)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException


class RandomExtract(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    METHOD = 'METHOD'
    NUMBER = 'NUMBER'

    def group(self):
        return self.tr('Vector selection tools')

    def __init__(self):
        super().__init__()
        self.methods = [self.tr('Number of selected features'),
                        self.tr('Percentage of selected features')]

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))

        self.addParameter(QgsProcessingParameterEnum(self.METHOD,
                                                     self.tr('Method'), self.methods, False, 0))

        self.addParameter(QgsProcessingParameterNumber(self.NUMBER,
                                                       self.tr('Number/percentage of selected features'), QgsProcessingParameterNumber.Integer,
                                                       10, False, 0.0, 999999999999.0))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Extracted (random)')))
        self.addOutput(QgsProcessingOutputVectorLayer(self.OUTPUT, self.tr('Extracted (random)')))

        self.source = None
        self.method = None
        self.value = None
        self.featureCount = None
        self.sink = None
        self.dest_id = None

    def name(self):
        return 'randomextract'

    def displayName(self):
        return self.tr('Random extract')

    def prepareAlgorithm(self, parameters, context, feedback):
        self.source = self.parameterAsSource(parameters, self.INPUT, context)
        self.method = self.parameterAsEnum(parameters, self.METHOD, context)
        self.value = self.parameterAsInt(parameters, self.NUMBER, context)
        self.featureCount = self.source.featureCount()

        if self.method == 0:
            if self.value > self.featureCount:
                raise GeoAlgorithmExecutionException(
                    self.tr('Selected number is greater than feature count. '
                            'Choose a lower value and try again.'))
        else:
            if self.value > 100:
                raise GeoAlgorithmExecutionException(
                    self.tr("Percentage can't be greater than 100. Set a "
                            "different value and try again."))
            self.value = int(round(self.value / 100.0000, 4) * self.featureCount)

        (self.sink, self.dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                                         self.source.fields(), self.source.wkbType(), self.source.sourceCrs())

        return True

    def processAlgorithm(self, context, feedback):
        selran = random.sample(list(range(self.featureCount)), self.value)
        features = self.source.getFeatures()

        total = 100.0 / self.featureCount if self.featureCount else 1
        for i, feat in enumerate(features):
            if feedback.isCanceled():
                break
            if i in selran:
                self.sink.addFeature(feat, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(i * total))
        return True

    def postProcessAlgorithm(self, context, feedback):
        return {self.OUTPUT: self.dest_id}
