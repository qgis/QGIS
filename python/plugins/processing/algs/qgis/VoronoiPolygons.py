# -*- coding: utf-8 -*-

"""
***************************************************************************
    VoronoiPolygons.py
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

from qgis.core import (QgsApplication,
                       QgsFeatureRequest,
                       QgsFeatureSink,
                       QgsFeature,
                       QgsGeometry,
                       QgsPointXY,
                       QgsWkbTypes,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterNumber)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

from . import voronoi

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class VoronoiPolygons(QgisAlgorithm):

    INPUT = 'INPUT'
    BUFFER = 'BUFFER'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QgsApplication.getThemeIcon("/algorithms/mAlgorithmVoronoi.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("/algorithms/mAlgorithmVoronoi.svg")

    def group(self):
        return self.tr('Vector geometry')

    def groupId(self):
        return 'vectorgeometry'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(
            self.INPUT, self.tr('Input layer'), [QgsProcessing.TypeVectorPoint]))
        self.addParameter(
            QgsProcessingParameterNumber(
                self.BUFFER, self.tr('Buffer region (% of extent)'),
                minValue=0.0, maxValue=9999999999, defaultValue=0.0))

        self.addParameter(QgsProcessingParameterFeatureSink(
            self.OUTPUT, self.tr('Voronoi polygons'), type=QgsProcessing.TypeVectorPolygon))

    def name(self):
        return 'voronoipolygons'

    def displayName(self):
        return self.tr('Voronoi polygons')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(
                self.invalidSourceError(parameters, self.INPUT))

        buf = self.parameterAsDouble(parameters, self.BUFFER, context)
        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), QgsWkbTypes.Polygon, source.sourceCrs())
        if sink is None:
            raise QgsProcessingException(
                self.invalidSinkError(parameters, self.OUTPUT))

        outFeat = QgsFeature()
        extent = source.sourceExtent()
        extraX = extent.width() * (buf / 100.0)
        # Adjust the extent
        extent.setXMinimum(extent.xMinimum() - extraX)
        extent.setXMaximum(extent.xMaximum() + extraX)
        extraY = extent.height() * (buf / 100.0)
        extent.setYMinimum(extent.yMinimum() - extraY)
        extent.setYMaximum(extent.yMaximum() + extraY)
        height = extent.height()
        width = extent.width()
        c = voronoi.Context()
        pts = []
        ptDict = {}
        ptNdx = -1

        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0
        for current, inFeat in enumerate(features):
            if feedback.isCanceled():
                break
            geom = inFeat.geometry()
            point = geom.asPoint()
            x = point.x() - extent.xMinimum()
            y = point.y() - extent.yMinimum()
            pts.append((x, y))
            ptNdx += 1
            ptDict[ptNdx] = inFeat.id()
            feedback.setProgress(int(current * total))

        if len(pts) < 3:
            raise QgsProcessingException(
                self.tr('Input file should contain at least 3 points. Choose '
                        'another file and try again.'))

        uniqueSet = set(item for item in pts)
        ids = [pts.index(item) for item in uniqueSet]
        sl = voronoi.SiteList([voronoi.Site(i[0], i[1], sitenum=j) for (j,
                                                                        i) in enumerate(uniqueSet)])
        voronoi.voronoi(sl, c)
        inFeat = QgsFeature()

        current = 0
        if len(c.polygons) == 0:
            raise QgsProcessingException(
                self.tr('There were no polygons created.'))

        total = 100.0 / len(c.polygons)

        for (site, edges) in list(c.polygons.items()):
            if feedback.isCanceled():
                break

            request = QgsFeatureRequest().setFilterFid(ptDict[ids[site]])
            inFeat = next(source.getFeatures(request))
            lines = self.clip_voronoi(edges, c, width, height, extent)

            geom = QgsGeometry.fromMultiPointXY(lines)
            geom = QgsGeometry(geom.convexHull())
            outFeat.setGeometry(geom)
            outFeat.setAttributes(inFeat.attributes())
            sink.addFeature(outFeat, QgsFeatureSink.FastInsert)

            current += 1
            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}

    def clip_voronoi(self, edges, c, width, height, extent):
        """Clip voronoi function based on code written for Inkscape.
        Copyright (C) 2010 Alvin Penner, penner@vaxxine.com
        """

        def clip_line(x1, y1, x2, y2, w, h):
            if x1 < 0 and x2 < 0:
                return [0, 0, 0, 0]
            if x1 > w and x2 > w:
                return [0, 0, 0, 0]
            if x1 < 0:
                y1 = (y1 * x2 - y2 * x1) / (x2 - x1)
                x1 = 0
            if x2 < 0:
                y2 = (y1 * x2 - y2 * x1) / (x2 - x1)
                x2 = 0
            if x1 > w:
                y1 = y1 + (w - x1) * (y2 - y1) / (x2 - x1)
                x1 = w
            if x2 > w:
                y2 = y1 + (w - x1) * (y2 - y1) / (x2 - x1)
                x2 = w
            if y1 < 0 and y2 < 0:
                return [0, 0, 0, 0]
            if y1 > h and y2 > h:
                return [0, 0, 0, 0]
            if x1 == x2 and y1 == y2:
                return [0, 0, 0, 0]
            if y1 < 0:
                x1 = (x1 * y2 - x2 * y1) / (y2 - y1)
                y1 = 0
            if y2 < 0:
                x2 = (x1 * y2 - x2 * y1) / (y2 - y1)
                y2 = 0
            if y1 > h:
                x1 = x1 + (h - y1) * (x2 - x1) / (y2 - y1)
                y1 = h
            if y2 > h:
                x2 = x1 + (h - y1) * (x2 - x1) / (y2 - y1)
                y2 = h
            return [x1, y1, x2, y2]

        lines = []
        hasXMin = False
        hasYMin = False
        hasXMax = False
        hasYMax = False
        for edge in edges:
            if edge[1] >= 0 and edge[2] >= 0:
                # Two vertices
                [x1, y1, x2, y2] = clip_line(
                    c.vertices[edge[1]][0],
                    c.vertices[edge[1]][1],
                    c.vertices[edge[2]][0],
                    c.vertices[edge[2]][1],
                    width,
                    height
                )
            elif edge[1] >= 0:
                # Only one vertex
                if c.lines[edge[0]][1] == 0:
                    # Vertical line
                    xtemp = c.lines[edge[0]][2] / c.lines[edge[0]][0]
                    if c.vertices[edge[1]][1] > height / 2:
                        ytemp = height
                    else:
                        ytemp = 0
                else:
                    xtemp = width
                    ytemp = (c.lines[edge[0]][2] - width *
                             c.lines[edge[0]][0]) / c.lines[edge[0]][1]
                [x1, y1, x2, y2] = clip_line(
                    c.vertices[edge[1]][0],
                    c.vertices[edge[1]][1],
                    xtemp,
                    ytemp,
                    width,
                    height
                )
            elif edge[2] >= 0:
                # Only one vertex
                if c.lines[edge[0]][1] == 0:
                    # Vertical line
                    xtemp = c.lines[edge[0]][2] / c.lines[edge[0]][0]
                    if c.vertices[edge[2]][1] > height / 2:
                        ytemp = height
                    else:
                        ytemp = 0.0
                else:
                    xtemp = 0.0
                    ytemp = c.lines[edge[0]][2] / c.lines[edge[0]][1]
                [x1, y1, x2, y2] = clip_line(
                    xtemp,
                    ytemp,
                    c.vertices[edge[2]][0],
                    c.vertices[edge[2]][1],
                    width,
                    height,
                )
            if x1 or x2 or y1 or y2:
                lines.append(QgsPointXY(x1 + extent.xMinimum(),
                                        y1 + extent.yMinimum()))
                lines.append(QgsPointXY(x2 + extent.xMinimum(),
                                        y2 + extent.yMinimum()))
                if 0 in (x1, x2):
                    hasXMin = True
                if 0 in (y1, y2):
                    hasYMin = True
                if height in (y1, y2):
                    hasYMax = True
                if width in (x1, x2):
                    hasXMax = True
        if hasXMin:
            if hasYMax:
                lines.append(QgsPointXY(extent.xMinimum(),
                                        height + extent.yMinimum()))
            if hasYMin:
                lines.append(QgsPointXY(extent.xMinimum(),
                                        extent.yMinimum()))
        if hasXMax:
            if hasYMax:
                lines.append(QgsPointXY(width + extent.xMinimum(),
                                        height + extent.yMinimum()))
            if hasYMin:
                lines.append(QgsPointXY(width + extent.xMinimum(),
                                        extent.yMinimum()))
        return lines
