# -*- coding: utf-8 -*-

"""
***************************************************************************
    nviz.py
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

import os
import time

from PyQt4.QtGui import QIcon
from qgis.core import QgsRasterLayer

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterMultipleInput
from processing.core.parameters import ParameterExtent
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterRaster
from GrassUtils import GrassUtils
from processing.tools.system import getNumExportedLayers
from processing.tools import dataobjects

pluginPath = os.path.normpath(os.path.join(
    os.path.split(os.path.dirname(__file__))[0], os.pardir))


class nviz(GeoAlgorithm):

    ELEVATION = 'ELEVATION'
    VECTOR = 'VECTOR'
    COLOR = 'COLOR'
    GRASS_REGION_EXTENT_PARAMETER = 'GRASS_REGION_PARAMETER'
    GRASS_REGION_CELLSIZE_PARAMETER = 'GRASS_REGION_CELLSIZE_PARAMETER'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'grass.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('nviz')
        self.group, self.i18n_group = self.trAlgorithm('Visualization(NVIZ)')
        self.addParameter(ParameterMultipleInput(nviz.ELEVATION,
                                                 self.tr('Raster file(s) for elevation'),
                                                 ParameterMultipleInput.TYPE_RASTER, True))
        self.addParameter(ParameterMultipleInput(nviz.VECTOR,
                                                 self.tr('Vector lines/areas overlay file(s)'),
                                                 ParameterMultipleInput.TYPE_VECTOR_ANY, True))
        self.addParameter(ParameterMultipleInput(nviz.COLOR,
                                                 self.tr('Raster file(s) for color'),
                                                 ParameterMultipleInput.TYPE_RASTER, True))
        self.addParameter(ParameterExtent(nviz.GRASS_REGION_EXTENT_PARAMETER,
                                          self.tr('GRASS region extent')))
        self.addParameter(ParameterNumber(self.GRASS_REGION_CELLSIZE_PARAMETER,
                                          self.tr('GRASS region cellsize (leave 0 for default)'),
                                          0, None, 0.0))

    def processAlgorithm(self, progress):
        commands = []
        vector = self.getParameterValue(self.VECTOR)
        elevation = self.getParameterValue(self.ELEVATION)
        color = self.getParameterValue(self.COLOR)

        region = \
            unicode(self.getParameterValue(self.GRASS_REGION_EXTENT_PARAMETER))
        regionCoords = region.split(',')
        command = 'g.region '
        command += 'n=' + unicode(regionCoords[3])
        command += ' s=' + unicode(regionCoords[2])
        command += ' e=' + unicode(regionCoords[1])
        command += ' w=' + unicode(regionCoords[0])
        cellsize = self.getParameterValue(self.GRASS_REGION_CELLSIZE_PARAMETER)
        if cellsize:
            command += ' res=' + unicode(cellsize)
        else:
            command += ' res=' + unicode(self.getDefaultCellsize())
        commands.append(command)

        command = 'nviz'
        if vector:
            layers = vector.split(';')
            for layer in layers:
                (cmd, newfilename) = self.exportVectorLayer(layer)
                commands.append(cmd)
                vector = vector.replace(layer, newfilename)
            command += ' vector=' + vector.replace(';', ',')
        if color:
            layers = color.split(';')
            for layer in layers:
                (cmd, newfilename) = self.exportRasterLayer(layer)
                commands.append(cmd)
                color = color.replace(layer, newfilename)
            command += ' color=' + color.replace(';', ',')
        if elevation:
            layers = elevation.split(';')
            for layer in layers:
                (cmd, newfilename) = self.exportRasterLayer(layer)
                commands.append(cmd)
                elevation = elevation.replace(layer, newfilename)
            command += ' elevation=' + elevation.replace(';', ',')
        if elevation is None and vector is None:
            command += ' -q'
        commands.append(command)
        GrassUtils.createTempMapset()
        GrassUtils.executeGrass(commands, progress)

    def getTempFilename(self):
        filename = 'tmp' + unicode(time.time()).replace('.', '') \
            + unicode(getNumExportedLayers())
        return filename

    def exportVectorLayer(self, layer):
        destFilename = self.getTempFilename()
        command = 'v.in.ogr'
        command += ' min_area=-1'
        command += ' dsn="' + os.path.dirname(layer) + '"'
        command += ' layer=' + os.path.basename(layer)[:-4]
        command += ' output=' + destFilename
        command += ' --overwrite -o'
        return (command, destFilename)

    def exportRasterLayer(self, layer):
        destFilename = self.getTempFilename()
        command = 'r.in.gdal'
        command += ' input="' + layer + '"'
        command += ' band=1'
        command += ' out=' + destFilename
        command += ' --overwrite -o'
        return (command, destFilename)

    def getDefaultCellsize(self):
        cellsize = 0
        for param in self.parameters:
            if param.value:
                if isinstance(param, ParameterRaster):
                    if isinstance(param.value, QgsRasterLayer):
                        layer = param.value
                    else:
                        layer = dataobjects.getObjectFromUri(param.value)
                    cellsize = max(cellsize, (layer.extent().xMaximum()
                                   - layer.extent().xMinimum())
                                   / layer.width())
                elif isinstance(param, ParameterMultipleInput):

                    layers = param.value.split(';')
                    for layername in layers:
                        layer = dataobjects.getObjectFromUri(layername)
                        if isinstance(layer, QgsRasterLayer):
                            cellsize = max(cellsize, (
                                layer.extent().xMaximum()
                                - layer.extent().xMinimum())
                                / layer.width())

        if cellsize == 0:
            cellsize = 1
        return cellsize
