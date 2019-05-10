# -*- coding: utf-8 -*-

"""
***************************************************************************
    EquidistanceBuffer.py
    --------------------
    Date                 : April 2019
    Copyright            : (C) 2019 by Ujaval Gandhi
    Email                : ujaval at spatialthoughts dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Ujaval Gandhi'
__date__ = 'April 2019'
__copyright__ = '(C) 2019, Ujaval Gandhi'

# This will get replaced with a git SHA1 when you do a git archive323

__revision__ = '$Format:%H$'

from qgis.core import (QgsGeometry,
                       QgsPoint,
                       QgsFeature,
                       QgsWkbTypes,
                       QgsProcessing,
                       QgsProcessingParameterDistance,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterEnum,
                       QgsProcessingException,
                       QgsCoordinateReferenceSystem,
                       QgsCoordinateTransform,
                       QgsProject,
                       QgsUnitTypes
                       )

from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm


class EquidistanceBuffer(QgisFeatureBasedAlgorithm):
    """
    This algorithm creates equidistance buffers for vector layers.

    Each geometry is transformed to a Azimuthal Equidistant projection centered
    at that geometry, buffered and transformed back to the original projection.
    """
    DISTANCE = 'DISTANCE'
    SEGMENTS = 'SEGMENTS'
    END_CAP_STYLE = 'END_CAP_STYLE'
    JOIN_STYLE = 'JOIN_STYLE'
    MITER_LIMIT = 'MITER_LIMIT'

    def group(self):
        return self.tr('Vector geometry')

    def groupId(self):
        return 'vectorgeometry'

    def __init__(self):
        super().__init__()
        self.distance = None
        self.segments = None
        self.join_style = None
        self.miter_limit = None
        self.join_styles = [self.tr('Round'),
                            self.tr('Miter'),
                            self.tr('Bevel')]
        self.end_cap_styles = [self.tr('Round'),
                               self.tr('Flat'),
                               self.tr('Square')]

    def initParameters(self, config=None):
        distanceParam = QgsProcessingParameterDistance(self.DISTANCE,
                                                       self.tr('Distance'),
                                                       defaultValue=10000.0)
        distanceParam.setDefaultUnit(QgsUnitTypes.DistanceMeters)
        self.addParameter(distanceParam)

        self.addParameter(
            QgsProcessingParameterNumber(
                self.SEGMENTS,
                self.tr('Segments'),
                QgsProcessingParameterNumber.Integer,
                minValue=1,
                defaultValue=8))

        self.addParameter(QgsProcessingParameterEnum(
            self.END_CAP_STYLE,
            self.tr('End cap style'),
            options=self.end_cap_styles, defaultValue=0))

        self.addParameter(QgsProcessingParameterEnum(
            self.JOIN_STYLE,
            self.tr('Join style'),
            options=self.join_styles, defaultValue=0))

        self.addParameter(
            QgsProcessingParameterNumber(
                self.MITER_LIMIT,
                self.tr('Miter limit'),
                QgsProcessingParameterNumber.Double,
                minValue=1,
                defaultValue=2))

    def name(self):
        return 'equidistancebuffer'

    def displayName(self):
        return self.tr('Equidistance buffer')

    def outputName(self):
        return self.tr('Buffer')

    def inputLayerTypes(self):
        return [QgsProcessing.TypeVector]

    def outputType(self):
        return QgsProcessing.TypeVectorPolygon

    def outputWkbType(self, input_wkb_type):
        return QgsWkbTypes.Polygon

    def prepareAlgorithm(self, parameters, context, feedback):
        self.distance = self.parameterAsDouble(
            parameters, self.DISTANCE, context)
        self.segments = self.parameterAsInt(parameters, self.SEGMENTS, context)
        self.end_cap_style = self.parameterAsEnum(
            parameters, self.END_CAP_STYLE, context) + 1
        self.join_style = self.parameterAsEnum(
            parameters, self.JOIN_STYLE, context) + 1
        self.miter_limit = self.parameterAsDouble(
            parameters, self.MITER_LIMIT, context)

        source = self.parameterAsSource(parameters, 'INPUT', context)
        self.source_crs = source.sourceCrs()
        if not self.source_crs.isGeographic():
            feedback.reportError(
                self.tr(
                    'Layer CRS must be a Geographic CRS.'))
            return False
        return super().prepareAlgorithm(parameters, context, feedback)

    def processFeature(self, feature, context, feedback):
        geometry = feature.geometry()
        # For point features, centroid() returns the point itself
        centroid = geometry.centroid()
        x = centroid.asPoint().x()
        y = centroid.asPoint().y()
        proj_string = ('PROJ4:+proj=aeqd +ellps=WGS84 +lat_0={} +lon_0={}'
                       ' +x_0=0 +y_0=0').format(y, x)
        dest_crs = QgsCoordinateReferenceSystem(proj_string)
        xform = QgsCoordinateTransform(
            self.source_crs, dest_crs, QgsProject.instance())
        geometry.transform(xform)
        buffer = geometry.buffer(
            self.distance,
            self.segments,
            self.end_cap_style,
            self.join_style,
            self.miter_limit)
        buffer.transform(xform, QgsCoordinateTransform.ReverseTransform)
        feature.setGeometry(buffer)
        return [feature]
