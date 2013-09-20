# -*- coding: utf-8 -*-

"""
***************************************************************************
    Eliminate.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2013 by Bernhard Str�bl
    Email                : bernhard.stroebl@jena.de
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""
from processing.parameters.ParameterSelection import ParameterSelection
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from PyQt4 import QtCore

__author__ = 'Bernhard Strobl'
__date__ = 'August 2013'
__copyright__ = '(C) 2013, Bernhard Strobl'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4.QtCore import *
from qgis.core import *
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.tools import dataobjects
from processing.parameters.ParameterVector import ParameterVector
from processing.outputs.OutputVector import OutputVector


class Eliminate(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    MODE = "MODE"
    
    MODES = ["Area", "Common boundary"]
    MODE_AREA = 0
    MODE_BOUNDARY = 1
    

    def defineCharacteristics(self):
        self.name = "Eliminate sliver polygons"
        self.group = "Vector geometry tools"
        self.addParameter(ParameterVector(self.INPUT, "Input layer", [ParameterVector.VECTOR_TYPE_POLYGON]))        
        self.addParameter(ParameterSelection(self.MODE, "Merge selection with the neighbouring polygon with the largest", self.MODES))        
        self.addOutput(OutputVector(self.OUTPUT, "Cleaned layer"))

    def processAlgorithm(self, progress):
        inLayer = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT))        
        boundary = self.getParameterValue(self.MODE) == self.MODE_BOUNDARY        

        # keep references to the features to eliminate
        fidsToEliminate = inLayer.selectedFeaturesIds()


        provider = inLayer.dataProvider()
        output = self.getOutputFromName(self.OUTPUT)
        writer = output.getVectorWriter( provider.fields(), 
                                         provider.geometryType(),                                           
                                         inLayer.crs() )
        
        #write all features to output layer 
        iterator = inLayer.getFeatures()
        for feature in iterator:
            writer.addFeature(feature)
            
        #Open the layer to start cleaning it
        outFileName = output.value
        outLayer = QgsVectorLayer(outFileName, QtCore.QFileInfo(outFileName).completeBaseName(), "ogr")
                

        # delete features to be eliminated in outLayer
        outLayer.setSelectedFeatures(fidsToEliminate)
        outLayer.startEditing()

        if outLayer.deleteSelectedFeatures():
            if self.saveChanges(outLayer):
                outLayer.startEditing()
        else:
            raise GeoAlgorithmExecutionException("Could not delete features")       

        # ANALYZE
        start = 20.00
        progress.setPercentage(start)
        add = 80.00 / len(fidsToEliminate)

        lastLen = 0
        geomsToMerge = dict()

        # we go through the list and see if we find any polygons we can merge the selected with
        # if we have no success with some we merge and then restart the whole story
        while (lastLen != inLayer.selectedFeatureCount()): #check if we made any progress
            lastLen = inLayer.selectedFeatureCount()
            fidsToDeselect = []

            #iterate over the polygons to eliminate
            for fid2Eliminate in inLayer.selectedFeaturesIds():
                feat = QgsFeature()

                if inLayer.getFeatures( QgsFeatureRequest().setFilterFid( fid2Eliminate ).setSubsetOfAttributes([]) ).nextFeature( feat ):
                    geom2Eliminate = feat.geometry()
                    bbox = geom2Eliminate.boundingBox()
                    fit = outLayer.getFeatures( QgsFeatureRequest().setFilterRect( bbox ) )
                    mergeWithFid = None
                    mergeWithGeom = None
                    max = 0

                    selFeat = QgsFeature()
                    while fit.nextFeature(selFeat):
                            selGeom = selFeat.geometry()

                            if geom2Eliminate.intersects(selGeom): # we have a candidate
                                iGeom = geom2Eliminate.intersection(selGeom)

                                if boundary:
                                    selValue = iGeom.length()
                                else:
                                    # we need a common boundary
                                    if 0 < iGeom.length():
                                        selValue = selGeom.area()
                                    else:
                                        selValue = 0

                                if selValue > max:
                                    max = selValue
                                    mergeWithFid = selFeat.id()
                                    mergeWithGeom = QgsGeometry(selGeom) # deep copy of the geometry

                    if mergeWithFid != None: # a successful candidate
                        newGeom = mergeWithGeom.combine(geom2Eliminate)

                        if outLayer.changeGeometry(mergeWithFid, newGeom):
                            # write change back to disc
                            if self.saveChanges(outLayer):
                                outLayer.startEditing()
                            else:
                                return

                            # mark feature as eliminated in inLayer
                            fidsToDeselect.append(fid2Eliminate)
                        else:
                            raise GeoAlgorithmExecutionException("Could not replace geometry of feature with id %s" % (mergeWithFid))                            

                        start = start + add
                        progress.setPercentage(start)
            # end for fid2Eliminate

            # deselect features that are already eliminated in inLayer
            inLayer.deselect(fidsToDeselect)

        #end while

        if inLayer.selectedFeatureCount() > 0:
            # copy all features that could not be eliminated to outLayer
            if outLayer.addFeatures(inLayer.selectedFeatures()):
                # inform user
                fidList = ""

                for fid in inLayer.selectedFeaturesIds():
                    if not fidList == "":
                        fidList += ", "

                    fidList += str(fid)

                raise GeoAlgorithmExecutionException("Could not eliminate features with these ids:\n%s" % (fidList))
            else:
                raise GeoAlgorithmExecutionException("Could not add features")

        # stop editing outLayer and commit any pending changes
        self.saveChanges(outLayer)

    def saveChanges(self, outLayer):
        if not outLayer.commitChanges():                    
            msg = ""
            for aStrm in outLayer.commitErrors():
                msg = msg + "\n" + aStrm
            outLayer.rollBack()
            raise GeoAlgorithmExecutionException("Commit error:\n%s" % (msg))
        
    def checkParameterValuesBeforeExecuting(self):
        inLayer = dataobjects.getObjectFromUri(self.getParameterValue(self.INPUT))
        if inLayer.selectedFeatureCount() == 0:
            return "No selection in input layer"
        
