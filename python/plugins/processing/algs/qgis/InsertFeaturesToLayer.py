# -*- coding: utf-8 -*-

"""
***************************************************************************
    InsertFeaturesToLayer.py
    ---------------------
    Date                 : Feb 2018
    Copyright            : (C) 2018 by Germán Carrillo
    Email                : gcarrillo at linuxmail dot org
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Germán Carrillo'
__date__ = 'Feb 2018'
__copyright__ = '(C) 2018, Germán Carrillo'

# This will get replaced with a git SHA1 when you do a git archive323

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant

from qgis.core import (
                       QgsGeometry,
                       QgsWkbTypes,
                       QgsProcessing,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterVectorLayer, 
                       QgsProcessingOutputVectorLayer,
                       QgsProject,
                       QgsVectorLayerUtils
                       )
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class InsertFeaturesToLayer(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    OUTPUT_ = 'OUTPUT_'

    def group(self):
        return self.tr('Vector table')

    def groupId(self):
        return 'vectortable'

    def tags(self):
        return self.tr('append,copy,insert,features,paste').split(',')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'),
                                                              [QgsProcessing.TypeVector]))
        self.addParameter(QgsProcessingParameterVectorLayer(self.OUTPUT,
                                                              self.tr('Output layer'),
                                                              [QgsProcessing.TypeVector]))
        self.addOutput(QgsProcessingOutputVectorLayer(self.OUTPUT,
                                                        self.tr('Output layer with new features')))

    def name(self):
        return 'insertfeaturestolayer'

    def displayName(self):
        return self.tr('Insert features to layer')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        target = self.parameterAsVectorLayer(parameters, self.OUTPUT, context)

        # Define a mapping between source and target layer
        mapping = dict()
        for target_idx in target.fields().allAttributesList():
            target_field = target.fields().field(target_idx)
            source_idx = source.fields().indexOf(target_field.name())
            if source_idx != -1:
                mapping[target_idx] = source_idx

        # Copy and Paste
        total = 100.0 / source.featureCount() if source.featureCount() else 0
        features = source.getFeatures()
        destType = target.geometryType()
        destIsMulti = QgsWkbTypes.isMultiType(target.wkbType())
        new_features = []
        for current, in_feature in enumerate(features):
            if feedback.isCanceled():
                break

            attrs = {target_idx: in_feature[source_idx] for target_idx, source_idx in mapping.items()}

            geom = QgsGeometry()

            if in_feature.hasGeometry():
                # Convert geometry to match destination layer
                # Adapted from QGIS qgisapp.cpp, pasteFromClipboard()
                geom = in_feature.geometry()

                if destType != QgsWkbTypes.UnknownGeometry:
                    newGeometry = geom.convertToType(destType, destIsMulti)

                    if newGeometry.isNull():
                        continue
                    geom = newGeometry

                # Avoid intersection if enabled in digitize settings
                geom.avoidIntersections(QgsProject.instance().avoidIntersectionsLayers())

            new_feature = QgsVectorLayerUtils().createFeature(target, geom, attrs)
            new_features.append(new_feature)

            feedback.setProgress(int(current * total))

        target.dataProvider().enterUpdateMode()
        try:
            target.dataProvider().addFeatures(new_features)
        except BaseException as e:
            raise e
        finally:
            target.dataProvider().leaveUpdateMode()

        feedback.pushInfo("{} out of {} features from input layer were successfully copied into '{}'!".format(
            len(new_features),
            source.featureCount(),
            target.name()
        ))

        return {self.OUTPUT: target}
