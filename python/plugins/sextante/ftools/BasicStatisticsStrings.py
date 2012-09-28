import os.path

from PyQt4 import QtGui
from PyQt4.QtCore import *

from qgis.core import *

from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.core.QGisLayers import QGisLayers

from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterTableField import ParameterTableField
from sextante.parameters.ParameterBoolean import ParameterBoolean

from sextante.outputs.OutputHTML import OutputHTML
from sextante.outputs.OutputNumber import OutputNumber

from sextante.ftools import FToolsUtils as utils

class BasicStatisticsStrings(GeoAlgorithm):

    INPUT_LAYER = "INPUT_LAYER"
    FIELD_NAME = "FIELD_NAME"
    USE_SELECTION = "USE_SELECTION"
    OUTPUT_HTML_FILE = "OUTPUT_HTML_FILE"

    MIN_LEN = "MIN_LEN"
    MAX_LEN = "MAX_LEN"
    MEAN_LEN = "MEAN_LEN"
    COUNT = "COUNT"
    EMPTY = "EMPTY"
    FILLED = "FILLED"
    UNIQUE = "UNIQUE"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/icons/basic_statistics.png")

    def defineCharacteristics(self):
        self.name = "Basic statistics for text fields"
        self.group = "Analysis tools"

        self.addParameter(ParameterVector(self.INPUT_LAYER, "Input vector layer", ParameterVector.VECTOR_TYPE_ANY, False))
        self.addParameter(ParameterTableField(self.FIELD_NAME, "Field to calculate statistics on", self.INPUT_LAYER, ParameterTableField.DATA_TYPE_STRING))
        self.addParameter(ParameterBoolean(self.USE_SELECTION, "Use selection", False))

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
        useSelection = self.getParameterValue(self.USE_SELECTION)

        outputFile = self.getOutputValue(self.OUTPUT_HTML_FILE)

        index = layer.fieldNameIndex(fieldName)
        layer.select([index], QgsRectangle(), False)

        count = 0
        sumValue = 0
        minValue = 0
        maxValue = 0
        meanValue = 0
        countEmpty = 0
        countFilled = 0

        isFirst = True
        values = []

        if useSelection:
            selection = layer.selectedFeatures()
            count = layer.selectedFeatureCount()
            total = 100.0 / float(count)
            current = 0

            for f in selection:
                length = float(len(f.attributeMap()[index].toString()))

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
        else:
            count = layer.featureCount()
            total = 100.0 / float(count)
            current = 0

            ft = QgsFeature()
            while layer.nextFeature(ft):
                length = float(len(ft.attributeMap()[index].toString()))

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

        uniqueValues = utils.getUniqueValuesCount(layer, index, useSelection)

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
        f = open(outputFile, "w")
        for s in algData:
            f.write("<p>" + str(s) + "</p>")
        f.close()
