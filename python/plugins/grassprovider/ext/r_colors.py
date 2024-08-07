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

import os
from grassprovider.grass_utils import GrassUtils
from processing.tools.system import getTempFilename


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    """ Verify if we have the right parameters """
    txtRules = alg.parameterAsString(parameters, 'rules_txt', context)
    rules = alg.parameterAsString(parameters, 'rules', context)
    if txtRules and rules:
        return False, alg.tr("You need to set either inline rules or a rules file!")

    rules_or_txtRules = bool(txtRules or rules)
    color = bool(alg.parameterAsEnum(parameters, 'color', context))
    raster = bool(alg.parameterAsString(parameters, 'raster', context))
    if not sum([rules_or_txtRules, color, raster]) == 1:
        return False, alg.tr("The color table, color rules and raster map parameters are mutually exclusive. You need to set one and only one of them!")

    return True, None


def processInputs(alg, parameters, context, feedback):
    # import all rasters with their color tables (and their bands)
    # We need to import all the bands and color tables of the input rasters
    rasters = alg.parameterAsLayerList(parameters, 'map', context)
    for idx, layer in enumerate(rasters):
        layerName = 'map_{}'.format(idx)
        # Add a raster layer
        alg.loadRasterLayer(layerName, layer, context, False, None)

    # Optional raster layer to copy from
    raster = alg.parameterAsString(parameters, 'raster', context)
    if raster:
        alg.loadRasterLayerFromParameter('raster', parameters, context, False, None)

    alg.postInputs(context)


def processCommand(alg, parameters, context, feedback):
    # Handle inline rules
    txtRules = alg.parameterAsString(parameters, 'txtrules', context)
    if txtRules:
        # Creates a temporary txt file
        tempRulesName = getTempFilename(context=context)

        # Inject rules into temporary txt file
        with open(tempRulesName, "w") as tempRules:
            tempRules.write(txtRules)
        alg.removeParameter('txtrules')
        parameters['rules'] = tempRulesName

    if alg.parameterAsEnum(parameters, 'color', context) == 0:
        alg.removeParameter('color')

    alg.processCommand(parameters, context, feedback, True)


def processOutputs(alg, parameters, context, feedback):
    createOpt = alg.parameterAsString(parameters, alg.GRASS_RASTER_FORMAT_OPT, context)
    metaOpt = alg.parameterAsString(parameters, alg.GRASS_RASTER_FORMAT_META, context)

    # Export all rasters with their color tables (and their bands)
    rasters = alg.parameterAsLayerList(parameters, 'map', context)
    outputDir = alg.parameterAsString(parameters, 'output_dir', context)
    for idx, raster in enumerate(rasters):
        rasterName = 'map_{}'.format(idx)
        fileName = os.path.join(outputDir, rasterName)
        outFormat = GrassUtils.getRasterFormatFromFilename(fileName)
        alg.exportRasterLayer(alg.exportedLayers[rasterName], fileName, True,
                              outFormat, createOpt, metaOpt)
