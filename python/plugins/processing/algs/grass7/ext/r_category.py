# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_category.py
    -------------
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

import codecs
from processing.tools.system import getTempFilename


def checkParameterValuesBeforeExecuting(alg):
    """ Verify if we have the right parameters """
    rules = alg.getParameterValue(u'rules')
    txtrules = alg.getParameterValue(u'txtrules')
    raster = alg.getParameterValue(u'raster')

    if rules and txtrules:
        return alg.tr("You need to set either a rules file or write directly the rules!")
    elif (rules and raster) or (txtrules and raster):
        return alg.tr("You need to set either rules or a raster from which to copy categories!")

    return None


def processInputs(alg):
    # If there is another raster to copy categories from
    # we need to import it with r.in.gdal rather than r.external
    inputRaster = alg.getParameterValue(u'map')
    copyRaster = alg.getParameterValue(u'raster')
    if copyRaster:
        if copyRaster in alg.exportedLayers.keys():
            return

        for raster, method in (inputRaster, 'r.external'), (copyRaster, 'r.in.gdal'):
            alg.setSessionProjectionFromLayer(raster, alg.commands)

            destFilename = alg.getTempFilename()
            alg.exportedLayers[raster] = destFilename
            command = '{} input={} output={} band=1 --overwrite -o'.format(method, raster, destFilename)
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
    else:
        alg.processInputs()


def processCommand(alg):
    # We temporary remove the output
    out = alg.getOutputFromName('output')
    mapParam = alg.getParameterValue('map')
    alg.exportedLayers[out.value] = alg.exportedLayers[mapParam]
    alg.removeOutputFromName('output')
    txtRulesParam = alg.getParameterFromName(u'txtrules')
    rules = alg.getParameterFromName(u'rules')

    # Handle inline rules
    if txtRulesParam.value:
        # Creates a temporary txt file
        tempRulesName = getTempFilename('txt')

        # Inject rules into temporary txt file
        with codecs.open(tempRulesName, 'w', 'utf-8') as tempRules:
            tempRules.write(txtRulesParam.value)

        # Replace rules with temporary file
        rules.value = tempRulesName
        alg.parameters.remove(txtRulesParam)

    alg.processCommand()

    # We re-add the new output
    alg.addOutput(out)


def processOutputs(alg):
    # Output results ('from' table and output table)
    out = alg.getOutputValue('output')
    command = u"r.out.gdal --overwrite -t -c createopt=\"TFW=YES,COMPRESS=LZW\" input={} output=\"{}\"".format(
        alg.exportedLayers[out], out)
    alg.commands.append(command)
    alg.outputCommands.append(command)
