from sextante.core.GeoAlgorithm import GeoAlgorithm
import os.path
from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputVector import OutputVector
from sextante.parameters.ParameterBoolean import ParameterBoolean
from sextante.parameters.ParameterTableField import ParameterTableField
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.core.SextanteLog import SextanteLog

class Dissolve(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    FIELD = "FIELD"
    USE_SELECTED = "USE_SELECTED"
    DISSOLVE_ALL = "DISSOLVE_ALL"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/dissolve.png")

    def processAlgorithm(self, progress):
        useSelection = self.getParameterValue(Dissolve.USE_SELECTED)
        useField = not self.getParameterValue(Dissolve.USE_SELECTED)
        fieldname = self.getParameterValue(Dissolve.FIELD)
        vlayerA = QGisLayers.getObjectFromUri(self.getParameterValue(Dissolve.INPUT))
        field = vlayerA.dataProvider().fieldNameIndex(fieldname)
        GEOS_EXCEPT = True
        FEATURE_EXCEPT = True
        vproviderA = vlayerA.dataProvider()
        allAttrsA = vproviderA.attributeIndexes()
        fields = vproviderA.fields()
        writer = self.getOutputFromName(Dissolve.OUTPUT).getVectorWriter(fields, vproviderA.geometryType(), vproviderA.crs() )
        inFeat = QgsFeature()
        outFeat = QgsFeature()
        vproviderA.rewind()
        nElement = 0
        # there is selection in input layer
        if useSelection:
          nFeat = vlayerA.selectedFeatureCount()
          selectionA = vlayerA.selectedFeatures()
          if not useField:
            first = True
            for inFeat in selectionA:
              nElement += 1
              progress.setPercentage(int(nElement/nFeat * 100))
              if first:
                attrs = inFeat.attributeMap()
                tmpInGeom = QgsGeometry( inFeat.geometry() )
                outFeat.setGeometry( tmpInGeom )
                first = False
              else:
                tmpInGeom = QgsGeometry( inFeat.geometry() )
                tmpOutGeom = QgsGeometry( outFeat.geometry() )
                try:
                  tmpOutGeom = QgsGeometry( tmpOutGeom.combine( tmpInGeom ) )
                  outFeat.setGeometry( tmpOutGeom )
                except:
                  GEOS_EXCEPT = False
                  continue
            outFeat.setAttributeMap( attrs )
            writer.addFeature( outFeat )
          else:
            unique = vproviderA.uniqueValues( int( field ) )
            nFeat = nFeat * len( unique )
            for item in unique:
              first = True
              add = False
              vproviderA.select( allAttrsA )
              vproviderA.rewind()
              for inFeat in selectionA:
                nElement += 1
                progress.setPercentage(int(nElement/nFeat * 100))
                atMap = inFeat.attributeMap()
                tempItem = atMap[ field ]
                if tempItem.toString().trimmed() == item.toString().trimmed():
                  add = True
                  if first:
                    QgsGeometry( inFeat.geometry() )
                    tmpInGeom = QgsGeometry( inFeat.geometry() )
                    outFeat.setGeometry( tmpInGeom )
                    first = False
                    attrs = inFeat.attributeMap()
                  else:
                    tmpInGeom = QgsGeometry( inFeat.geometry() )
                    tmpOutGeom = QgsGeometry( outFeat.geometry() )
                    try:
                      tmpOutGeom = QgsGeometry( tmpOutGeom.combine( tmpInGeom ) )
                      outFeat.setGeometry( tmpOutGeom )
                    except:
                      GEOS_EXCEPT = False
                      add = False
              if add:
                outFeat.setAttributeMap( attrs )
                writer.addFeature( outFeat )
        # there is no selection in input layer
        else:
          nFeat = vproviderA.featureCount()
          if not useField:
            first = True
            while vproviderA.nextFeature( inFeat ):
              nElement += 1
              progress.setPercentage(int(nElement/nFeat * 100))
              if first:
                attrs = inFeat.attributeMap()
                tmpInGeom = QgsGeometry( inFeat.geometry() )
                outFeat.setGeometry( tmpInGeom )
                first = False
              else:
                tmpInGeom = QgsGeometry( inFeat.geometry() )
                tmpOutGeom = QgsGeometry( outFeat.geometry() )
                try:
                  tmpOutGeom = QgsGeometry( tmpOutGeom.combine( tmpInGeom ) )
                  outFeat.setGeometry( tmpOutGeom )
                except:
                  GEOS_EXCEPT = False
                  continue
            outFeat.setAttributeMap( attrs )
            writer.addFeature( outFeat )
          else:
            unique = vproviderA.uniqueValues( int( field ) )
            nFeat = nFeat * len( unique )
            for item in unique:
              first = True
              add = True
              vproviderA.select( allAttrsA )
              vproviderA.rewind()
              while vproviderA.nextFeature( inFeat ):
                nElement += 1
                progress.setPercentage(int(nElement/nFeat * 100))
                atMap = inFeat.attributeMap()
                tempItem = atMap[ field ]
                if tempItem.toString().trimmed() == item.toString().trimmed():
                  if first:
                    QgsGeometry( inFeat.geometry() )
                    tmpInGeom = QgsGeometry( inFeat.geometry() )
                    outFeat.setGeometry( tmpInGeom )
                    first = False
                    attrs = inFeat.attributeMap()
                  else:
                    tmpInGeom = QgsGeometry( inFeat.geometry() )
                    tmpOutGeom = QgsGeometry( outFeat.geometry() )
                    try:
                      tmpOutGeom = QgsGeometry( tmpOutGeom.combine( tmpInGeom ) )
                      outFeat.setGeometry( tmpOutGeom )
                    except:
                      GEOS_EXCEPT = False
                      add = False
              if add:
                outFeat.setAttributeMap( attrs )
                writer.addFeature( outFeat )
        del writer
        if not GEOS_EXCEPT:
            SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Geometry exception while dissolving")
        if not FEATURE_EXCEPT:
            SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Feature exception while dissolving")



    def defineCharacteristics(self):
        self.name = "Dissolve"
        self.group = "Geoprocessing tools"
        self.addParameter(ParameterVector(Dissolve.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_POLYGON))
        self.addParameter(ParameterBoolean(Dissolve.USE_SELECTED, "Use selected features", False))
        self.addParameter(ParameterBoolean(Dissolve.DISSOLVE_ALL, "Dissolve all (do not use field)", True))
        self.addParameter(ParameterTableField(Dissolve.FIELD, "Unique ID field",Dissolve.INPUT ))
        self.addOutput(OutputVector(Dissolve.OUTPUT, "Dissolved"))

    #=========================================================
