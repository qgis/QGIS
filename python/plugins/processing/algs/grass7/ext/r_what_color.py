# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_what_color.py
    ---------------
    Date                 : February 2016
    Copyright            : (C) 2016 by Médéric Ribreux
    Email                : medspx at medspx dot fr
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""
from builtins import str

__author__ = 'Médéric Ribreux'
__date__ = 'February 2016'
__copyright__ = '(C) 2016, Médéric Ribreux'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


def processInputs(alg):
    # We need to import all the bands and color tables of the input rasters
    raster = alg.getParameterValue('input')
    if raster in list(alg.exportedLayers.keys()):
        return

    alg.setSessionProjectionFromLayer(raster, alg.commands)
    destFilename = alg.getTempFilename()
    alg.exportedLayers[raster] = destFilename
    command = 'r.in.gdal input={} output={} --overwrite -o'.format(raster, destFilename)
    alg.commands.append(command)

    alg.setSessionProjectionFromProject(alg.commands)

    region = str(alg.getParameterValue(alg.GRASS_REGION_EXTENT_PARAMETER))
    regionCoords = region.split(',')
    command = 'g.region'
    command += ' -a'
    command += ' n=' + str(regionCoords[3])
    command += ' s=' + str(regionCoords[2])
    command += ' e=' + str(regionCoords[1])
    command += ' w=' + str(regionCoords[0])
    cellsize = alg.getParameterValue(alg.GRASS_REGION_CELLSIZE_PARAMETER)
    if cellsize:
        command += ' res=' + str(cellsize)
    else:
        command += ' res=' + str(alg.getDefaultCellsize())
    alignToResolution = alg.getParameterValue(alg.GRASS_REGION_ALIGN_TO_RESOLUTION)
    if alignToResolution:
        command += ' -a'
    alg.commands.append(command)
