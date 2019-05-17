# -*- coding: utf-8 -*-

"""
***************************************************************************
    ExportGeometryInfo.py
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

import os
import math

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant

from qgis.core import (NULL,
                       QgsApplication,
                       QgsCoordinateTransform,
                       QgsField,
                       QgsFields,
                       QgsWkbTypes,
                       QgsPointXY,
                       QgsFeatureSink,
                       QgsDistanceArea,
                       QgsProcessingUtils,
                       QgsProcessingException,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterFeatureSink)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class ExportGeometryInfo(QgisAlgorithm):

    INPUT = 'INPUT'
    METHOD = 'CALC_METHOD'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QgsApplication.getThemeIcon("/algorithms/mAlgorithmAddGeometryAttributes.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("/algorithms/mAlgorithmAddGeometryAttributes.svg")

    def tags(self):
        return self.tr('export,add,information,measurements,areas,lengths,perimeters,latitudes,longitudes,x,y,z,extract,points,lines,polygons,sinuosity,fields').split(',')

    def group(self):
        return self.tr('Vector geometry')

    def groupId(self):
        return 'vectorgeometry'

    def __init__(self):
        super().__init__()
        self.export_z = False
        self.export_m = False
        self.distance_area = None
        self.calc_methods = [self.tr('Layer CRS'),
                             self.tr('Project CRS'),
                             self.tr('Ellipsoidal')]

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterEnum(self.METHOD,
                                                     self.tr('Calculate using'), options=self.calc_methods, defaultValue=0))
        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Added geom info')))

    def name(self):
        return 'exportaddgeometrycolumns'

    def displayName(self):
        return self.tr('Add geometry attributes')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        method = self.parameterAsEnum(parameters, self.METHOD, context)

        wkb_type = source.wkbType()
        fields = source.fields()

        new_fields = QgsFields()
        if QgsWkbTypes.geometryType(wkb_type) == QgsWkbTypes.PolygonGeometry:
            new_fields.append(QgsField('area', QVariant.Double))
            new_fields.append(QgsField('perimeter', QVariant.Double))
        elif QgsWkbTypes.geometryType(wkb_type) == QgsWkbTypes.LineGeometry:
            new_fields.append(QgsField('length', QVariant.Double))
            if not QgsWkbTypes.isMultiType(source.wkbType()):
                new_fields.append(QgsField('straightdis', QVariant.Double))
                new_fields.append(QgsField('sinuosity', QVariant.Double))
        else:
            if QgsWkbTypes.isMultiType(source.wkbType()):
                new_fields.append(QgsField('numparts', QVariant.Int))
            else:
                new_fields.append(QgsField('xcoord', QVariant.Double))
                new_fields.append(QgsField('ycoord', QVariant.Double))
                if QgsWkbTypes.hasZ(source.wkbType()):
                    self.export_z = True
                    new_fields.append(QgsField('zcoord', QVariant.Double))
                if QgsWkbTypes.hasM(source.wkbType()):
                    self.export_m = True
                    new_fields.append(QgsField('mvalue', QVariant.Double))

        fields = QgsProcessingUtils.combineFields(fields, new_fields)
        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, wkb_type, source.sourceCrs())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        coordTransform = None

        # Calculate with:
        # 0 - layer CRS
        # 1 - project CRS
        # 2 - ellipsoidal

        self.distance_area = QgsDistanceArea()
        if method == 2:
            self.distance_area.setSourceCrs(source.sourceCrs(), context.transformContext())
            self.distance_area.setEllipsoid(context.project().ellipsoid())
        elif method == 1:
            coordTransform = QgsCoordinateTransform(source.sourceCrs(), context.project().crs(), context.project())

        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0
        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            outFeat = f
            attrs = f.attributes()
            inGeom = f.geometry()
            if inGeom:
                if coordTransform is not None:
                    inGeom.transform(coordTransform)

                if inGeom.type() == QgsWkbTypes.PointGeometry:
                    attrs.extend(self.point_attributes(inGeom))
                elif inGeom.type() == QgsWkbTypes.PolygonGeometry:
                    attrs.extend(self.polygon_attributes(inGeom))
                else:
                    attrs.extend(self.line_attributes(inGeom))

            # ensure consistent count of attributes - otherwise null
            # geometry features will have incorrect attribute length
            # and provider may reject them
            if len(attrs) < len(fields):
                attrs += [NULL] * (len(fields) - len(attrs))

            outFeat.setAttributes(attrs)
            sink.addFeature(outFeat, QgsFeatureSink.FastInsert)

            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}

    def point_attributes(self, geometry):
        attrs = []
        if not geometry.isMultipart():
            pt = geometry.constGet()
            attrs.append(pt.x())
            attrs.append(pt.y())
            # add point z/m
            if self.export_z:
                attrs.append(pt.z())
            if self.export_m:
                attrs.append(pt.m())
        else:
            attrs = [geometry.constGet().numGeometries()]
        return attrs

    def line_attributes(self, geometry):
        if geometry.isMultipart():
            return [self.distance_area.measureLength(geometry)]
        else:
            curve = geometry.constGet()
            p1 = curve.startPoint()
            p2 = curve.endPoint()
            straight_distance = self.distance_area.measureLine(QgsPointXY(p1), QgsPointXY(p2))
            sinuosity = curve.sinuosity()
            if math.isnan(sinuosity):
                sinuosity = NULL
            return [self.distance_area.measureLength(geometry), straight_distance, sinuosity]

    def polygon_attributes(self, geometry):
        area = self.distance_area.measureArea(geometry)
        perimeter = self.distance_area.measurePerimeter(geometry)
        return [area, perimeter]
