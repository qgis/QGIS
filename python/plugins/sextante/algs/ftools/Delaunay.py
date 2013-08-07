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

from PyQt4.QtCore import *

from qgis.core import *

from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.core.QGisLayers import QGisLayers

from sextante.parameters.ParameterVector import ParameterVector

from sextante.outputs.OutputVector import OutputVector

from sextante.algs.ftools import voronoi


class Delaunay(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    #===========================================================================
    # def getIcon(self):
    #    return QtGui.QIcon(os.path.dirname(__file__) + "/icons/delaunay.png")
    #===========================================================================

    def defineCharacteristics(self):
        self.name = "Delaunay triangulation"
        self.group = "Vector geometry tools"

        self.addParameter(ParameterVector(self.INPUT, "Input layer", [ParameterVector.VECTOR_TYPE_POINT]))

        self.addOutput(OutputVector(self.OUTPUT, "Delaunay triangulation"))

    def processAlgorithm(self, progress):
        layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.INPUT))


        fields = [QgsField("POINTA", QVariant.Double, "", 24, 15),
                  QgsField("POINTB", QVariant.Double, "", 24, 15),
                  QgsField("POINTC", QVariant.Double, "", 24, 15)
                 ]

        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(fields,
                     QGis.WKBPolygon, layer.crs())

        pts = []
        ptDict = {}
        ptNdx = -1
        c = voronoi.Context()
        features = QGisLayers.features(layer)
        for inFeat in features:
            geom = QgsGeometry(inFeat.geometry())
            point = geom.asPoint()
            x = point.x()
            y = point.y()
            pts.append((x, y))
            ptNdx += 1
            ptDict[ptNdx] = inFeat.id()

        if len(pts) < 3:
            raise GeoAlgorithmExecutionException("Input file should contain at least 3 points. Choose another file and try again.")

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
