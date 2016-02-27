# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_colors_stddev.py
    ------------------
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
    # We need to import all the bands and to preserve color table
    raster = alg.getParameterValue('map')
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
    # We need to remove output
    output = alg.getOutputFromName('output')
    alg.exportedLayers[output.value] = output.name + alg.uniqueSufix
    alg.removeOutputFromName('output')
    alg.processCommand()
    alg.addOutput(output)


def processOutputs(alg):
    # We need to export the raster with all its bands and its color table
    output = alg.getOutputValue('output')
    raster = alg.getParameterFromName('map')

    # Get the list of rasters matching the basename
    command = "r.out.gdal -t input={} output=\"{}\" createopt=\"TFW=YES,COMPRESS=LZW\"".format(
        alg.exportedLayers[raster.value], output)
    alg.commands.append(command)
    alg.outputCommands.append(command)
