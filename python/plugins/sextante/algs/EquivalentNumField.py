# -*- coding: utf-8 -*-

"""
***************************************************************************
    EquivalentNumField.py
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
from qgis.core import *
from sextante.parameters.ParameterVector import ParameterVector
from sextante.core.QGisLayers import QGisLayers
from sextante.outputs.OutputVector import OutputVector
import os
from PyQt4 import QtGui
from sextante.parameters.ParameterTableField import ParameterTableField

class EquivalentNumField(GeoAlgorithm):

    INPUT = "INPUT"
    OUTPUT = "OUTPUT"
    FIELD = "FIELD"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/toolbox.png")

    def processAlgorithm(self, progress):
        fieldname = self.getParameterValue(self.FIELD)
        output = self.getOutputFromName(self.OUTPUT)
        vlayer = QGisLayers.getObjectFromUri(self.getParameterValue(self.INPUT))
        vprovider = vlayer.dataProvider()
        field_index = vprovider.fieldNameIndex(fieldname)
        allAttrs = vprovider.attributeIndexes()
        vprovider.select(allAttrs)
        fields = vprovider.fields()
        fields[len(fields)] = QgsField("NUM_FIELD", QVariant.Int)
        writer = output.getVectorWriter(fields, vprovider.geometryType(), vprovider.crs() )
        inFeat = QgsFeature()
        outFeat = QgsFeature()
        inGeom = QgsGeometry()
        nFeat = vprovider.featureCount()
        nElement = 0
        classes = {}
        while vprovider.nextFeature(inFeat):
          progress.setPercentage(int((100 * nElement)/nFeat))
          nElement += 1
          atMap = inFeat.attributeMap()
          clazz = atMap[field_index].toString()
          if clazz not in classes:
              classes[clazz] = len(classes.keys())
        while vprovider.nextFeature(inFeat):
          progress.setPercentage(int((100 * nElement)/nFeat))
          nElement += 1
          inGeom = inFeat.geometry()
          outFeat.setGeometry( inGeom )
          atMap = inFeat.attributeMap()
          clazz = atMap[field_index].toString()
          outFeat.setAttributeMap( atMap )
          outFeat.addAttribute( len(vprovider.fields()), QVariant(classes[clazz]) )
          writer.addFeature( outFeat )
        del writer

    def defineCharacteristics(self):
        self.name = "Create equivalent numerical field"
        self.group = "Algorithms for vector layers"
        self.addParameter(ParameterVector(self.INPUT, "Input layer", ParameterVector.VECTOR_TYPE_ANY))
        self.addParameter(ParameterTableField(self.FIELD, "Class field", self.INPUT))
        self.addOutput(OutputVector(self.OUTPUT, "Output layer"))

