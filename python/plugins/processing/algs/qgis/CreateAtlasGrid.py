# -*- coding: utf-8 -*-

"""
/***************************************************************************
 CreateAtlasGrid.py
                              -------------------
        begin                : 2019-02-27
        copyright            : (C) 2019 by fpsampayo
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
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterDistance,
                       QgsProcessingParameterLayout,
                       QgsProcessingParameterLayoutItem,
                       QgsLayoutItemRegistry,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsFields,
                       QgsField,
                       QgsWkbTypes,
                       QgsFeature,
                       QgsGeometry,
                       QgsFeatureRequest)


class CreateAtlasGrid(QgisAlgorithm):

    LAYOUT = 'LAYOUT'
    MAP = 'MAP'
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

        param = QgsProcessingParameterLayout(self.LAYOUT, 'Print layout')
        self.addParameter(param)
        map_param = QgsProcessingParameterLayoutItem(self.MAP, 'Map item', parentLayoutParameterName=self.LAYOUT,
                                                     itemType=QgsLayoutItemRegistry.LayoutMap)
        self.addParameter(map_param)
        self.addParameter(QgsProcessingParameterFeatureSource(self.COVERAGE_LAYER,
                                                              self.tr('Coverage layer'),
                                                              [QgsProcessing.TypeMapLayer]))
        self.addParameter(QgsProcessingParameterDistance(self.COVERAGE_BUFFER,
                                                         self.tr('Coverage buffer'), parentParameterName='COVERAGE_LAYER',
                                                         defaultValue=0.0))
        self.addParameter(QgsProcessingParameterBoolean(self.ONLY_ON_FEATURES,
                                                        self.tr('Only on intersected features'),
                                                        defaultValue=True))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                            self.tr('Grid'),
                                                            QgsProcessing.TypeVectorPolygon))

    def prepareAlgorithm(self, parameters, context, feedback):

        layout = self.parameterAsLayout(parameters, self.LAYOUT, context)
        if layout is None:
            raise QgsProcessingException('Cannot find layout with name "{}"'.format(parameters[self.LAYOUT]))

        map = self.parameterAsLayoutItem(parameters, self.MAP, context, layout)
        if map is None:
            raise QgsProcessingException('Cannot find matching map item with ID {}'.format(parameters[self.MAP]))

        self.mapheight = map.extent().height()
        self.mapwidth = map.extent().width()

        self.coverage_layer = self.parameterAsVectorLayer(
            parameters,
            self.COVERAGE_LAYER,
            context
        )
        coverage_distance = self.parameterAsDouble(
            parameters,
            self.COVERAGE_BUFFER,
            context
        )
        self.only_on_features = self.parameterAsBool(
            parameters,
            self.ONLY_ON_FEATURES,
            context
        )
        self.coverage = self.coverage_layer.sourceExtent().buffered(coverage_distance)

        return True

    def processAlgorithm(self, parameters, context, feedback):

        fields = QgsFields()
        fields.append(QgsField('id', QVariant.Int, '', 10, 0))
        (sink, dest_id) = self.parameterAsSink(
            parameters,
            self.OUTPUT,
            context,
            fields,
            QgsWkbTypes.Polygon,
            self.coverage_layer.sourceCrs()
        )

        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        xmin = self.coverage.xMinimum()
        xmax = self.coverage.xMaximum()
        remainder = (self.mapwidth - ((xmax - xmin) % self.mapwidth)) / 2

        yMax = self.coverage.yMaximum()
        yMin = yMax - self.mapheight

        n = 1
        current_y = 0
        # Columns loop
        while current_y < self.coverage.height():
            current_x = 0
            xMin = self.coverage.xMinimum() - remainder
            xMax = xMin + self.mapwidth
            # Lines loop
            while current_x < self.coverage.width():
                geom = QgsRectangle(xMin, yMin, xMax, yMax)
                output_feature = QgsFeature()
                output_feature.setGeometry(QgsGeometry.fromRect(geom))
                output_feature.setAttributes([n])
                if self.only_on_features:
                    fr = QgsFeatureRequest(geom)
                    fr.setFlags(QgsFeatureRequest.ExactIntersect)
                    fr.setNoAttributes()
                    fr.setLimit(1)

                    if len(list(self.coverage_layer.getFeatures(fr))) > 0:
                        sink.addFeatures([output_feature])
                        n += 1
                else:
                    sink.addFeatures([output_feature])
                    n += 1
                xMin = xMax
                xMax = xMax + self.mapwidth
                current_x += self.mapwidth
            yMax = yMin
            yMin = yMin - self.mapheight
            current_y += self.mapheight

        return {self.OUTPUT: dest_id}
