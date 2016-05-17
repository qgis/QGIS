# -*- coding: utf-8 -*-

"""
***************************************************************************
    rasterize_over.py
    ---------------------
    Date                 : September 2013
    Copyright            : (C) 2013 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'September 2013'
__copyright__ = '(C) 2013, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterTableField

from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.algs.gdal.GdalUtils import GdalUtils

from processing.tools.vector import ogrConnectionString, ogrLayerName

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class rasterize_over(GdalAlgorithm):

    INPUT = 'INPUT'
    INPUT_RASTER = 'INPUT_RASTER'
    FIELD = 'FIELD'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'gdaltools', 'rasterize.png'))

    def commandLineName(self):
        return "gdalogr:rasterize_over"

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Rasterize (write over existing raster)')
        self.group, self.i18n_group = self.trAlgorithm('[GDAL] Conversion')
        self.addParameter(ParameterVector(self.INPUT, self.tr('Input layer')))
        self.addParameter(ParameterTableField(self.FIELD,
                                              self.tr('Attribute field'), self.INPUT))
        self.addParameter(ParameterRaster(self.INPUT_RASTER,
                                          self.tr('Existing raster layer'), False))

    def getConsoleCommands(self):
        inLayer = self.getParameterValue(self.INPUT)
        ogrLayer = ogrConnectionString(inLayer)[1:-1]
        inRasterLayer = self.getParameterValue(self.INPUT_RASTER)
        ogrRasterLayer = ogrConnectionString(inRasterLayer)[1:-1]

        arguments = []
        arguments.append('-a')
        arguments.append(unicode(self.getParameterValue(self.FIELD)))

        arguments.append('-l')
        arguments.append(ogrLayerName(inLayer))
        arguments.append(ogrLayer)
        arguments.append(ogrRasterLayer)

        return ['gdal_rasterize', GdalUtils.escapeAndJoin(arguments)]
