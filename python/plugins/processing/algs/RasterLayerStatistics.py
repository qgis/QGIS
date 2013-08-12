# -*- coding: utf-8 -*-

"""
***************************************************************************
    RasterLayerStatistics.py
    ---------------------
    Date                 : January 2013
    Copyright            : (C) 2013 by Victor Olaya
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
__date__ = 'January 2013'
__copyright__ = '(C) 2013, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import math
from PyQt4.QtCore import *
from qgis.core import *
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.QGisLayers import QGisLayers
from processing.parameters.ParameterRaster import ParameterRaster
from processing.tools import raster
from processing.outputs.OutputNumber import OutputNumber
from processing.outputs.OutputHTML import OutputHTML

class RasterLayerStatistics(GeoAlgorithm):

    INPUT = "INPUT"

    MIN = "MIN"
    MAX = "MAX"
    SUM = "SUM"
    MEAN = "MEAN"
    COUNT = "COUNT"
    NO_DATA_COUNT = "NO_DATA_COUNT"
    STD_DEV = "STD_DEV"
    OUTPUT_HTML_FILE = "OUTPUT_HTML_FILE"

    def processAlgorithm(self, progress):
        outputFile = self.getOutputValue(self.OUTPUT_HTML_FILE)
        uri = self.getParameterValue(self.INPUT)
        layer = QGisLayers.getObjectFromUri(uri)
        values = raster.scanraster(layer, progress)

        n = 0
        nodata = 0
        mean = 0
        M2 = 0
        sum = 0
        minvalue = None
        maxvalue = None

        for v in values:
            if v is not None:
                sum += v
                n = n + 1
                delta = v - mean
                mean = mean + delta/n
                M2 = M2 + delta*(v - mean)
                if minvalue is None:
                    minvalue = v
                    maxvalue = v
                else:
                    minvalue = min(v, minvalue)
                    maxvalue = max(v, maxvalue)
            else:
                nodata += 1

        variance = M2/(n - 1)
        stddev = math.sqrt(variance)

        data = []
        data.append("Valid cells: " + unicode(n))
        data.append("No-data cells: " + unicode(nodata))
        data.append("Minimum value: " + unicode(minvalue))
        data.append("Maximum value: " + unicode(maxvalue))
        data.append("Sum: " + unicode(sum))
        data.append("Mean value: " + unicode(mean))
        data.append("Standard deviation: " + unicode(stddev))

        self.createHTML(outputFile, data)

        self.setOutputValue(self.COUNT, n)
        self.setOutputValue(self.NO_DATA_COUNT, nodata)
        self.setOutputValue(self.MIN, minvalue)
        self.setOutputValue(self.MAX, maxvalue)
        self.setOutputValue(self.SUM, sum)
        self.setOutputValue(self.MEAN, mean)
        self.setOutputValue(self.STD_DEV, stddev)


    def defineCharacteristics(self):
        self.name = "Raster layer statistics"
        self.group = "Raster tools"
        self.addParameter(ParameterRaster(self.INPUT, "Input layer"))
        self.addOutput(OutputHTML(self.OUTPUT_HTML_FILE, "Statistics"))
        self.addOutput(OutputNumber(self.MIN, "Minimum value"))
        self.addOutput(OutputNumber(self.MAX, "Maximum value"))
        self.addOutput(OutputNumber(self.SUM, "Sum"))
        self.addOutput(OutputNumber(self.MEAN, "Mean value"))
        self.addOutput(OutputNumber(self.COUNT, "valid cells count"))
        self.addOutput(OutputNumber(self.COUNT, "No-data cells count"))
        self.addOutput(OutputNumber(self.STD_DEV, "Standard deviation"))

    def createHTML(self, outputFile, algData):
        f = open(outputFile, "w")
        for s in algData:
            f.write("<p>" + str(s) + "</p>")
        f.close()
