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
                       QgsProcessingException,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterFeatureSink)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class RandomExtract(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    METHOD = 'METHOD'
    NUMBER = 'NUMBER'

    def group(self):
        return self.tr('Vector selection')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
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

    def name(self):
        return 'randomextract'

    def displayName(self):
        return self.tr('Random extract')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        method = self.parameterAsEnum(parameters, self.METHOD, context)

        features = source.getFeatures()
        featureCount = source.featureCount()
        value = self.parameterAsInt(parameters, self.NUMBER, context)

        if method == 0:
            if value > featureCount:
                raise QgsProcessingException(
                    self.tr('Selected number is greater than feature count. '
                            'Choose a lower value and try again.'))
        else:
            if value > 100:
                raise QgsProcessingException(
                    self.tr("Percentage can't be greater than 100. Set a "
                            "different value and try again."))
            value = int(round(value / 100.0000, 4) * featureCount)

        selran = random.sample(list(range(featureCount)), value)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), source.wkbType(), source.sourceCrs())

        total = 100.0 / featureCount if featureCount else 1
        for i, feat in enumerate(features):
            if feedback.isCanceled():
                break
            if i in selran:
                sink.addFeature(feat, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(i * total))
        return {self.OUTPUT: dest_id}
