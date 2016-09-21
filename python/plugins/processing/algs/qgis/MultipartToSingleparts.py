# -*- coding: utf-8 -*-

"""
***************************************************************************
    MultipartToSingleparts.py
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

from qgis.core import QgsFeature, QgsGeometry, QgsWkbTypes

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class MultipartToSingleparts(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'multi_to_single.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Multipart to singleparts')
        self.group, self.i18n_group = self.trAlgorithm('Vector geometry tools')

        self.addParameter(ParameterVector(self.INPUT, self.tr('Input layer')))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Single parts')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT))
        geomType = self.multiToSingleGeom(layer.wkbType())

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            layer.fields().toList(), geomType, layer.crs())

        features = vector.features(layer)
        total = 100.0 / len(features)
        for current, f in enumerate(features):
            outFeat = QgsFeature()
            attrs = f.attributes()
            outFeat.setAttributes(attrs)

            inGeom = f.geometry()
            if inGeom:
                geometries = self.extractAsSingle(inGeom)

                for g in geometries:
                    outFeat.setGeometry(g)
                    writer.addFeature(outFeat)
            else:
                #input feature with null geometry
                writer.addFeature(outFeat)

            progress.setPercentage(int(current * total))

        del writer

    def multiToSingleGeom(self, wkbType):
        try:
            if wkbType in (QgsWkbTypes.Point, QgsWkbTypes.MultiPoint,
                           QgsWkbTypes.Point25D, QgsWkbTypes.MultiPoint25D):
                return QgsWkbTypes.Point
            elif wkbType in (QgsWkbTypes.LineString, QgsWkbTypes.MultiLineString,
                             QgsWkbTypes.MultiLineString25D,
                             QgsWkbTypes.LineString25D):

                return QgsWkbTypes.LineString
            elif wkbType in (QgsWkbTypes.Polygon, QgsWkbTypes.MultiPolygon,
                             QgsWkbTypes.MultiPolygon25D, QgsWkbTypes.Polygon25D):

                return QgsWkbTypes.Polygon
            else:
                return QgsWkbTypes.Unknown
        except Exception as err:
            raise GeoAlgorithmExecutionException(str(err))

    def extractAsSingle(self, geom):
        multiGeom = QgsGeometry()
        geometries = []
        if geom.type() == QgsWkbTypes.PointGeometry:
            if geom.isMultipart():
                multiGeom = geom.asMultiPoint()
                for i in multiGeom:
                    geometries.append(QgsGeometry().fromPoint(i))
            else:
                geometries.append(geom)
        elif geom.type() == QgsWkbTypes. LineGeometry:
            if geom.isMultipart():
                multiGeom = geom.asMultiPolyline()
                for i in multiGeom:
                    geometries.append(QgsGeometry().fromPolyline(i))
            else:
                geometries.append(geom)
        elif geom.type() == QgsWkbTypes.PolygonGeometry:
            if geom.isMultipart():
                multiGeom = geom.asMultiPolygon()
                for i in multiGeom:
                    geometries.append(QgsGeometry().fromPolygon(i))
            else:
                geometries.append(geom)
        return geometries
