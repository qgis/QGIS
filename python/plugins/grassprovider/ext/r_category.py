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

__author__ = "Médéric Ribreux"
__date__ = "February 2016"
__copyright__ = "(C) 2016, Médéric Ribreux"

from processing.tools.system import getTempFilename
from grassprovider.grass_utils import GrassUtils


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    """Verify if we have the right parameters"""
    rules = alg.parameterAsString(parameters, "rules", context)
    txtrules = alg.parameterAsString(parameters, "txtrules", context)
    raster = alg.parameterAsString(parameters, "raster", context)

    if rules and txtrules:
        return False, alg.tr(
            "You need to set either a rules file or write directly the rules!"
        )
    elif (rules and raster) or (txtrules and raster):
        return False, alg.tr(
            "You need to set either rules or a raster from which to copy categories!"
        )

    return True, None


def processInputs(alg, parameters, context, feedback):
    # If there is another raster to copy categories from
    # we need to import it with r.in.gdal rather than r.external
    raster = alg.parameterAsString(parameters, "raster", context)
    if raster:
        alg.loadRasterLayerFromParameter("raster", parameters, context, False, None)
    alg.loadRasterLayerFromParameter("map", parameters, context)
    alg.postInputs(context)


def processCommand(alg, parameters, context, feedback):
    # Handle inline rules
    txtRules = alg.parameterAsString(parameters, "txtrules", context)
    if txtRules:
        # Creates a temporary txt file
        tempRulesName = getTempFilename(context=context)

        # Inject rules into temporary txt file
        with open(tempRulesName, "w") as tempRules:
            tempRules.write(txtRules)
        alg.removeParameter("txtrules")
        parameters["rules"] = tempRulesName

    alg.processCommand(parameters, context, feedback, True)


def processOutputs(alg, parameters, context, feedback):
    # Output results ('map' layer)
    createOpt = alg.parameterAsString(parameters, alg.GRASS_RASTER_FORMAT_OPT, context)
    metaOpt = alg.parameterAsString(parameters, alg.GRASS_RASTER_FORMAT_META, context)

    # We need to export the raster with all its bands and its color table
    fileName = alg.parameterAsOutputLayer(parameters, "output", context)
    outFormat = GrassUtils.getRasterFormatFromFilename(fileName)
    grassName = alg.exportedLayers["map"]
    alg.exportRasterLayer(grassName, fileName, True, outFormat, createOpt, metaOpt)
