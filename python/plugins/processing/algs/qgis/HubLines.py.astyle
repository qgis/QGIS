# -*- coding: utf-8 -*-

"""
***************************************************************************
    Gridify.py
    ---------------------
    Date                 : May 2010
    Copyright            : (C) 2010 by Michael Minn
    Email                : pyqgis at michaelminn dot com
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

__author__ = 'Michael Minn'
__date__ = 'May 2010'
__copyright__ = '(C) 2010, Michael Minn'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsFeature,
                       QgsFeatureSink,
                       QgsGeometry,
                       QgsPointXY,
                       QgsWkbTypes,
                       QgsFeatureRequest,
                       QgsProcessing,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterField,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingException,
                       QgsExpression)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import vector


class HubLines(QgisAlgorithm):
    HUBS = 'HUBS'
    HUB_FIELD = 'HUB_FIELD'
    SPOKES = 'SPOKES'
    SPOKE_FIELD = 'SPOKE_FIELD'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector analysis')

    def __init__(self):
        super().__init__()

    def tags(self):
        return self.tr('join,points,lines,connect,hub,spoke').split(',')

    def initAlgorithm(self, config=None):

        self.addParameter(QgsProcessingParameterFeatureSource(self.HUBS,
                                                              self.tr('Hub layer')))
        self.addParameter(QgsProcessingParameterField(self.HUB_FIELD,
                                                      self.tr('Hub ID field'), parentLayerParameterName=self.HUBS))
        self.addParameter(QgsProcessingParameterFeatureSource(self.SPOKES,
                                                              self.tr('Spoke layer')))
        self.addParameter(QgsProcessingParameterField(self.SPOKE_FIELD,
                                                      self.tr('Spoke ID field'), parentLayerParameterName=self.SPOKES))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Hub lines'), QgsProcessing.TypeVectorLine))

    def name(self):
        return 'hublines'

    def displayName(self):
        return self.tr('Hub lines')

    def processAlgorithm(self, parameters, context, feedback):
        if parameters[self.SPOKES] == parameters[self.HUBS]:
            raise QgsProcessingException(
                self.tr('Same layer given for both hubs and spokes'))

        hub_source = self.parameterAsSource(parameters, self.HUBS, context)
        spoke_source = self.parameterAsSource(parameters, self.SPOKES, context)
        field_hub = self.parameterAsString(parameters, self.HUB_FIELD, context)
        field_hub_index = hub_source.fields().lookupField(field_hub)
        field_spoke = self.parameterAsString(parameters, self.SPOKE_FIELD, context)
        field_spoke_index = hub_source.fields().lookupField(field_spoke)

        fields = vector.combineFields(hub_source.fields(), spoke_source.fields())

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.LineString, hub_source.sourceCrs())

        hubs = hub_source.getFeatures()
        total = 100.0 / hub_source.featureCount() if hub_source.featureCount() else 0

        matching_field_types = hub_source.fields().at(field_hub_index).type() == spoke_source.fields().at(field_spoke_index).type()

        for current, hub_point in enumerate(hubs):
            if feedback.isCanceled():
                break

            if not hub_point.hasGeometry():
                continue

            p = hub_point.geometry().boundingBox().center()
            hub_x = p.x()
            hub_y = p.y()
            hub_id = str(hub_point[field_hub])
            hub_attributes = hub_point.attributes()

            request = QgsFeatureRequest().setDestinationCrs(hub_source.sourceCrs())
            if matching_field_types:
                request.setFilterExpression(QgsExpression.createFieldEqualityExpression(field_spoke, hub_attributes[field_hub_index]))

            spokes = spoke_source.getFeatures()
            for spoke_point in spokes:
                if feedback.isCanceled():
                    break

                spoke_id = str(spoke_point[field_spoke])
                if hub_id == spoke_id:
                    p = spoke_point.geometry().boundingBox().center()
                    spoke_x = p.x()
                    spoke_y = p.y()

                    f = QgsFeature()
                    f.setAttributes(hub_attributes + spoke_point.attributes())
                    f.setGeometry(QgsGeometry.fromPolyline(
                        [QgsPointXY(hub_x, hub_y), QgsPointXY(spoke_x, spoke_y)]))
                    sink.addFeature(f, QgsFeatureSink.FastInsert)

            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
