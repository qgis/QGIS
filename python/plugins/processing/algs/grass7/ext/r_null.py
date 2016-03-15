# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_null.py
    ---------------------
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
    if alg.getParameterValue(u'setnull') or alg.getParameterValue(u'null'):
        return None

    return alg.tr("You need to set at least 'setnull' or 'null' parameters for this algorithm!")


def processInputs(alg):
    """Prepare the GRASS import commands"""
    inputRaster = alg.getParameterValue(u'map')
    if inputRaster in alg.exportedLayers.keys():
        return
    else:
        alg.setSessionProjectionFromLayer(inputRaster, alg.commands)

    destFilename = alg.getTempFilename()
    alg.exportedLayers[inputRaster] = destFilename
    command = 'r.in.gdal input={} output={} band=1 --overwrite -o'.format(inputRaster, destFilename)
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
    # We temporary remove the output 'sequence'
    output = alg.getOutputFromName(u'output')
    alg.removeOutputFromName(u'output')

    # Launch the algorithm
    alg.processCommand()

    # We re-add the previous output
    alg.addOutput(output)


def processOutputs(alg):
    output = alg.getOutputValue(u'output')
    command = u"r.out.gdal -c createopt=\"TFW=YES,COMPRESS=LZW\" input={} output=\"{}\" --overwrite".format(
        alg.exportedLayers[alg.getParameterValue(u'map')],
        output
    )
    alg.commands.append(command)
    alg.outputCommands.append(command)
