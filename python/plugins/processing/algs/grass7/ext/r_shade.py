# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_shade.py
    ----------
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


def checkParameterValuesBeforeExecuting(alg):
    """ Verify if we have the right parameters """
    if alg.getParameterValue('brighten') and alg.getParameterValue('bgcolor'):
        return alg.tr("You need to set either a brighten percentage or a NULL color!")
    return None


def processInputs(alg):
    # We need to import all the bands and color tables of the input rasters
    shade = alg.getParameterValue('shade')
    color = alg.getParameterValue('color')
    if color in alg.exportedLayers.keys():
        return

    for raster, method in (shade, 'r.external'), (color, 'r.in.gdal'):
        alg.setSessionProjectionFromLayer(raster, alg.commands)

        destFilename = alg.getTempFilename()
        alg.exportedLayers[raster] = destFilename
        command = '{} input={} output={} --overwrite -o'.format(method, raster, destFilename)
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


def processOutputs(alg):
    # Keep color table ?
    output = alg.getOutputValue(u'output')
    command = u"r.out.gdal -t createopt=\"TFW=YES,COMPRESS=LZW\" input={} output=\"{}\" --overwrite".format(
        alg.exportedLayers[output],
        output
    )
    alg.commands.append(command)
    alg.outputCommands.append(command)
