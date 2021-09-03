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

import os
import random

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsApplication,
                       QgsFeatureRequest,
                       QgsProcessingException,
                       QgsProcessingUtils,
                       QgsProcessingAlgorithm,
                       QgsProcessingParameterVectorLayer,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterField,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingOutputVectorLayer)
from collections import defaultdict
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class RandomSelectionWithinSubsets(QgisAlgorithm):
    INPUT = 'INPUT'
    METHOD = 'METHOD'
    NUMBER = 'NUMBER'
    FIELD = 'FIELD'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QgsApplication.getThemeIcon("/algorithms/mAlgorithmSelectRandom.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("/algorithms/mAlgorithmSelectRandom.svg")

    def group(self):
        return self.tr('Vector selection')

    def groupId(self):
        return 'vectorselection'

    def __init__(self):
        super().__init__()

    def flags(self):
        return super().flags() | QgsProcessingAlgorithm.FlagNoThreading | QgsProcessingAlgorithm.FlagNotAvailableInStandaloneTool

    def initAlgorithm(self, config=None):
        self.methods = [self.tr('Number of selected features'),
                        self.tr('Percentage of selected features')]

        self.addParameter(QgsProcessingParameterVectorLayer(self.INPUT,
                                                            self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterField(self.FIELD,
                                                      self.tr('ID field'), None, self.INPUT))
        self.addParameter(QgsProcessingParameterEnum(self.METHOD,
                                                     self.tr('Method'), self.methods, False, 0))
        self.addParameter(QgsProcessingParameterNumber(self.NUMBER,
                                                       self.tr('Number/percentage of selected features'),
                                                       QgsProcessingParameterNumber.Integer,
                                                       10, False, 0.0))
        self.addOutput(QgsProcessingOutputVectorLayer(self.OUTPUT, self.tr('Selected (stratified random)')))

    def name(self):
        return 'randomselectionwithinsubsets'

    def displayName(self):
        return self.tr('Random selection within subsets')

    def processAlgorithm(self, parameters, context, feedback):
        layer = self.parameterAsVectorLayer(parameters, self.INPUT, context)
        method = self.parameterAsEnum(parameters, self.METHOD, context)
        field = self.parameterAsString(parameters, self.FIELD, context)

        index = layer.fields().lookupField(field)

        unique = layer.uniqueValues(index)
        featureCount = layer.featureCount()

        value = self.parameterAsInt(parameters, self.NUMBER, context)
        if method == 0:
            if value > featureCount:
                raise QgsProcessingException(
                    self.tr('Selected number is greater that feature count. '
                            'Choose lesser value and try again.'))
        else:
            if value > 100:
                raise QgsProcessingException(
                    self.tr("Percentage can't be greater than 100. Set a "
                            "different value and try again."))
            value = value / 100.0

        total = 100.0 / (featureCount * len(unique)) if featureCount else 1

        if len(unique) != featureCount:
            classes = defaultdict(list)

            features = layer.getFeatures(QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry).setSubsetOfAttributes([index]))

            for i, feature in enumerate(features):
                if feedback.isCanceled():
                    break

                classes[feature[index]].append(feature.id())
                feedback.setProgress(int(i * total))

            selran = []
            for k, subset in classes.items():
                if feedback.isCanceled():
                    break

                selValue = value if method != 1 else int(round(value * len(subset), 0))
                if selValue > len(subset):
                    selValue = len(subset)
                    feedback.reportError(self.tr('Subset "{}" is smaller than requested number of features.'.format(k)))
                selran.extend(random.sample(subset, selValue))

            layer.selectByIds(selran)
        else:
            layer.selectByIds(list(range(featureCount)))  # FIXME: implies continuous feature ids

        return {self.OUTPUT: parameters[self.INPUT]}
