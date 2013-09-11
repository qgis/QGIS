# -*- coding: utf-8 -*-

"""
***************************************************************************
    BasicStatisticsStrings.py
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

__author__ = 'Victor Olaya'
__date__ = 'September 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import codecs

from PyQt4.QtCore import *
from qgis.core import *
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.QGisLayers import QGisLayers
from processing.parameters.ParameterVector import ParameterVector
from processing.parameters.ParameterTableField import ParameterTableField
from processing.outputs.OutputHTML import OutputHTML
from processing.outputs.OutputNumber import OutputNumber
from processing.tools import vector as utils

class BasicStatisticsStrings(GeoAlgorithm):

    INPUT_LAYER = "INPUT_LAYER"
    FIELD_NAME = "FIELD_NAME"
    OUTPUT_HTML_FILE = "OUTPUT_HTML_FILE"

    MIN_LEN = "MIN_LEN"
    MAX_LEN = "MAX_LEN"
    MEAN_LEN = "MEAN_LEN"
    COUNT = "COUNT"
    EMPTY = "EMPTY"
    FILLED = "FILLED"
    UNIQUE = "UNIQUE"

    #===========================================================================
    # def getIcon(self):
    #    return QtGui.QIcon(os.path.dirname(__file__) + "/icons/basic_statistics.png")
    #===========================================================================

    def defineCharacteristics(self):
        self.name = "Basic statistics for text fields"
        self.group = "Vector table tools"

        self.addParameter(ParameterVector(self.INPUT_LAYER, "Input vector layer", ParameterVector.VECTOR_TYPE_ANY, False))
        self.addParameter(ParameterTableField(self.FIELD_NAME, "Field to calculate statistics on", self.INPUT_LAYER, ParameterTableField.DATA_TYPE_STRING))

        self.addOutput(OutputHTML(self.OUTPUT_HTML_FILE, "Statistics for text field"))

        self.addOutput(OutputNumber(self.MIN_LEN, "Minimum length"))
        self.addOutput(OutputNumber(self.MAX_LEN, "Maximum length"))
        self.addOutput(OutputNumber(self.MEAN_LEN, "Mean length"))
        self.addOutput(OutputNumber(self.COUNT, "Count"))
        self.addOutput(OutputNumber(self.EMPTY, "Number of empty values"))
        self.addOutput(OutputNumber(self.FILLED, "Number of non-empty values"))
        self.addOutput(OutputNumber(self.UNIQUE, "Number of unique values"))

    def processAlgorithm(self, progress):
        layer = QGisLayers.getObjectFromUri(self.getParameterValue(self.INPUT_LAYER))
        fieldName = self.getParameterValue(self.FIELD_NAME)

        outputFile = self.getOutputValue(self.OUTPUT_HTML_FILE)

        index = layer.fieldNameIndex(fieldName)

        sumValue = 0
        minValue = 0
        maxValue = 0
        meanValue = 0
        countEmpty = 0
        countFilled = 0

        isFirst = True
        values = []

        features = QGisLayers.features(layer)
        count = len(features)
        total = 100.0 / float(count)
        current = 0
        for ft in features:
            length = float(len(ft.attributes()[index]))

            if isFirst:
                minValue = length
                maxValue = length
                isFirst = False
            else:
                if length < minValue:
                    minValue = length
                if length > maxValue:
                    maxValue = length

            if length != 0.00:
                countFilled += 1
            else:
                countEmpty += 1

            values.append(length)
            sumValue += length

            current += 1
            progress.setPercentage(int(current * total))

        n = float(len(values))
        if n > 0:
            meanValue = sumValue / n

        uniqueValues = utils.getUniqueValuesCount(layer, index)

        data = []
        data.append("Minimum length: " + unicode(minValue))
        data.append("Maximum length: " + unicode(maxValue))
        data.append("Mean length: " + unicode(meanValue))
        data.append("Filled: " + unicode(countFilled))
        data.append("Empty: " + unicode(countEmpty))
        data.append("Count: " + unicode(count))
        data.append("Unique: " + unicode(uniqueValues))

        self.createHTML(outputFile, data)

        self.setOutputValue(self.MIN_LEN, minValue)
        self.setOutputValue(self.MAX_LEN, maxValue)
        self.setOutputValue(self.MEAN_LEN, meanValue)
        self.setOutputValue(self.FILLED, countFilled)
        self.setOutputValue(self.EMPTY, countEmpty)
        self.setOutputValue(self.COUNT, count)
        self.setOutputValue(self.UNIQUE, uniqueValues)

    def createHTML(self, outputFile, algData):
        f = codecs.open(outputFile, "w", encoding="utf-8")
        f.write('<html><head>')
        f.write('<meta http-equiv="Content-Type" content="text/html; charset=utf-8" /></head><body>')
        for s in algData:
            f.write("<p>" + str(s) + "</p>")
        f.write("</body></html>")
        f.close()
