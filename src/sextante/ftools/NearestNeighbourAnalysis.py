from sextante.core.GeoAlgorithm import GeoAlgorithm
import os.path
from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.ftools import ftools_utils
import math
from sextante.outputs.OutputHTML import OutputHTML

class NearestNeighbourAnalysis(GeoAlgorithm):

    POINTS = "POINTS"
    OUTPUT = "OUTPUT"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/neighbour.png")

    def processAlgorithm(self, progress):
        output = self.getOutputValue(NearestNeighbourAnalysis.OUTPUT)
        vlayer = QGisLayers.getObjectFromUri(self.getParameterValue(NearestNeighbourAnalysis.POINTS))
        vprovider = vlayer.dataProvider()
        allAttrs = vprovider.attributeIndexes()
        vprovider.select( allAttrs )
        feat = QgsFeature()
        neighbour = QgsFeature()
        sumDist = 0.00
        distance = QgsDistanceArea()
        A = vlayer.extent()
        A = float( A.width() * A.height() )
        index = ftools_utils.createIndex( vprovider )
        vprovider.rewind()
        nFeat = vprovider.featureCount()
        nElement = 0
        while vprovider.nextFeature( feat ):
          neighbourID = index.nearestNeighbor( feat.geometry().asPoint(), 2 )[ 1 ]
          vprovider.featureAtId( neighbourID, neighbour, True, [] )
          nearDist = distance.measureLine( neighbour.geometry().asPoint(), feat.geometry().asPoint() )
          sumDist += nearDist
          nElement += 1
          progress.setPercentage(int(nElement/nFeat * 100))
        nVal = vprovider.featureCount()
        do = float( sumDist) / nVal
        de = float( 0.5 / math.sqrt( nVal / A ) )
        d = float( do / de )
        SE = float( 0.26136 / math.sqrt( ( nVal * nVal ) / A ) )
        zscore = float( ( do - de ) / SE )
        lstStats = []
        lstStats.append("Observed mean distance:" + unicode( do ) )
        lstStats.append("Expected mean distance:" + unicode( de ) )
        lstStats.append("Nearest neighbour index:" + unicode( d ) )
        lstStats.append("N:" + unicode( nVal ) )
        lstStats.append("Z-Score:"  + unicode( zscore ) )
        self.createHTML(output, lstStats)


    def createHTML(self, outputFile, lstStats):
        f = open(outputFile, "w")
        for s in lstStats:
            f.write("<p>" + str(s) + "</p>")
        f.close()


    def defineCharacteristics(self):
        self.name = "Nearest neighbour analysis"
        self.group = "Analysis tools"
        self.addParameter(ParameterVector(NearestNeighbourAnalysis.POINTS, "Points", ParameterVector.VECTOR_TYPE_POINT))
        self.addOutput(OutputHTML(NearestNeighbourAnalysis.OUTPUT, "Result"))