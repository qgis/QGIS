# -*- coding: utf-8 -*-

"""
***************************************************************************
    self.py
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

from qgis.core import *
from processing.algs.gdal.GdalAlgorithm import GdalAlgorithm
from processing.core.parameters import ParameterRaster
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterCrs
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterString
from processing.core.outputs import OutputRaster
from processing.algs.gdal.GdalUtils import GdalUtils


class warp(GdalAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    SOURCE_SRS = 'SOURCE_SRS'
    DEST_SRS = 'DEST_SRS '
    METHOD = 'METHOD'
    METHOD_OPTIONS = ['near', 'bilinear', 'cubic', 'cubicspline', 'lanczos']
    TR = 'TR'
    EXTRA = 'EXTRA'
    RTYPE = 'RTYPE'

    TYPE = ['Byte', 'Int16', 'UInt16', 'UInt32', 'Int32', 'Float32', 'Float64',
            'CInt16', 'CInt32', 'CFloat32', 'CFloat64']

    def defineCharacteristics(self):
        self.name = 'Warp (reproject)'
        self.group = '[GDAL] Projections'
        self.addParameter(ParameterRaster(self.INPUT, self.tr('Input layer'), False))
        self.addParameter(ParameterCrs(self.SOURCE_SRS,
            self.tr('Source SRS'), 'EPSG:4326'))
        self.addParameter(ParameterCrs(self.DEST_SRS,
            self.tr('Destination SRS'), 'EPSG:4326'))
        self.addParameter(ParameterNumber(self.TR,
            self.tr('Output file resolution in target georeferenced units (leave 0 for no change)'),
            0.0, None, 0.0))
        self.addParameter(ParameterSelection(self.METHOD,
            self.tr('Resampling method'), self.METHOD_OPTIONS))
        self.addParameter(ParameterString(self.EXTRA,
            self.tr('Additional creation parameters'), '', optional=True))
        self.addParameter(ParameterSelection(self.RTYPE,
            self.tr('Output raster type'), self.TYPE, 5))

        self.addOutput(OutputRaster(self.OUTPUT, self.tr('Output layer')))

    def processAlgorithm(self, progress):
        arguments = []
        arguments.append('-ot')
        arguments.append(self.TYPE[self.getParameterValue(self.RTYPE)])
        arguments.append('-s_srs')
        arguments.append(str(self.getParameterValue(self.SOURCE_SRS)))
        arguments.append('-t_srs')
        crsId = self.getParameterValue(self.DEST_SRS)
        self.crs = QgsCoordinateReferenceSystem(crsId)
        arguments.append(str(crsId))
        arguments.append('-r')
        arguments.append(
                self.METHOD_OPTIONS[self.getParameterValue(self.METHOD)])
        arguments.append('-of')
        out = self.getOutputValue(self.OUTPUT)
        arguments.append(GdalUtils.getFormatShortNameFromFilename(out))
        if self.getParameterValue(self.TR) != 0:
            arguments.append('-tr')
            arguments.append(str(self.getParameterValue(self.TR)))
            arguments.append(str(self.getParameterValue(self.TR)))
        extra = str(self.getParameterValue(self.EXTRA))
        if len(extra) > 0:
            arguments.append(extra)
        arguments.append(self.getParameterValue(self.INPUT))
        arguments.append(out)

        GdalUtils.runGdal(['gdalwarp', GdalUtils.escapeAndJoin(arguments)],
                          progress)
