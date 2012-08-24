from sextante.core.GeoAlgorithm import GeoAlgorithm
import os.path
from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputVector import OutputVector
import voronoi
from sets import Set

class Delaunay(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/delaunay.png")

    def processAlgorithm(self, progress):
        vlayer = QGisLayers.getObjectFromUri(self.getParameterValue(Delaunay.INPUT))
        vprovider = vlayer.dataProvider()
        allAttrs = vprovider.attributeIndexes()
        vprovider.select( allAttrs )
        fields = {
                  0 : QgsField( "POINTA", QVariant.Double ),
                  1 : QgsField( "POINTB", QVariant.Double ),
                  2 : QgsField( "POINTC", QVariant.Double ) }
        writer = self.getOutputFromName(Delaunay.OUTPUT).getVectorWriter(fields, QGis.WKBPolygon, vprovider.crs() )
        inFeat = QgsFeature()
        c = voronoi.Context()
        pts = []
        ptDict = {}
        ptNdx = -1
        while vprovider.nextFeature(inFeat):
          geom = QgsGeometry(inFeat.geometry())
          point = geom.asPoint()
          x = point.x()
          y = point.y()
          pts.append((x, y))
          ptNdx +=1
          ptDict[ptNdx] = inFeat.id()
        if len(pts) < 3:
          return False
        uniqueSet = Set(item for item in pts)
        ids = [pts.index(item) for item in uniqueSet]
        sl = voronoi.SiteList([voronoi.Site(*i) for i in uniqueSet])
        c.triangulate = True
        voronoi.voronoi(sl, c)
        triangles = c.triangles
        feat = QgsFeature()
        nFeat = len( triangles )
        nElement = 0
        for triangle in triangles:
          indicies = list(triangle)
          indicies.append(indicies[0])
          polygon = []
          step = 0
          for index in indicies:
            vprovider.featureAtId(ptDict[ids[index]], inFeat, True,  allAttrs)
            geom = QgsGeometry(inFeat.geometry())
            point = QgsPoint(geom.asPoint())
            polygon.append(point)
            if step <= 3: feat.addAttribute(step, QVariant(ids[index]))
            step += 1
          geometry = QgsGeometry().fromPolygon([polygon])
          feat.setGeometry(geometry)
          writer.addFeature(feat)
          nElement += 1
          progress.setPercentage(nElement/nFeat * 100)
        del writer

    def defineCharacteristics(self):
        self.name = "Delaunay triangulation"
        self.group = "Geometry tools"
        self.addParameter(ParameterVector(Delaunay.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_POINT))
        self.addOutput(OutputVector(Delaunay.OUTPUT, "Delaunay triangulation"))

