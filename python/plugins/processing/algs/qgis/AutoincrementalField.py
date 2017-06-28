# -*- coding: utf-8 -*-

"""
***************************************************************************
    AutoincrementalField.py
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

from qgis.PyQt.QtCore import QVariant
from qgis.core import (QgsField,
                       QgsFeature,
                       QgsFeatureSink,
                       QgsApplication,
                       QgsProcessingUtils,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingOutputVectorLayer)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class AutoincrementalField(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def __init__(self):
        super().__init__()

        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Incremented')))
        self.addOutput(QgsProcessingOutputVectorLayer(self.OUTPUT, self.tr('Incremented')))

        self.source = None
        self.sink = None
        self.dest_id = None

    def group(self):
        return self.tr('Vector table tools')

    def name(self):
        return 'addautoincrementalfield'

    def displayName(self):
        return self.tr('Add autoincremental field')

    def prepareAlgorithm(self, parameters, context, feedback):
        self.source = self.parameterAsSource(parameters, self.INPUT, context)
        fields = self.source.fields()
        fields.append(QgsField('AUTO', QVariant.Int))

        (self.sink, self.dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                                         fields, self.source.wkbType(), self.source.sourceCrs())
        return True

    def processAlgorithm(self, context, feedback):
        features = self.source.getFeatures()
        total = 100.0 / self.source.featureCount() if self.source.featureCount() else 0
        for current, input_feature in enumerate(features):
            if feedback.isCanceled():
                break

            output_feature = input_feature
            attributes = input_feature.attributes()
            attributes.append(current)
            output_feature.setAttributes(attributes)

            self.sink.addFeature(output_feature, QgsFeatureSink.FastInsert)
            feedback.setProgress(int(current * total))
        return True

    def postProcessAlgorithm(self, context, feedback):
        return {self.OUTPUT: self.dest_id}
