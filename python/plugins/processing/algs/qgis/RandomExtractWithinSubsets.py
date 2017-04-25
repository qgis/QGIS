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
from builtins import range

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import random

from qgis.core import (QgsApplication,
                       QgsProcessingUtils)
from collections import defaultdict
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterTableField
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class RandomExtractWithinSubsets(GeoAlgorithm):

    INPUT = 'INPUT'
    METHOD = 'METHOD'
    NUMBER = 'NUMBER'
    FIELD = 'FIELD'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def group(self):
        return self.tr('Vector selection tools')

    def name(self):
        return 'randomextractwithinsubsets'

    def displayName(self):
        return self.tr('Random extract within subsets')

    def defineCharacteristics(self):
        self.methods = [self.tr('Number of selected features'),
                        self.tr('Percentage of selected features')]

        self.addParameter(ParameterVector(self.INPUT,
                                          self.tr('Input layer')))
        self.addParameter(ParameterTableField(self.FIELD,
                                              self.tr('ID field'), self.INPUT))
        self.addParameter(ParameterSelection(self.METHOD,
                                             self.tr('Method'), self.methods, 0))
        self.addParameter(ParameterNumber(self.NUMBER,
                                          self.tr('Number/percentage of selected features'), 1, None, 10))

        self.addOutput(OutputVector(self.OUTPUT, self.tr('Extracted (random stratified)')))

    def processAlgorithm(self, context, feedback):
        filename = self.getParameterValue(self.INPUT)

        layer = dataobjects.getLayerFromString(filename)
        field = self.getParameterValue(self.FIELD)
        method = self.getParameterValue(self.METHOD)

        index = layer.fields().lookupField(field)

        features = QgsProcessingUtils.getFeatures(layer, context)
        featureCount = QgsProcessingUtils.featureCount(layer, context)
        unique = QgsProcessingUtils.uniqueValues(layer, index, context)
        value = int(self.getParameterValue(self.NUMBER))
        if method == 0:
            if value > featureCount:
                raise GeoAlgorithmExecutionException(
                    self.tr('Selected number is greater that feature count. '
                            'Choose lesser value and try again.'))
        else:
            if value > 100:
                raise GeoAlgorithmExecutionException(
                    self.tr("Percentage can't be greater than 100. Set "
                            "correct value and try again."))
            value = value / 100.0

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            layer.fields().toList(), layer.wkbType(), layer.crs())

        selran = []
        total = 100.0 / (featureCount * len(unique))
        features = QgsProcessingUtils.getFeatures(layer, context)

        classes = defaultdict(list)
        for i, feature in enumerate(features):
            attrs = feature.attributes()
            classes[attrs[index]].append(feature)
            feedback.setProgress(int(i * total))

        for subset in classes.values():
            selValue = value if method != 1 else int(round(value * len(subset), 0))
            selran.extend(random.sample(subset, selValue))

        features = QgsProcessingUtils.getFeatures(layer, context)
        total = 100.0 / QgsProcessingUtils.featureCount(layer, context)
        for (i, feat) in enumerate(selran):
            writer.addFeature(feat)
            feedback.setProgress(int(i * total))
        del writer
