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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant

from qgis.core import (QgsCoordinateTransform,
                       QgsField,
                       QgsWkbTypes,
                       QgsFeatureSink,
                       QgsDistanceArea,
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
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'export_geometry.png'))

    def tags(self):
        return self.tr('export,add,information,measurements,areas,lengths,perimeters,latitudes,longitudes,x,y,z,extract,points,lines,polygons').split(',')

    def group(self):
        return self.tr('Vector geometry')

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
        return self.tr('Export geometry columns')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        method = self.parameterAsEnum(parameters, self.METHOD, context)

        wkb_type = source.wkbType()
        fields = source.fields()

        if QgsWkbTypes.geometryType(wkb_type) == QgsWkbTypes.PolygonGeometry:
            areaName = vector.createUniqueFieldName('area', fields)
            fields.append(QgsField(areaName, QVariant.Double))
            perimeterName = vector.createUniqueFieldName('perimeter', fields)
            fields.append(QgsField(perimeterName, QVariant.Double))
        elif QgsWkbTypes.geometryType(wkb_type) == QgsWkbTypes.LineGeometry:
            lengthName = vector.createUniqueFieldName('length', fields)
            fields.append(QgsField(lengthName, QVariant.Double))
        else:
            xName = vector.createUniqueFieldName('xcoord', fields)
            fields.append(QgsField(xName, QVariant.Double))
            yName = vector.createUniqueFieldName('ycoord', fields)
            fields.append(QgsField(yName, QVariant.Double))
            if QgsWkbTypes.hasZ(source.wkbType()):
                self.export_z = True
                zName = vector.createUniqueFieldName('zcoord', fields)
                fields.append(QgsField(zName, QVariant.Double))
            if QgsWkbTypes.hasM(source.wkbType()):
                self.export_m = True
                zName = vector.createUniqueFieldName('mvalue', fields)
                fields.append(QgsField(zName, QVariant.Double))

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, wkb_type, source.sourceCrs())

        coordTransform = None

        # Calculate with:
        # 0 - layer CRS
        # 1 - project CRS
        # 2 - ellipsoidal

        self.distance_area = QgsDistanceArea()
        if method == 2:
            self.distance_area.setSourceCrs(source.sourceCrs())
            self.distance_area.setEllipsoid(context.project().ellipsoid())
        elif method == 1:
            coordTransform = QgsCoordinateTransform(source.sourceCrs(), context.project().crs())

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

            outFeat.setAttributes(attrs)
            sink.addFeature(outFeat, QgsFeatureSink.FastInsert)

            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}

    def point_attributes(self, geometry):
        pt = None
        if not geometry.isMultipart():
            pt = geometry.geometry()
        else:
            if geometry.numGeometries() > 0:
                pt = geometry.geometryN(0)
        attrs = []
        if pt:
            attrs.append(pt.x())
            attrs.append(pt.y())
            # add point z/m
            if self.export_z:
                attrs.append(pt.z())
            if self.export_m:
                attrs.append(pt.m())
        return attrs

    def line_attributes(self, geometry):
        return [self.distance_area.measureLength(geometry)]

    def polygon_attributes(self, geometry):
        area = self.distance_area.measureArea(geometry)
        perimeter = self.distance_area.measurePerimeter(geometry)
        return [area, perimeter]
