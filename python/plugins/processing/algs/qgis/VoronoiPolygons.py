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

__authors__ = 'Victor Olaya, Håvard Tveite'
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
        self.addParameter(
            QgsProcessingParameterFeatureSource(
                self.INPUT, self.tr('Input layer'),
                [QgsProcessing.TypeVectorPoint]))
        self.addParameter(
            QgsProcessingParameterNumber(
                self.BUFFER, self.tr('Buffer region (% of extent)'),
                minValue=0.0, maxValue=9999999999, defaultValue=0.0))
        self.addParameter(
            QgsProcessingParameterFeatureSink(
                self.OUTPUT, self.tr('Voronoi polygons'),
                type=QgsProcessing.TypeVectorPolygon))

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
        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT,
                                               context, source.fields(),
                                               QgsWkbTypes.Polygon,
                                               source.sourceCrs())
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
        # Find the minimum and maximum x and y for the input points
        xmin = width
        xmax = 0
        ymin = height
        ymax = 0
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
            if x < xmin:
                xmin = x
            if y < ymin:
                ymin = y
            if x > xmax:
                xmax = x
            if y > ymax:
                ymax = y
            feedback.setProgress(int(current * total))
        if xmin == xmax or ymin == ymax:
            raise QgsProcessingException('The extent of the input points is '
                                         'not a polygon (all the points are '
                                         'on a vertical or horizontal line) '
                                         '- cannot make a Voronoi diagram!')
        xyminmax = [xmin, ymin, xmax, ymax]
        if len(pts) < 3:
            raise QgsProcessingException(
                self.tr('Input file should contain at least 3 points. Choose '
                        'another file and try again.'))
        # Eliminate duplicate points
        uniqueSet = set(item for item in pts)
        ids = [pts.index(item) for item in uniqueSet]
        sl = voronoi.SiteList([voronoi.Site(i[0], i[1], sitenum=j)
                               for (j, i) in enumerate(uniqueSet)])
        voronoi.voronoi(sl, c)
        if len(c.polygons) == 0:
            raise QgsProcessingException(
                self.tr('There were no polygons created.'))

        inFeat = QgsFeature()
        current = 0
        total = 100.0 / len(c.polygons)
        # Clip each of the generated "polygons"
        for (site, edges) in list(c.polygons.items()):
            if feedback.isCanceled():
                break
            request = QgsFeatureRequest().setFilterFid(ptDict[ids[site]])
            inFeat = next(source.getFeatures(request))
            boundarypoints = self.clip_voronoi(edges, c, width,
                                               height, extent,
                                               inFeat.geometry().asPoint(),
                                               xyminmax)
            ptgeom = QgsGeometry.fromMultiPointXY(boundarypoints)
            geom = QgsGeometry(ptgeom.convexHull())
            outFeat.setGeometry(geom)
            outFeat.setAttributes(inFeat.attributes())
            sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
            current += 1
            feedback.setProgress(int(current * total))
        return {self.OUTPUT: dest_id}

    def clip_voronoi(self, edges, c, width, height, extent, point, xyminmax):
        """Clip voronoi function based on code written for Inkscape.
        Copyright (C) 2010 Alvin Penner, penner@vaxxine.com
        Clips one Thiessen polygon (convex polygon) to extent
        """

        pt_x = point.x() - extent.xMinimum()
        pt_y = point.y() - extent.yMinimum()
        (xmin, ymin, xmax, ymax) = xyminmax

        def clip_line(x1, y1, x2, y2, w, h):
            if x1 < 0 and x2 < 0:
                # Completely to the left
                return [0, 0, 0, 0]
            if x1 > w and x2 > w:
                # Completely to the right
                return [0, 0, 0, 0]
            if y1 < 0 and y2 < 0:
                # Completely below
                return [0, 0, 0, 0]
            if y1 > h and y2 > h:
                # Completely above
                return [0, 0, 0, 0]
            if x1 < 0:
                # First point to the left
                y1 = (y1 * x2 - y2 * x1) / (x2 - x1)
                x1 = 0
            if x2 < 0:
                # Last point to the left
                y2 = (y1 * x2 - y2 * x1) / (x2 - x1)
                x2 = 0
            if x1 > w:
                # First point to the right
                y1 = y1 + (w - x1) * (y2 - y1) / (x2 - x1)
                x1 = w
            if x2 > w:
                # Last point to the right
                y2 = y1 + (w - x1) * (y2 - y1) / (x2 - x1)
                x2 = w
            if x1 == x2 and y1 == y2:
                return [0, 0, 0, 0]
            if y1 < 0:
                # First point below
                x1 = (x1 * y2 - x2 * y1) / (y2 - y1)
                y1 = 0
            if y2 < 0:
                # Second point below
                x2 = (x1 * y2 - x2 * y1) / (y2 - y1)
                y2 = 0
            if y1 > h:
                # First point above
                x1 = x1 + (h - y1) * (x2 - x1) / (y2 - y1)
                y1 = h
            if y2 > h:
                # Second point above
                x2 = x1 + (h - y1) * (x2 - x1) / (y2 - y1)
                y2 = h
            return [x1, y1, x2, y2]

        bndpoints = []
        hasXMin = False
        hasYMin = False
        hasXMax = False
        hasYMax = False
        XMinNumber = 0
        XMaxNumber = 0
        YMinNumber = 0
        YMaxNumber = 0
        # The same line may appear twice for collinear input points,
        # so have to remember which lines have contributed
        XMinLine = -1
        XMaxLine = -1
        YMinLine = -1
        YMaxLine = -1
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
                # Only one (left) vertex
                if c.lines[edge[0]][1] == 0:
                    # Vertical line
                    #xtemp = c.lines[edge[0]][2] / c.lines[edge[0]][0]
                    xtemp = c.vertices[edge[1]][0]
                    ytemp = 0 - 1
                    #if c.vertices[edge[1]][1] > height / 2:
                    #    ytemp = height
                    #else:
                    #    ytemp = 0
                else:
                    # Create an end of the line at the right edge - OK
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
                # Only one (right) vertex
                if c.lines[edge[0]][1] == 0:
                    # Vertical line
                    #xtemp = c.lines[edge[0]][2] / c.lines[edge[0]][0]
                    xtemp = c.vertices[edge[2]][0]
                    ytemp = height + 1
                    #if c.vertices[edge[2]][1] > height / 2:
                    #    ytemp = height
                    #else:
                    #    ytemp = 0.0
                else:
                    # End the line at the left edge - OK
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
            else:
                # No vertex, only a line
                if c.lines[edge[0]][1] == 0:
                    # Vertical line - should not happen
                    xtemp = c.lines[edge[0]][2] / c.lines[edge[0]][0]
                    ytemp = 0.0
                    xend = xtemp
                    yend = height
                else:
                    # End the line at both edges - ???
                    xtemp = 0.0
                    ytemp = c.lines[edge[0]][2] / c.lines[edge[0]][1]
                    xend = width
                    yend = (c.lines[edge[0]][2] - width *
                            c.lines[edge[0]][0]) / c.lines[edge[0]][1]
                [x1, y1, x2, y2] = clip_line(
                    xtemp,
                    ytemp,
                    xend,
                    yend,
                    width,
                    height,
                )
            if x1 or x2 or y1 or y2:
                bndpoints.append(QgsPointXY(x1 + extent.xMinimum(),
                                            y1 + extent.yMinimum()))
                bndpoints.append(QgsPointXY(x2 + extent.xMinimum(),
                                            y2 + extent.yMinimum()))
                if 0 in (x1, x2):
                    hasXMin = True
                    if XMinLine != edge[0]:
                        XMinNumber = XMinNumber + 1
                        XMinLine = edge[0]
                if 0 in (y1, y2):
                    hasYMin = True
                    if YMinLine != edge[0]:
                        YMinNumber = YMinNumber + 1
                        YMinLine = edge[0]
                if height in (y1, y2):
                    hasYMax = True
                    if YMaxLine != edge[0]:
                        YMaxNumber = YMaxNumber + 1
                        YMaxLine = edge[0]
                if width in (x1, x2):
                    hasXMax = True
                    if XMaxLine != edge[0]:
                        XMaxNumber = XMaxNumber + 1
                        XMaxLine = edge[0]

        # Add auxiliary points for corner cases, if necessary (duplicate
        # points is not a problem - will be ignored later).
        # a) Extreme input points (lowest, leftmost, rightmost, highest)
        #    A point can be extreme on both axis
        if pt_x == xmin:   # leftmost point
            if XMinNumber == 0:
                bndpoints.append(QgsPointXY(extent.xMinimum(),
                                            extent.yMinimum()))
                bndpoints.append(QgsPointXY(extent.xMinimum(),
                                            height + extent.yMinimum()))
            elif XMinNumber == 1:
                if hasYMin:
                    bndpoints.append(QgsPointXY(extent.xMinimum(),
                                                extent.yMinimum()))
                elif hasYMax:
                    bndpoints.append(QgsPointXY(extent.xMinimum(),
                                                height + extent.yMinimum()))
        elif pt_x == xmax:   # rightmost point
            if XMaxNumber == 0:
                bndpoints.append(QgsPointXY(width + extent.xMinimum(),
                                            extent.yMinimum()))
                bndpoints.append(QgsPointXY(width + extent.xMinimum(),
                                            height + extent.yMinimum()))
            elif XMaxNumber == 1:
                if hasYMin:
                    bndpoints.append(QgsPointXY(width + extent.xMinimum(),
                                                extent.yMinimum()))
                elif HasYMax:
                    bndpoints.append(QgsPointXY(width + extent.xMinimum(),
                                                height + extent.yMinimum()))
        if pt_y == ymin:    # lowest point
            if YMinNumber == 0:
                bndpoints.append(QgsPointXY(extent.xMinimum(),
                                            extent.yMinimum()))
                bndpoints.append(QgsPointXY(width + extent.xMinimum(),
                                            extent.yMinimum()))
            elif YMinNumber == 1:
                if hasXMin:
                    bndpoints.append(QgsPointXY(extent.xMinimum(),
                                                extent.yMinimum()))
                elif hasXMax:
                    bndpoints.append(QgsPointXY(width + extent.xMinimum(),
                                                extent.yMinimum()))
        elif pt_y == ymax:  # highest point
            if YMaxNumber == 0:
                bndpoints.append(QgsPointXY(extent.xMinimum(),
                                            height + extent.yMinimum()))
                bndpoints.append(QgsPointXY(width + extent.xMinimum(),
                                            height + extent.yMinimum()))
            elif YMaxNumber == 1:
                if hasXMin:
                    bndpoints.append(QgsPointXY(extent.xMinimum(),
                                                height + extent.yMinimum()))
                elif hasXMax:
                    bndpoints.append(QgsPointXY(width + extent.xMinimum(),
                                                height + extent.yMinimum()))
        # b) Polygon that covers the x or the y extent:
        if hasYMin and hasYMax:
            if YMaxNumber > 1:
                if hasXMin:
                    bndpoints.append(QgsPointXY(extent.xMinimum(),
                                                extent.yMinimum()))
                elif hasXMax:
                    bndpoints.append(QgsPointXY(width + extent.xMinimum(),
                                                extent.yMinimum()))
            elif YMinNumber > 1:
                if hasXMin:
                    bndpoints.append(QgsPointXY(extent.xMinimum(),
                                                height + extent.yMinimum()))
                elif hasXMax:
                    bndpoints.append(QgsPointXY(width + extent.xMinimum(),
                                                height + extent.yMinimum()))
        elif hasXMin and hasXMax:
            if XMaxNumber > 1:
                if hasYMin:
                    bndpoints.append(QgsPointXY(extent.xMinimum(),
                                                extent.yMinimum()))
                elif hasYMax:
                    bndpoints.append(QgsPointXY(width + extent.xMinimum(),
                                                extent.yMinimum()))
            elif XMinNumber > 1:
                if hasYMin:
                    bndpoints.append(QgsPointXY(extent.xMinimum(),
                                                height + extent.yMinimum()))
                elif hasYMax:
                    bndpoints.append(QgsPointXY(width + extent.xMinimum(),
                                                height + extent.yMinimum()))
        # c) Simple corners:
        if XMinNumber == 1 and YMinNumber == 1 and not hasXMax and not hasYMax:
            bndpoints.append(QgsPointXY(extent.xMinimum(),
                                        extent.yMinimum()))
        if XMinNumber == 1 and YMaxNumber == 1 and not hasXMax and not hasYMin:
            bndpoints.append(QgsPointXY(extent.xMinimum(),
                                        height + extent.yMinimum()))
        if XMaxNumber == 1 and YMinNumber == 1 and not hasXMin and not hasYMax:
            bndpoints.append(QgsPointXY(width + extent.xMinimum(),
                                        extent.yMinimum()))
        if XMaxNumber == 1 and YMaxNumber == 1 and not hasXMin and not hasYMin:
            bndpoints.append(QgsPointXY(width + extent.xMinimum(),
                                        height + extent.yMinimum()))
        return bndpoints
