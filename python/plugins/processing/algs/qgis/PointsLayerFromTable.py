# -*- coding: utf-8 -*-

"""
***************************************************************************
    PointsLayerFromTable.py
    ---------------------
    Date                 : January 2013
    Copyright            : (C) 2013 by Victor Olaya
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
__date__ = 'August 2013'
__copyright__ = '(C) 2013, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsApplication,
                       QgsWkbTypes,
                       QgsPoint,
                       QgsFeatureRequest,
                       QgsGeometry,
                       QgsProcessing,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterField)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class PointsLayerFromTable(QgisAlgorithm):

    INPUT = 'INPUT'
    XFIELD = 'XFIELD'
    YFIELD = 'YFIELD'
    ZFIELD = 'ZFIELD'
    MFIELD = 'MFIELD'
    OUTPUT = 'OUTPUT'
    TARGET_CRS = 'TARGET_CRS'

    def tags(self):
        return self.tr('points,create,values,attributes').split(',')

    def group(self):
        return self.tr('Vector creation')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT, self.tr('Input layer'), types=[QgsProcessing.TypeVector]))

        self.addParameter(QgsProcessingParameterField(self.XFIELD,
                                                      self.tr('X field'), parentLayerParameterName=self.INPUT, type=QgsProcessingParameterField.Any))
        self.addParameter(QgsProcessingParameterField(self.YFIELD,
                                                      self.tr('Y field'), parentLayerParameterName=self.INPUT, type=QgsProcessingParameterField.Any))
        self.addParameter(QgsProcessingParameterField(self.ZFIELD,
                                                      self.tr('Z field'), parentLayerParameterName=self.INPUT, type=QgsProcessingParameterField.Any, optional=True))
        self.addParameter(QgsProcessingParameterField(self.MFIELD,
                                                      self.tr('M field'), parentLayerParameterName=self.INPUT, type=QgsProcessingParameterField.Any, optional=True))
        self.addParameter(QgsProcessingParameterCrs(self.TARGET_CRS,
                                                    self.tr('Target CRS'), defaultValue='EPSG:4326'))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Points from table'), type=QgsProcessing.TypeVectorPoint))

    def name(self):
        return 'createpointslayerfromtable'

    def displayName(self):
        return self.tr('Create points layer from table')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)

        fields = source.fields()
        x_field_index = fields.lookupField(self.parameterAsString(parameters, self.XFIELD, context))
        y_field_index = fields.lookupField(self.parameterAsString(parameters, self.YFIELD, context))
        z_field_index = -1
        if self.parameterAsString(parameters, self.ZFIELD, context):
            z_field_index = fields.lookupField(self.parameterAsString(parameters, self.ZFIELD, context))
        m_field_index = -1
        if self.parameterAsString(parameters, self.MFIELD, context):
            m_field_index = fields.lookupField(self.parameterAsString(parameters, self.MFIELD, context))

        wkb_type = QgsWkbTypes.Point
        if z_field_index >= 0:
            wkb_type = QgsWkbTypes.addZ(wkb_type)
        if m_field_index >= 0:
            wkb_type = QgsWkbTypes.addM(wkb_type)

        target_crs = self.parameterAsCrs(parameters, self.TARGET_CRS, context)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, wkb_type, target_crs)

        request = QgsFeatureRequest().setFlags(QgsFeatureRequest.NoGeometry)
        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0

        for current, feature in enumerate(features):
            if feedback.isCanceled():
                break

            feedback.setProgress(int(current * total))
            attrs = feature.attributes()

            try:
                x = float(attrs[x_field_index])
                y = float(attrs[y_field_index])

                point = QgsPoint(x, y)

                if z_field_index >= 0:
                    try:
                        point.addZValue(float(attrs[z_field_index]))
                    except:
                        point.addZValue(0.0)

                if m_field_index >= 0:
                    try:
                        point.addMValue(float(attrs[m_field_index]))
                    except:
                        point.addMValue(0.0)

                feature.setGeometry(QgsGeometry(point))
            except:
                pass  # no geometry

            sink.addFeature(feature)

        return {self.OUTPUT: dest_id}
