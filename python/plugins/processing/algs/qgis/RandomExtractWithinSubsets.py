# -*- coding: utf-8 -*-

"""
***************************************************************************
    RandomSelectionWithinSubsets.py
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

import random

from qgis.core import (QgsFeatureSink,
                       QgsProcessingException,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterField,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingFeatureSource,
                       QgsFeatureRequest)
from collections import defaultdict
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class RandomExtractWithinSubsets(QgisAlgorithm):

    INPUT = 'INPUT'
    METHOD = 'METHOD'
    NUMBER = 'NUMBER'
    FIELD = 'FIELD'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector selection')

    def groupId(self):
        return 'vectorselection'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.methods = [self.tr('Number of selected features'),
                        self.tr('Percentage of selected features')]

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))

        self.addParameter(QgsProcessingParameterField(self.FIELD,
                                                      self.tr('ID field'), None, self.INPUT))

        self.addParameter(QgsProcessingParameterEnum(self.METHOD,
                                                     self.tr('Method'), self.methods, False, 0))

        self.addParameter(QgsProcessingParameterNumber(self.NUMBER,
                                                       self.tr('Number/percentage of selected features'), QgsProcessingParameterNumber.Integer,
                                                       10, False, 0.0))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Extracted (random stratified)')))

    def name(self):
        return 'randomextractwithinsubsets'

    def displayName(self):
        return self.tr('Random extract within subsets')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        method = self.parameterAsEnum(parameters, self.METHOD, context)

        field = self.parameterAsString(parameters, self.FIELD, context)

        index = source.fields().lookupField(field)

        features = source.getFeatures(QgsFeatureRequest(), QgsProcessingFeatureSource.FlagSkipGeometryValidityChecks)
        featureCount = source.featureCount()
        unique = source.uniqueValues(index)
        value = self.parameterAsInt(parameters, self.NUMBER, context)
        if method == 0:
            if value > featureCount:
                raise QgsProcessingException(
                    self.tr('Selected number is greater that feature count. '
                            'Choose lesser value and try again.'))
        else:
            if value > 100:
                raise QgsProcessingException(
                    self.tr("Percentage can't be greater than 100. Set "
                            "correct value and try again."))
            value = value / 100.0

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), source.wkbType(), source.sourceCrs())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        selran = []
        total = 100.0 / (featureCount * len(unique)) if featureCount else 1

        classes = defaultdict(list)
        for i, feature in enumerate(features):
            if feedback.isCanceled():
                break
            attrs = feature.attributes()
            classes[attrs[index]].append(feature)
            feedback.setProgress(int(i * total))

        for k, subset in classes.items():
            selValue = value if method != 1 else int(round(value * len(subset), 0))
            if selValue > len(subset):
                selValue = len(subset)
                feedback.reportError(self.tr('Subset "{}" is smaller than requested number of features.'.format(k)))
            selran.extend(random.sample(subset, selValue))

        total = 100.0 / featureCount if featureCount else 1
        for (i, feat) in enumerate(selran):
            if feedback.isCanceled():
                break
            sink.addFeature(feat, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(i * total))
        return {self.OUTPUT: dest_id}
