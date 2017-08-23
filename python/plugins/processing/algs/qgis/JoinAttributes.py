# -*- coding: utf-8 -*-

"""
***************************************************************************
    JoinAttributes.py
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
from builtins import str

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.core import (QgsFeature,
                       QgsFeatureSink,
                       QgsFeatureRequest,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingUtils,
                       QgsProcessingParameterField,
                       QgsProcessingParameterFeatureSink)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class JoinAttributes(QgisAlgorithm):

    OUTPUT = 'OUTPUT'
    INPUT = 'INPUT'
    INPUT_2 = 'INPUT_2'
    FIELD = 'FIELD'
    FIELD_2 = 'FIELD_2'

    def group(self):
        return self.tr('Vector general')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT_2,
                                                              self.tr('Input layer 2')))
        self.addParameter(QgsProcessingParameterField(self.FIELD,
                                                      self.tr('Table field'), parentLayerParameterName=self.INPUT))
        self.addParameter(QgsProcessingParameterField(self.FIELD_2,
                                                      self.tr('Table field 2'), parentLayerParameterName=self.INPUT_2))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Joined layer')))

    def name(self):
        return 'joinattributestable'

    def displayName(self):
        return self.tr('Join attributes table')

    def processAlgorithm(self, parameters, context, feedback):
        input = self.parameterAsSource(parameters, self.INPUT, context)
        input2 = self.parameterAsSource(parameters, self.INPUT_2, context)
        field = self.parameterAsString(parameters, self.FIELD, context)
        field2 = self.parameterAsString(parameters, self.FIELD_2, context)

        joinField1Index = input.fields().lookupField(field)
        joinField2Index = input2.fields().lookupField(field2)

        outFields = vector.combineFields(input.fields(), input2.fields())

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               outFields, input.wkbType(), input.sourceCrs())

        # Cache attributes of input2
        cache = {}
        features = input2.getFeatures(QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry))
        total = 100.0 / input2.featureCount() if input2.featureCount() else 0
        for current, feat in enumerate(features):
            if feedback.isCanceled():
                break

            attrs = feat.attributes()
            joinValue2 = str(attrs[joinField2Index])
            if joinValue2 not in cache:
                cache[joinValue2] = attrs
            feedback.setProgress(int(current * total))

        # Create output vector layer with additional attribute
        outFeat = QgsFeature()
        features = input.getFeatures()
        total = 100.0 / input.featureCount() if input.featureCount() else 0
        for current, feat in enumerate(features):
            if feedback.isCanceled():
                break

            outFeat.setGeometry(feat.geometry())
            attrs = feat.attributes()
            joinValue1 = str(attrs[joinField1Index])
            attrs.extend(cache.get(joinValue1, []))
            outFeat.setAttributes(attrs)
            sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
