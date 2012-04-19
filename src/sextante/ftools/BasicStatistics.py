from sextante.core.GeoAlgorithm import GeoAlgorithm
import os.path
from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.ftools import ftools_utils
import math
from sextante.outputs.OutputHTML import OutputHTML
from sextante.parameters.ParameterTableField import ParameterTableField
from sextante.parameters.ParameterBoolean import ParameterBoolean

class BasicStatistics(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    FIELD = "FIELD"
    USE_SELECTION = "USE_SELECTION"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/basic_statistics.png")


    def createHTML(self, outputFile, lstStats):
        f = open(outputFile, "w")
        for s in lstStats:
            f.write("<p>" + str(s) + "</p>")
        f.close()


    def processAlgorithm(self, progress):
        outputFile = self.getOutputValue(BasicStatistics.OUTPUT)
        vlayer = QGisLayers.getObjectFromUri(self.getParameterValue(BasicStatistics.INPUT))
        attfield = self.getParameterValue(BasicStatistics.FIELD)
        useSelection = self.getParameterValue(BasicStatistics.USE_SELECTION)
        vprovider = vlayer.dataProvider()
        allAttrs = vprovider.attributeIndexes()
        vprovider.select( allAttrs )
        index = vprovider.fieldNameIndex(attfield)
        feat = QgsFeature()
        sumVal = 0.0
        meanVal = 0.0
        nVal = 0.0
        values = []
        first = True
        nElement = 0
        # determine selected field type
        if ftools_utils.getFieldType( vlayer, attfield ) in ('String', 'varchar', 'char', 'text'):
          fillVal = 0
          emptyVal = 0
          if useSelection: # only selected features
            selection = vlayer.selectedFeatures()
            nFeat = vlayer.selectedFeatureCount()
            for f in selection:
              atMap = f.attributeMap()
              lenVal = float( len( atMap[ index ].toString() ) )
              if first:
                minVal = lenVal
                maxVal = lenVal
                first = False
              else:
                if lenVal < minVal: minVal = lenVal
                if lenVal > maxVal: maxVal = lenVal
              if lenVal != 0.00:
                fillVal += 1
              else:
                emptyVal += 1
              values.append( lenVal )
              sumVal = sumVal + lenVal
              nElement += 1
              progress.setPercentage(int(nElement/nFeat * 100))
          else: # there is no selection, process the whole layer
            nFeat = vprovider.featureCount()
          if nFeat > 0:
            vprovider.select( allAttrs )
            while vprovider.nextFeature( feat ):
              atMap = feat.attributeMap()
              lenVal = float( len( atMap[ index ].toString() ) )
              if first:
                minVal = lenVal
                maxVal = lenVal
                first = False
              else:
                if lenVal < minVal: minVal = lenVal
                if lenVal > maxVal: maxVal = lenVal
              if lenVal != 0.00:
                fillVal += 1
              else:
                emptyVal += 1
              values.append( lenVal )
              sumVal = sumVal + lenVal
              nElement += 1
              progress.setPercentage(int(nElement/nFeat * 100))
            nVal= float( len( values ) )
            if nVal > 0:
                meanVal = sumVal / nVal
                lstStats = []
                lstStats.append( "Max. len:"  + unicode( maxVal ) )
                lstStats.append(  "Min. len:"  + unicode( minVal ) )
                lstStats.append( "Mean. len:"  + unicode( meanVal ) )
                lstStats.append( "Filled:"  + unicode( fillVal ) )
                lstStats.append( "Empty:"  + unicode( emptyVal ) )
                lstStats.append( "N:"  + unicode( nVal ) )
                self.createHTML(outputFile, lstStats)
            else:
                raise GeoAlgorithmExecutionException("Error:No features selected!")
        else: # numeric field
          stdVal = 0.00
          cvVal = 0.00
          rangeVal = 0.00
          medianVal = 0.00
          maxVal = 0.00
          minVal = 0.00
          if useSelection: # only selected features
            selection = vlayer.selectedFeatures()
            nFeat = vlayer.selectedFeatureCount()
            uniqueVal = ftools_utils.getUniqueValuesCount( vlayer, index, True )
            for f in selection:
              atMap = f.attributeMap()
              value = float( atMap[ index ].toDouble()[ 0 ] )
              if first:
                minVal = value
                maxVal = value
                first = False
              else:
                if value < minVal: minVal = value
                if value > maxVal: maxVal = value
              values.append( value )
              sumVal = sumVal + value
              nElement += 1
              progress.setPercentage(int(nElement/nFeat * 100))
          else: # there is no selection, process the whole layer
            nFeat = vprovider.featureCount()
            uniqueVal = ftools_utils.getUniqueValuesCount( vlayer, index, False )
          if nFeat > 0:
            vprovider.select( allAttrs )
            while vprovider.nextFeature( feat ):
              atMap = feat.attributeMap()
              value = float( atMap[ index ].toDouble()[ 0 ] )
              if first:
                minVal = value
                maxVal = value
                first = False
              else:
                if value < minVal: minVal = value
                if value > maxVal: maxVal = value
              values.append( value )
              sumVal = sumVal + value
              nElement += 1
          nVal= float( len( values ) )
          if nVal > 0.00:
              rangeVal = maxVal - minVal
              meanVal = sumVal / nVal
              if meanVal != 0.00:
                for val in values:
                  stdVal += ( ( val - meanVal ) * ( val - meanVal ) )
                stdVal = math.sqrt( stdVal / nVal )
                cvVal = stdVal / meanVal
              if nVal > 1:
                  lstVal = values
                  lstVal.sort()
                  if ( nVal % 2 ) == 0:
                      medianVal = 0.5 * ( lstVal[ int( ( nVal - 1 ) / 2 ) ] + lstVal[ int( ( nVal ) / 2 ) ] )
                  else:
                      medianVal = lstVal[ int( ( nVal + 1 ) / 2 - 1 ) ]
              lstStats = []
              lstStats.append(  "Mean:"  + unicode( meanVal ) )
              lstStats.append( "StdDev:"  + unicode( stdVal ) )
              lstStats.append( "Sum:"  + unicode( sumVal) )
              lstStats.append( "Min:"  + unicode( minVal ) )
              lstStats.append( "Max:"  + unicode( maxVal ) )
              lstStats.append( "N:"  + unicode( nVal ) )
              lstStats.append( "CV:"  + unicode( cvVal ) )
              lstStats.append( "Number of unique values:"  + unicode( uniqueVal ) )
              lstStats.append( "Range:"  + unicode( rangeVal ) )
              lstStats.append( "Median:"  + unicode( medianVal ) )
              self.createHTML(outputFile, lstStats)
          else:
            raise GeoAlgorithmExecutionException("Error:No features selected!")

    def defineCharacteristics(self):
        self.name = "Basic statistics"
        self.group = "Analysis tools"
        self.addParameter(ParameterVector(BasicStatistics.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterTableField(BasicStatistics.FIELD, "Field", BasicStatistics.INPUT))
        self.addParameter(ParameterBoolean(BasicStatistics.USE_SELECTION, "Use selection", False))
        self.addOutput(OutputHTML(BasicStatistics.OUTPUT, "Statistics"))
