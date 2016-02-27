# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_blend_rgb.py
    --------------
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

from processing.core.outputs import getOutputFromString


def processInputs(alg):
    # If there is another raster to copy categories from
    # we need to import it with r.in.gdal rather than r.external
    first = alg.getParameterValue(u'first')
    second = alg.getParameterValue(u'second')
    if first in alg.exportedLayers.keys() and second in alg.exportedLayers.keys():
        return

    for raster in [first, second]:
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
    # We need to remove all outputs
    basename = getOutputFromString('OutputRaster|output|Output basename')
    basename.value = 'output'
    alg.addOutput(basename)
    outputNames = ['output_{}'.format(f) for f in ['red', 'green', 'blue']]
    outputs = [alg.getOutputFromName(f) for f in outputNames]
    for output in outputNames:
        alg.exportedLayers[alg.getOutputValue(output)] = 'output' + alg.uniqueSufix
        alg.removeOutputFromName(output)

    alg.processCommand()

    # And to re-add them
    alg.removeOutputFromName('output')
    for output in outputs:
        alg.addOutput(output)


def processOutputs(alg):
    # Export each color raster
    colors = ['red', 'green', 'blue']
    for color in colors:
        output = alg.getOutputValue('output_{}'.format(color))
        command = "r.out.gdal input={} output=\"{}\" createopt=\"TFW=YES,COMPRESS=LZW\"".format(
            alg.exportedLayers[output] + '.' + color[0], output)
        alg.commands.append(command)
        alg.outputCommands.append(command)
