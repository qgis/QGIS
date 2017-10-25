# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_mapcalc.py
    ------------
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

from os import path


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    """ Verify if we have the right parameters """
    if (alg.parameterAsString(parameters, 'expression', context)
            and alg.parameterAsString(parameters, 'file', context)):
        return alg.tr("You need to set either inline expression or a rules file!")

    return None


def processInputs(alg, parameters, context):
    # We need to use the same raster names than in QGIS
    if alg.getParameterValue('maps'):
        rasters = alg.getParameterValue('maps').split(',')
        for raster in rasters:
            if raster in list(alg.exportedLayers.keys()):
                continue

            alg.setSessionProjectionFromLayer(raster, alg.commands)
            destFilename = path.splitext(path.basename(raster))[0]
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
        command += ' res=' + str(alg.getDefaultCellsize(parameters, context))
    alignToResolution = alg.getParameterValue(alg.GRASS_REGION_ALIGN_TO_RESOLUTION)
    if alignToResolution:
        command += ' -a'
    alg.commands.append(command)


def processCommand(alg, parameters, context):
    # Remove output for command
    output_dir = alg.getOutputFromName('output_dir')
    maps = alg.getParameterFromName('maps')
    alg.removeOutputFromName('output_dir')
    alg.parameters.remove(maps)
    alg.processCommand()
    alg.addOutput(output_dir)
    alg.addParameter(maps)
