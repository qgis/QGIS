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

from PyQt4 import QtGui
from qgis.core import *

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterMultipleInput
from processing.core.parameters import ParameterExtent
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterRaster
from GrassUtils import GrassUtils
from processing.tools.system import *
from processing.tools import dataobjects


class nviz(GeoAlgorithm):

    ELEVATION = 'ELEVATION'
    VECTOR = 'VECTOR'
    COLOR = 'COLOR'
    GRASS_REGION_EXTENT_PARAMETER = 'GRASS_REGION_PARAMETER'
    GRASS_REGION_CELLSIZE_PARAMETER = 'GRASS_REGION_CELLSIZE_PARAMETER'

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + '/../images/grass.png')

    def defineCharacteristics(self):
        self.name = 'nviz'
        self.group = 'Visualization(NVIZ)'
        self.addParameter(ParameterMultipleInput(nviz.ELEVATION,
                          'Raster file(s) for elevation',
                          ParameterMultipleInput.TYPE_RASTER, True))
        self.addParameter(ParameterMultipleInput(nviz.VECTOR, 'Vector lines/areas overlay file(s)',
                          ParameterMultipleInput.TYPE_VECTOR_ANY, True))
        self.addParameter(ParameterMultipleInput(nviz.COLOR, 'Raster file(s) for color',
                          ParameterMultipleInput.TYPE_RASTER, True))
        self.addParameter(ParameterExtent(nviz.GRASS_REGION_EXTENT_PARAMETER,
                          'GRASS region extent'))
        self.addParameter(ParameterNumber(self.GRASS_REGION_CELLSIZE_PARAMETER,
                          'GRASS region cellsize (leave 0 for default)', 0,
                          None, 0.0))

    def processAlgorithm(self, progress):
        commands = []
        vector = self.getParameterValue(self.VECTOR)
        elevation = self.getParameterValue(self.ELEVATION)
        color = self.getParameterValue(self.COLOR)

        region = \
            str(self.getParameterValue(self.GRASS_REGION_EXTENT_PARAMETER))
        regionCoords = region.split(',')
        command = 'g.region '
        command += 'n=' + str(regionCoords[3])
        command += ' s=' + str(regionCoords[2])
        command += ' e=' + str(regionCoords[1])
        command += ' w=' + str(regionCoords[0])
        cellsize = self.getParameterValue(self.GRASS_REGION_CELLSIZE_PARAMETER)
        if cellsize:
            command += ' res=' + str(cellsize)
        else:
            command += ' res=' + str(self.getDefaultCellsize())
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
        filename = 'tmp' + str(time.time()).replace('.', '') \
            + str(getNumExportedLayers())
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
                            cellsize = max(cellsize,
                                    (layer.extent().xMaximum()
                                    - layer.extent().xMinimum())
                                    / layer.width())

        if cellsize == 0:
            cellsize = 1
        return cellsize
