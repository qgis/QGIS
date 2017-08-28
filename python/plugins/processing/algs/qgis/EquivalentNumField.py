# -*- coding: utf-8 -*-

"""
***************************************************************************
    EquivalentNumField.py
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
                       QgsFeatureSink,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterField,
                       QgsProcessingParameterFeatureSink)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class EquivalentNumField(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    FIELD = 'FIELD'

    def group(self):
        return self.tr('Vector table')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterField(self.FIELD,
                                                      self.tr('Class field'),
                                                      None, self.INPUT, QgsProcessingParameterField.Any))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Layer with index field')))

    def name(self):
        return 'adduniquevalueindexfield'

    def displayName(self):
        return self.tr('Add unique value index field')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        fields = source.fields()
        fields.append(QgsField('NUM_FIELD', QVariant.Int))

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, source.wkbType(), source.sourceCrs())

        field_name = self.parameterAsString(parameters, self.FIELD, context)
        field_index = source.fields().lookupField(field_name)

        classes = {}

        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0
        for current, feature in enumerate(features):
            if feedback.isCanceled():
                break

            feedback.setProgress(int(current * total))

            attributes = feature.attributes()
            clazz = attributes[field_index]

            if clazz not in classes:
                classes[clazz] = len(list(classes.keys()))

            attributes.append(classes[clazz])
            feature.setAttributes(attributes)
            sink.addFeature(feature, QgsFeatureSink.FastInsert)

        return {self.OUTPUT: dest_id}
