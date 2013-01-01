# -*- coding: utf-8 -*-

"""
***************************************************************************
    Intersection.py
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

from sextante.core.GeoAlgorithm import GeoAlgorithm
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from sextante.outputs.OutputVector import OutputVector
from sextante.algs.ftools import ftools_utils
from sextante.core.SextanteLog import SextanteLog
from sextante.core.SextanteConfig import SextanteConfig

class Intersection(GeoAlgorithm):

    INPUT = "INPUT"
    INPUT2 = "INPUT2"
    OUTPUT = "OUTPUT"

    #===========================================================================
    # def getIcon(self):
    #    return QtGui.QIcon(os.path.dirname(__file__) + "/icons/intersect.png")
    #===========================================================================

    def processAlgorithm(self, progress):
        useSelection = SextanteConfig.getSetting(SextanteConfig.USE_SELECTED)
        vlayerA = QGisLayers.getObjectFromUri(self.getParameterValue(Intersection.INPUT))
        vlayerB = QGisLayers.getObjectFromUri(self.getParameterValue(Intersection.INPUT2))
        GEOS_EXCEPT = True
        FEATURE_EXCEPT = True
        vproviderA = vlayerA.dataProvider()
        allAttrsA = vproviderA.attributeIndexes()
        vproviderA.select( allAttrsA )
        vproviderB = vlayerB.dataProvider()
        allAttrsB = vproviderB.attributeIndexes()
        vproviderB.select(allAttrsB )
        # check for crs compatibility
        crsA = vproviderA.crs()
        crsB = vproviderB.crs()
        if not crsA.isValid() or not crsB.isValid():
            SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Intersection. Invalid CRS. Results might be unexpected")
        else:
            if not crsA != crsB:
                SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Intersection. Non-matching CRSs. Results might be unexpected")
        fields = ftools_utils.combineVectorFields(vlayerA, vlayerB)
        longNames = ftools_utils.checkFieldNameLength( fields )
        if not longNames.isEmpty():
            raise GeoAlgorithmExecutionException("Following field names are longer than 10 characters:\n" +  longNames.join('\n') )
        writer = self.getOutputFromName(Intersection.OUTPUT).getVectorWriter(fields, vproviderA.geometryType(), vproviderA.crs() )
        inFeatA = QgsFeature()
        inFeatB = QgsFeature()
        outFeat = QgsFeature()
        index = ftools_utils.createIndex( vproviderB )
        nElement = 0
        # there is selection in input layer
        if useSelection:
            nFeat = vlayerA.selectedFeatureCount()
            selectionA = vlayerA.selectedFeatures()
            selectionB = vlayerB.selectedFeaturesIds()
            for inFeatA in selectionA:
              nElement += 1
              progress.setPercentage(int(nElement/nFeat * 100))
              geom = QgsGeometry( inFeatA.geometry() )
              atMapA = inFeatA.attributeMap()
              intersects = index.intersects( geom.boundingBox() )
              for id in intersects:
                if id in selectionB:
                  vproviderB.featureAtId( int( id ), inFeatB , True, allAttrsB )
                  tmpGeom = QgsGeometry( inFeatB.geometry() )
                  try:
                    if geom.intersects( tmpGeom ):
                      atMapB = inFeatB.attributeMap()
                      int_geom = QgsGeometry( geom.intersection( tmpGeom ) )
                      if int_geom.wkbType() == 7:
                        int_com = geom.combine( tmpGeom )
                        int_sym = geom.symDifference( tmpGeom )
                        int_geom = QgsGeometry( int_com.difference( int_sym ) )
                      try:
                        outFeat.setGeometry( int_geom )
                        outFeat.setAttributeMap( ftools_utils.combineVectorAttributes( atMapA, atMapB ) )
                        writer.addFeature( outFeat )
                      except:
                        FEATURE_EXCEPT = False
                        continue
                  except:
                    GEOS_EXCEPT = False
                    break

        else:
            nFeat = vproviderA.featureCount()
            vproviderA.rewind()          
            while vproviderA.nextFeature( inFeatA ):
              nElement += 1
              progress.setPercentage(int(nElement/nFeat * 100))
              geom = QgsGeometry( inFeatA.geometry() )
              atMapA = inFeatA.attributeMap()
              intersects = index.intersects( geom.boundingBox() )
              for id in intersects:
                vproviderB.featureAtId( int( id ), inFeatB , True, allAttrsB )
                tmpGeom = QgsGeometry( inFeatB.geometry() )
                try:
                  if geom.intersects( tmpGeom ):
                    atMapB = inFeatB.attributeMap()
                    int_geom = QgsGeometry( geom.intersection( tmpGeom ) )
                    if int_geom.wkbType() == 7:
                      int_com = geom.combine( tmpGeom )
                      int_sym = geom.symDifference( tmpGeom )
                      int_geom = QgsGeometry( int_com.difference( int_sym ) )
                    try:
                      outFeat.setGeometry( int_geom )
                      outFeat.setAttributeMap( ftools_utils.combineVectorAttributes( atMapA, atMapB ) )
                      writer.addFeature( outFeat )
                    except:
                      FEATURE_EXCEPT = False
                      continue
                except:
                  GEOS_EXCEPT = False
                  break
              
        del writer
        if not GEOS_EXCEPT:
            SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Geometry exception while computing intersection")
        if not FEATURE_EXCEPT:
            SextanteLog.addToLog(SextanteLog.LOG_WARNING, "Feature exception while computing intersection")

    def defineCharacteristics(self):
        self.name = "Intersection"
        self.group = "Vector overlay tools"
        self.addParameter(ParameterVector(Intersection.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterVector(Intersection.INPUT2, "Intersect layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addOutput(OutputVector(Intersection.OUTPUT, "Intersection"))
