# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_rgb.py
    --------
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

__author__ = 'Médéric Ribreux'
__date__ = 'February 2016'
__copyright__ = '(C) 2016, Médéric Ribreux'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


def processInputs(alg):
    # We need to import all the bands and color tables of the input raster
    raster = alg.getParameterValue('input')
    if raster in alg.exportedLayers.keys():
        return

    alg.setSessionProjectionFromLayer(raster, alg.commands)
    destFilename = alg.getTempFilename()
    alg.exportedLayers[raster] = destFilename
    command = 'r.in.gdal input={} output={} --overwrite -o'.format(raster, destFilename)
    alg.commands.append(command)

    alg.setSessionProjectionFromProject(alg.commands)

    region = unicode(alg.getParameterValue(alg.GRASS_REGION_EXTENT_PARAMETER))
    regionCoords = region.split(',')
    command = 'g.region'
    command += ' -a'
    command += ' n=' + unicode(regionCoords[3])
    command += ' s=' + unicode(regionCoords[2])
    command += ' e=' + unicode(regionCoords[1])
    command += ' w=' + unicode(regionCoords[0])
    cellsize = alg.getParameterValue(alg.GRASS_REGION_CELLSIZE_PARAMETER)
    if cellsize:
        command += ' res=' + unicode(cellsize)
    else:
        command += ' res=' + unicode(alg.getDefaultCellsize())
    alignToResolution = alg.getParameterValue(alg.GRASS_REGION_ALIGN_TO_RESOLUTION)
    if alignToResolution:
        command += ' -a'
    alg.commands.append(command)


def processCommand(alg):
    # We need to introduce something clever:
    # if the input raster is multiband: export each component directly
    raster = alg.exportedLayers[alg.getParameterValue('input')]
    for color in ['red', 'green', 'blue']:
        alg.exportedLayers[alg.getOutputValue(color)] = color + alg.uniqueSufix

    commands = ["if [ $(g.list type=rast pattern='{}.*' | wc -l) -eq \"0\" ]; then".format(raster)]
    commands.append("  r.rgb input={} red={} green={} blue={} --overwrite".format(
        raster,
        alg.exportedLayers[alg.getOutputValue('red')],
        alg.exportedLayers[alg.getOutputValue('green')],
        alg.exportedLayers[alg.getOutputValue('blue')]
    ))
    commands.append("fi")
    alg.commands.extend(commands)


def processOutputs(alg):
    raster = alg.exportedLayers[alg.getParameterValue('input')]
    commands = ["if [ $(g.list type=rast pattern='{}.*' | wc -l) -eq \"0\" ]; then".format(raster)]
    for color in ['red', 'green', 'blue']:
        commands.append("  r.out.gdal -t input={} output={} createopt=\"TFW=YES,COMPRESS=LZW\" --overwrite".format(
            alg.exportedLayers[alg.getOutputValue(color)],
            alg.getOutputValue(color)
        ))
    commands.append("else")
    for color in ['red', 'green', 'blue']:
        commands.append("  r.out.gdal -t input={} output={} createopt=\"TFW=YES,COMPRESS=LZW\" --overwrite".format(
            '{}.{}'.format(raster, color),
            alg.getOutputValue(color)
        ))
    commands.append("fi")
    alg.commands.extend(commands)
