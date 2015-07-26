# -*- coding: utf-8 -*-

"""
***************************************************************************
    Delaunay.py
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

from sets import Set
from PyQt4.QtCore import QVariant
from qgis.core import QGis, QgsField, QgsFeatureRequest, QgsFeature, QgsGeometry, QgsPoint
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.tools import dataobjects, vector
from processing.core.parameters import ParameterVector
from processing.core.outputs import OutputVector
import voronoi


class Delaunay(GeoAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Delaunay triangulation')
        self.group, self.i18n_group = self.trAlgorithm('Vector geometry tools')

        self.addParameter(ParameterVector(self.INPUT,
            self.tr('Input layer'), [ParameterVector.VECTOR_TYPE_POINT]))

        self.addOutput(OutputVector(self.OUTPUT,
            self.tr('Delaunay triangulation')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT))

        fields = [QgsField('POINTA', QVariant.Double, '', 24, 15),
                  QgsField('POINTB', QVariant.Double, '', 24, 15),
                  QgsField('POINTC', QVariant.Double, '', 24, 15)]

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fields,
                QGis.WKBPolygon, layer.crs())

        pts = []
        ptDict = {}
        ptNdx = -1
        c = voronoi.Context()
        features = vector.features(layer)
        for inFeat in features:
            geom = QgsGeometry(inFeat.geometry())
            point = geom.asPoint()
            x = point.x()
            y = point.y()
            pts.append((x, y))
            ptNdx += 1
            ptDict[ptNdx] = inFeat.id()

        if len(pts) < 3:
            raise GeoAlgorithmExecutionException(
                self.tr('Input file should contain at least 3 points. Choose '
                        'another file and try again.'))

        uniqueSet = Set(item for item in pts)
        ids = [pts.index(item) for item in uniqueSet]
        sl = voronoi.SiteList([voronoi.Site(*i) for i in uniqueSet])
        c.triangulate = True
        voronoi.voronoi(sl, c)
        triangles = c.triangles
        feat = QgsFeature()

        current = 0
        total = 100.0 / float(len(triangles))

        for triangle in triangles:
            indicies = list(triangle)
            indicies.append(indicies[0])
            polygon = []
            attrs = []
            step = 0
            for index in indicies:
                request = QgsFeatureRequest().setFilterFid(ptDict[ids[index]])
                inFeat = layer.getFeatures(request).next()
                geom = QgsGeometry(inFeat.geometry())
                point = QgsPoint(geom.asPoint())
                polygon.append(point)
                if step <= 3:
                    attrs.append(ids[index])
                step += 1
            feat.setAttributes(attrs)
            geometry = QgsGeometry().fromPolygon([polygon])
            feat.setGeometry(geometry)
            writer.addFeature(feat)
            current += 1
            progress.setPercentage(int(current * total))

        del writer
