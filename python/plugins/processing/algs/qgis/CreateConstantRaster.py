# -*- coding: utf-8 -*-

"""
***************************************************************************
    CreateConstantRaster.py
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

import os
import math
import struct

from qgis.core import (Qgis,
                       QgsErrorMessage,
                       QgsProcessingException,
                       QgsRasterBlock,
                       QgsRasterFileWriter,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterRasterDestination)
from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class CreateConstantRaster(QgisAlgorithm):
    EXTENT = 'EXTENT'
    TARGET_CRS = 'TARGET_CRS'
    PIXEL_SIZE = 'PIXEL_SIZE'
    NUMBER = 'NUMBER'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Raster tools')

    def groupId(self):
        return 'rastertools'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterExtent(self.EXTENT,
                                                       self.tr('Desired extent')))
        self.addParameter(QgsProcessingParameterCrs(self.TARGET_CRS,
                                                    self.tr('Target CRS'),
                                                    'ProjectCrs'))
        self.addParameter(QgsProcessingParameterNumber(self.PIXEL_SIZE,
                                                       self.tr('Pixel size'),
                                                       QgsProcessingParameterNumber.Double,
                                                       0.1, False, 0.01))
        self.addParameter(QgsProcessingParameterNumber(self.NUMBER,
                                                       self.tr('Constant value'),
                                                       QgsProcessingParameterNumber.Double,
                                                       defaultValue=1))
        self.addParameter(QgsProcessingParameterRasterDestination(self.OUTPUT, self.tr('Constant')))

    def name(self):
        return 'createconstantrasterlayer'

    def displayName(self):
        return self.tr('Create constant raster layer')

    def processAlgorithm(self, parameters, context, feedback):
        crs = self.parameterAsCrs(parameters, self.TARGET_CRS, context)
        extent = self.parameterAsExtent(parameters, self.EXTENT, context, crs)
        value = self.parameterAsDouble(parameters, self.NUMBER, context)
        pixelSize = self.parameterAsDouble(parameters, self.PIXEL_SIZE, context)

        outputFile = self.parameterAsOutputLayer(parameters, self.OUTPUT, context)
        outputFormat = QgsRasterFileWriter.driverForExtension(os.path.splitext(outputFile)[1])

        rows = int(max([math.ceil(extent.height() / pixelSize), 1.0]))
        cols = int(max([math.ceil(extent.width() / pixelSize), 1.0]))

        writer = QgsRasterFileWriter(outputFile)
        writer.setOutputProviderKey('gdal')
        writer.setOutputFormat(outputFormat)
        provider = writer.createOneBandRaster(Qgis.Float32, cols, rows, extent, crs)
        if provider is None:
            raise QgsProcessingException(self.tr("Could not create raster output: {}").format(outputFile))
        if not provider.isValid():
            raise QgsProcessingException(self.tr("Could not create raster output {}: {}").format(outputFile,
                                                                                                 provider.error().message(
                                                                                                     QgsErrorMessage.Text)))

        provider.setNoDataValue(1, -9999)

        data = [value] * cols
        block = QgsRasterBlock(Qgis.Float32, cols, 1)
        block.setData(struct.pack('{}f'.format(len(data)), *data))

        total = 100.0 / rows if rows else 0
        for i in range(rows):
            if feedback.isCanceled():
                break

            provider.writeBlock(block, 1, 0, i)
            feedback.setProgress(int(i * rows))

        provider.setEditable(False)

        return {self.OUTPUT: outputFile}
