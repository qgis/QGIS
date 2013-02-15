# -*- coding: utf-8 -*-

"""
***************************************************************************
    StatisticsByCategories.py
    ---------------------
    Date                 : September 2012
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
import math

__author__ = 'Victor Olaya'
__date__ = 'September 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4.QtCore import *
from qgis.core import *
from scipy import stats
from sextante.outputs.OutputTable import OutputTable
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.QGisLayers import QGisLayers
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterTableField import ParameterTableField

class StatisticsByCategories(GeoAlgorithm):

    INPUT_LAYER = "INPUT_LAYER"
    VALUES_FIELD_NAME = "VALUES_FIELD_NAME"
    CATEGORIES_FIELD_NAME = "CATEGORIES_FIELD_NAME"    
    OUTPUT = "OUTPUT"
    
    def defineCharacteristics(self):
        self.name = "Statistics by categories"
        self.group = "Vector table tools"

        self.addParameter(ParameterVector(self.INPUT_LAYER, "Input vector layer", ParameterVector.VECTOR_TYPE_ANY, False))
        self.addParameter(ParameterTableField(self.VALUES_FIELD_NAME, "Field to calculate statistics on", 
                                              self.INPUT_LAYER, ParameterTableField.DATA_TYPE_NUMBER))
        self.addParameter(ParameterTableField(self.CATEGORIES_FIELD_NAME, "Field with categories", 
                                              self.INPUT_LAYER, ParameterTableField.DATA_TYPE_ANY))        

        self.addOutput(OutputTable(self.OUTPUT, "Statistics for numeric field"))
        
    def processAlgorithm(self, progress):
        layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.INPUT_LAYER))
        valuesFieldName = self.getParameterValue(self.VALUES_FIELD_NAME)
        categoriesFieldName = self.getParameterValue(self.CATEGORIES_FIELD_NAME)

        output = self.getOutputFromName(self.OUTPUT)
        valuesField = layer.fieldNameIndex(valuesFieldName)
        categoriesField = layer.fieldNameIndex(categoriesFieldName)
        
        features = QGisLayers.features(layer)
        nFeat = len(features)
        values = {}
        for feat in features:
            attrs = feat.attributes()
            value = float(attrs[valuesField].toDouble()[0])
            cat = unicode(attrs[categoriesField].toString())
            if cat not in values:
                values[cat] = []
            values[cat].append(value)
        
        fields = [QgsField("category", QVariant.String), QgsField("mean", QVariant.Double), QgsField("variance", QVariant.Double)]
        writer = output.getTableWriter(fields)
        for cat, value in values.items():                       
            n, min_max, mean, var, skew, kurt = stats.describe(value)
            record = [cat, mean, math.sqrt(var)]            
            writer.addRecord(record)
            
   