# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_colors.py
    -----------
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

import os


def checkParameterValuesBeforeExecuting(alg):
    """ Verify if we have the right parameters """
    if alg.getParameterValue('rules_txt') and alg.getParameterValue('rules'):
        return alg.tr("You need to set either inline rules or a rules file!")

    return None


def processInputs(alg):
    # import all rasters with their color tables (and their bands)
    # We need to import all the bands and color tables of the input rasters
    rasters = alg.getParameterValue('map').split(',')
    for raster in rasters:
        if raster in alg.exportedLayers.keys():
            continue

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
    # remove output before processCommand
    output = alg.getOutputFromName('output_dir')
    alg.removeOutputFromName('output_dir')
    color = alg.getParameterFromName('color')
    if color.value == 0:
        alg.parameters.remove(color)

    # Handle rules
    txtRules = alg.getParameterFromName('rules_txt')
    if txtRules.value:
        # Creates a temporary txt file
        tempRulesName = alg.getTempFilename()

        # Inject rules into temporary txt file
        with open(tempRulesName, "w") as tempRules:
            tempRules.write(txtRules.value)

        # Use temporary file as rules file
        alg.setParameterValue('rules', tempRulesName)
        alg.parameters.remove(txtRules)

    alg.processCommand()

    # re-add the previous output
    alg.addOutput(output)
    alg.addParameter(color)
    alg.addParameter(txtRules)


def processOutputs(alg):
    # Export all rasters with their color tables (and their bands)
    rasters = [alg.exportedLayers[f] for f in alg.getParameterValue('map').split(',')]
    output_dir = alg.getOutputValue('output_dir')
    for raster in rasters:
        command = u"r.out.gdal -t createopt=\"TFW=YES,COMPRESS=LZW\" input={} output=\"{}\" --overwrite".format(
            raster,
            os.path.join(output_dir, raster + '.tif')
        )
        alg.commands.append(command)
        alg.outputCommands.append(command)
