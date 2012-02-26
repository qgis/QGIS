from sextante.core.GeoAlgorithm import GeoAlgorithm
import os.path
from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.outputs.OutputVector import OutputVector
from sextante.ftools import ftools_utils
import math

class BasicStatisticsAlgorithm(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/centroids.png")

    def processAlgorithm(self, progress):
        settings = QSettings()
        systemEncoding = settings.value( "/UI/encoding", "System" ).toString()
        output = self.getOutputValue(CentroidsAlgorithm.OUTPUT)
        vlayer = QGisLayers.getObjectFromUri(self.getParameterValue(CentroidsAlgorithm.INPUT))
        vprovider = vlayer.dataProvider()
        allAttrs = vprovider.attributeIndexes()
        vprovider.select( allAttrs )
        fields = vprovider.fields()
        index = vprovider.fieldNameIndex( myField )
        feat = QgsFeature()
        sumVal = 0.0
        meanVal = 0.0
        nVal = 0.0
        values = []
        first = True
        nElement = 0
        # determine selected field type
        if ftools_utils.getFieldType( vlayer, myField ) in ('String', 'varchar', 'char', 'text'):
          fillVal = 0
          emptyVal = 0
          if self.mySelection: # only selected features
            selection = vlayer.selectedFeatures()
            nFeat = vlayer.selectedFeatureCount()
            if nFeat > 0:
              self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
              self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
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
              self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
          else: # there is no selection, process the whole layer
            nFeat = vprovider.featureCount()
          if nFeat > 0:
            self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
            self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
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
              self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
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
                return ( lstStats, [] )
            else:
                return ( ["Error:No features selected!"], [] )
        else: # numeric field
          stdVal = 0.00
          cvVal = 0.00
          rangeVal = 0.00
          medianVal = 0.00
          maxVal = 0.00
          minVal = 0.00
          if self.mySelection: # only selected features
            selection = vlayer.selectedFeatures()
            nFeat = vlayer.selectedFeatureCount()
            uniqueVal = ftools_utils.getUniqueValuesCount( vlayer, index, True )
            if nFeat > 0:
              self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
              self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
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
              self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
          else: # there is no selection, process the whole layer
            nFeat = vprovider.featureCount()
            uniqueVal = ftools_utils.getUniqueValuesCount( vlayer, index, False )
          if nFeat > 0:
            self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
            self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
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
              self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
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
              return ( lstStats, [] )
          else:
            return ( ["Error:No features selected!"], [] )

    def defineCharacteristics(self):
        self.name = "Basic statistics"
        self.group = "Analysis tools"
        self.addParameter(ParameterVector(BasicStatisticsAlgorithm.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_POLYGON))
        self.addOutput(OutputVector(BasicStatisticsAlgorithm.OUTPUT, "Statistics"))
