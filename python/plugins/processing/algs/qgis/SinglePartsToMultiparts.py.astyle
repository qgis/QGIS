# -*- coding: utf-8 -*-

"""
***************************************************************************
    SinglePartsToMultiparts.py
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

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsFeature,
                       QgsFeatureSink,
                       QgsGeometry,
                       QgsWkbTypes,
                       QgsProcessingUtils,
                       NULL,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterField,
                       QgsProcessingParameterFeatureSink)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class SinglePartsToMultiparts(QgisAlgorithm):

    INPUT = 'INPUT'
    FIELD = 'FIELD'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'single_to_multi.png'))

    def group(self):
        return self.tr('Vector geometry')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterField(self.FIELD,
                                                      self.tr('Unique ID field'), parentLayerParameterName=self.INPUT))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Multipart')))

    def name(self):
        return 'singlepartstomultipart'

    def displayName(self):
        return self.tr('Singleparts to multipart')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        field_name = self.parameterAsString(parameters, self.FIELD, context)

        geom_type = QgsWkbTypes.multiType(source.wkbType())

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), geom_type, source.sourceCrs())

        index = source.fields().lookupField(field_name)

        collection_geom = {}
        collection_attrs = {}

        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0
        for current, feature in enumerate(features):
            if feedback.isCanceled():
                break

            atMap = feature.attributes()
            idVar = atMap[index]
            if idVar in [None, NULL] or not feature.hasGeometry():
                sink.addFeature(feature, QgsFeatureSink.FastInsert)
                feedback.setProgress(int(current * total))
                continue

            key = str(idVar).strip()
            if key not in collection_geom:
                collection_geom[key] = []
                collection_attrs[key] = atMap

            inGeom = feature.geometry()
            collection_geom[key].append(inGeom)

            feedback.setProgress(int(current * total))

        for key, geoms in collection_geom.items():
            if feedback.isCanceled():
                break

            feature = QgsFeature()
            feature.setAttributes(collection_attrs[key])
            feature.setGeometry(QgsGeometry.collectGeometry(geoms))
            sink.addFeature(feature, QgsFeatureSink.FastInsert)

        return {self.OUTPUT: dest_id}
