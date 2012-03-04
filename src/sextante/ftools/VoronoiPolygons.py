from sextante.core.GeoAlgorithm import GeoAlgorithm
from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputVector import OutputVector
import voronoi
from sets import Set

class VoronoiPolygons(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def processAlgorithm(self, progress):
        settings = QSettings()
        systemEncoding = settings.value( "/UI/encoding", "System" ).toString()
        output = self.getOutputValue(VoronoiPolygons.OUTPUT)
        vlayer = QGisLayers.getObjectFromUri(self.getParameterValue(VoronoiPolygons.INPUT))
        vprovider = vlayer.dataProvider()
        allAttrs = vprovider.attributeIndexes()
        vprovider.select( allAttrs )
        writer = QgsVectorFileWriter( output, systemEncoding, vprovider.fields(), QGis.WKBPolygon, vprovider.crs() )
        inFeat = QgsFeature()
        outFeat = QgsFeature()
        extent = vlayer.extent()
        height = extent.height()
        width = extent.width()
        c = voronoi.Context()
        pts = []
        ptDict = {}
        ptNdx = -1
        while vprovider.nextFeature(inFeat):
          geom = QgsGeometry(inFeat.geometry())
          point = geom.asPoint()
          x = point.x()-extent.xMinimum()
          y = point.y()-extent.yMinimum()
          pts.append((x, y))
          ptNdx +=1
          ptDict[ptNdx] = inFeat.id()
        #self.vlayer = None
        if len(pts) < 3:
          return False
        uniqueSet = Set(item for item in pts)
        ids = [pts.index(item) for item in uniqueSet]
        sl = voronoi.SiteList([voronoi.Site(i[0], i[1], sitenum=j) for j, i in enumerate(uniqueSet)])
        voronoi.voronoi(sl, c)
        inFeat = QgsFeature()
        nFeat = len(c.polygons)
        nElement = 0
        for site, edges in c.polygons.iteritems():
          vprovider.featureAtId(ptDict[ids[site]], inFeat, True,  allAttrs)
          lines = self.clip_voronoi(edges, c, width, height, extent, 0, 0)
          geom = QgsGeometry.fromMultiPoint(lines)
          geom = QgsGeometry(geom.convexHull())
          outFeat.setGeometry(geom)
          outFeat.setAttributeMap(inFeat.attributeMap())
          writer.addFeature(outFeat)
          nElement += 1
          progress.setPercentage(nElement/nFeat * 100)
        del writer
        return True


    def clip_voronoi(self, edges, c, width, height, extent, exX, exY):
        """ Clip voronoi function based on code written for Inkscape
            Copyright (C) 2010 Alvin Penner, penner@vaxxine.com
        """
        def clip_line(x1, y1, x2, y2, w, h, x, y):
          if x1 < 0-x and x2 < 0-x:
            return [0, 0, 0, 0]
          if x1 > w+x and x2 > w+x:
            return [0, 0, 0, 0]
          if x1 < 0-x:
            y1 = (y1*x2 - y2*x1)/(x2 - x1)
            x1 = 0-x
          if x2 < 0-x:
            y2 = (y1*x2 - y2*x1)/(x2 - x1)
            x2 = 0-x
          if x1 > w+x:
            y1 = y1 + (w+x - x1)*(y2 - y1)/(x2 - x1)
            x1 = w+x
          if x2 > w+x:
            y2 = y1 + (w+x - x1)*(y2 - y1)/(x2 - x1)
            x2 = w+x
          if y1 < 0-y and y2 < 0-y:
            return [0, 0, 0, 0]
          if y1 > h+y and y2 > h+y:
            return [0, 0, 0, 0]
          if x1 == x2 and y1 == y2:
            return [0, 0, 0, 0]
          if y1 < 0-y:
            x1 = (x1*y2 - x2*y1)/(y2 - y1)
            y1 = 0-y
          if y2 < 0-y:
            x2 = (x1*y2 - x2*y1)/(y2 - y1)
            y2 = 0-y
          if y1 > h+y:
            x1 = x1 + (h+y - y1)*(x2 - x1)/(y2 - y1)
            y1 = h+y
          if y2 > h+y:
            x2 = x1 + (h+y - y1)*(x2 - x1)/(y2 - y1)
            y2 = h+y
          return [x1, y1, x2, y2]
        lines = []
        hasXMin = False
        hasYMin = False
        hasXMax = False
        hasYMax = False
        for edge in edges:
          if edge[1] >= 0 and edge[2] >= 0:       # two vertices
              [x1, y1, x2, y2] = clip_line(c.vertices[edge[1]][0], c.vertices[edge[1]][1], c.vertices[edge[2]][0], c.vertices[edge[2]][1], width, height, exX, exY)
          elif edge[1] >= 0:                      # only one vertex
            if c.lines[edge[0]][1] == 0:        # vertical line
              xtemp = c.lines[edge[0]][2]/c.lines[edge[0]][0]
              if c.vertices[edge[1]][1] > (height+exY)/2:
                ytemp = height+exY
              else:
                ytemp = 0-exX
            else:
              xtemp = width+exX
              ytemp = (c.lines[edge[0]][2] - (width+exX)*c.lines[edge[0]][0])/c.lines[edge[0]][1]
            [x1, y1, x2, y2] = clip_line(c.vertices[edge[1]][0], c.vertices[edge[1]][1], xtemp, ytemp, width, height, exX, exY)
          elif edge[2] >= 0:                      # only one vertex
            if c.lines[edge[0]][1] == 0:        # vertical line
              xtemp = c.lines[edge[0]][2]/c.lines[edge[0]][0]
              if c.vertices[edge[2]][1] > (height+exY)/2:
                ytemp = height+exY
              else:
                ytemp = 0.0-exY
            else:
              xtemp = 0.0-exX
              ytemp = c.lines[edge[0]][2]/c.lines[edge[0]][1]
            [x1, y1, x2, y2] = clip_line(xtemp, ytemp, c.vertices[edge[2]][0], c.vertices[edge[2]][1], width, height, exX, exY)
          if x1 or x2 or y1 or y2:
            lines.append(QgsPoint(x1+extent.xMinimum(),y1+extent.yMinimum()))
            lines.append(QgsPoint(x2+extent.xMinimum(),y2+extent.yMinimum()))
            if 0-exX in (x1, x2):
              hasXMin = True
            if 0-exY in (y1, y2):
              hasYMin = True
            if height+exY in (y1, y2):
              hasYMax = True
            if width+exX in (x1, x2):
              hasXMax = True
        if hasXMin:
          if hasYMax:
            lines.append(QgsPoint(extent.xMinimum()-exX, height+extent.yMinimum()+exY))
          if hasYMin:
            lines.append(QgsPoint(extent.xMinimum()-exX, extent.yMinimum()-exY))
        if hasXMax:
          if hasYMax:
            lines.append(QgsPoint(width+extent.xMinimum()+exX, height+extent.yMinimum()+exY))
          if hasYMin:
            lines.append(QgsPoint(width+extent.xMinimum()+exX, extent.yMinimum()-exY))
        return lines

    def defineCharacteristics(self):
        self.name = "Voronoi polygons"
        self.group = "Geometry tools"
        self.addParameter(ParameterVector(VoronoiPolygons.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_POINT))
        self.addOutput(OutputVector(VoronoiPolygons.OUTPUT, "Voronoi polygons"))

