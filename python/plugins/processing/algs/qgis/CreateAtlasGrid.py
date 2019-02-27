# -*- coding: utf-8 -*-

"""
/***************************************************************************
 CreateAtlasGrid.py
                              -------------------
        begin                : 2019-02-27
        copyright            : (C) 20169 by fpsampayo
        email                : fpsampayo@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""

__author__ = 'Francisco Pérez Sampayo'
__date__ = '2019-02-27'
__copyright__ = '(C) 2019 by fpsampayo'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

from qgis.PyQt.QtCore import QVariant
from qgis.core import (QgsRectangle,
                       QgsProject,
                       QgsProcessingException,
                       QgsProcessingParameterString,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterDistance,
                       QgsProcessing,
                       QgsFields,
                       QgsField,
                       QgsWkbTypes,
                       QgsFeature,
                       QgsGeometry)


class CreateAtlasGrid(QgisAlgorithm):

    LAYOUT = 'LAYOUT'
    COVERAGE_LAYER = 'COVERAGE_LAYER'
    COVERAGE_BUFFER = 'COVERAGE_BUFFER'
    ONLY_ON_FEATURES = 'ONLY_ON_FEATURES'
    OUTPUT = 'OUTPUT'

    def name(self):
        # Unique (non-user visible) name of algorithm
        return 'createatlasgrid'

    def displayName(self):
        # The name that the user will see in the toolbox
        return self.tr('Create a grid from map layout')

    def group(self):
        return self.tr('Cartography')

    def groupId(self):
        return 'cartography'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):

        layout_param = QgsProcessingParameterString(
            self.LAYOUT,
            description=self.tr('Layout to use'),
            defaultValue=None,
            optional=False)

        layout_param.setMetadata(
            {'widget_wrapper': {
                'class':
                    'processing.gui.wrappers_layout.LayoutWrapper'}})
        self.addParameter(layout_param)

        self.addParameter(QgsProcessingParameterFeatureSource(self.COVERAGE_LAYER,
                                                              self.tr('Coverage layer'),
                                                              [QgsProcessing.TypeMapLayer]))
        self.addParameter(QgsProcessingParameterDistance(self.COVERAGE_BUFFER,
                                                         self.tr('Coverage Buffer'), parentParameterName='COVERAGE_LAYER',
                                                         defaultValue=0.0))
        self.addParameter(QgsProcessingParameterBoolean(self.ONLY_ON_FEATURES,
                                                        self.tr('Only on intersection features'),
                                                        defaultValue=True))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                            self.tr('Grid'),
                                                            QgsProcessing.TypeVectorPolygon))

    def processAlgorithm(self, parameters, context, feedback):

        layout_name = self.parameterAsString(
            parameters,
            self.LAYOUT,
            context
        )
        coverage_layer = self.parameterAsVectorLayer(
            parameters,
            self.COVERAGE_LAYER,
            context
        )
        coverage_distance = self.parameterAsDouble(
            parameters,
            self.COVERAGE_BUFFER,
            context
        )
        only_on_features = self.parameterAsBool(
            parameters,
            self.ONLY_ON_FEATURES,
            context
        )

        fields = QgsFields()
        fields.append(QgsField('id', QVariant.Int, '', 10, 0))
        (sink, dest_id) = self.parameterAsSink(
            parameters,
            self.OUTPUT,
            context,
            fields,
            QgsWkbTypes.Polygon,
            coverage_layer.sourceCrs()
        )

        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        layout = QgsProject().instance().layoutManager().layoutByName(layout_name)
        map = layout.referenceMap()
        h = map.extent().height()
        w = map.extent().width()

        coverage = coverage_layer.sourceExtent().buffered(coverage_distance)

        yMax = coverage.yMaximum()
        yMin = yMax - h

        n = 1
        current_y = 0
        # Columns loop
        while current_y < coverage.height():
            current_x = 0
            xMin = coverage.xMinimum()
            xMax = xMin + w
            # Lines loop
            while current_x < coverage.width():
                geom = QgsRectangle(xMin, yMin, xMax, yMax)
                output_feature = QgsFeature()
                output_feature.setGeometry(QgsGeometry.fromRect(geom))
                output_feature.setAttributes([n])
                if only_on_features:
                    if len(list(coverage_layer.getFeatures(geom))) > 0:
                        sink.addFeatures([output_feature])
                        n += 1
                else:
                    sink.addFeatures([output_feature])
                    n += 1
                xMin = xMax
                xMax = xMax + w
                current_x += w
            yMax = yMin
            yMin = yMin - h
            current_y += h

        return {self.OUTPUT: dest_id}



