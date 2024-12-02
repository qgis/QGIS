"""
***************************************************************************
    r_series_interp.py
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

__author__ = "Médéric Ribreux"
__date__ = "February 2016"
__copyright__ = "(C) 2016, Médéric Ribreux"

import os
from grassprovider.grass_utils import GrassUtils


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    """Verify if we have the right parameters"""
    datapos = alg.parameterAsDouble(parameters, "datapos", context)
    infile = alg.parameterAsString(parameters, "infile", context)
    output = alg.parameterAsString(parameters, "output", context)
    outfile = alg.parameterAsString(parameters, "outfile", context)

    if datapos and infile:
        return False, alg.tr(
            "You need to set either inline data positions or an input data positions file!"
        )
    if output and outfile:
        return False, alg.tr(
            "You need to set either sampling data positions or an output sampling data positions file!"
        )
    if not (datapos or infile or output or outfile):
        return False, alg.tr(
            "You need to set input and output data positions parameters!"
        )
    return True, None


def processCommand(alg, parameters, context, feedback):
    # We temporary remove the output directory
    alg.processCommand(parameters, context, feedback, True)


def processOutputs(alg, parameters, context, feedback):
    # We take all the outputs and we export them to the output directory
    outputDir = alg.parameterAsString(parameters, "output_dir", context)
    output = alg.parameterAsString(parameters, "output", context)
    outfile = alg.parameterAsString(parameters, "outfile", context)
    outs = []
    if output:
        outs = output.split(",")
    elif outfile:
        # Handle file manually to find the name of the layers
        with open(outfile) as f:
            for line in f:
                if "|" in line:
                    outs.append(line.split("|")[0])

    createOpt = alg.parameterAsString(parameters, alg.GRASS_RASTER_FORMAT_OPT, context)
    metaOpt = alg.parameterAsString(parameters, alg.GRASS_RASTER_FORMAT_META, context)

    for out in outs:
        # We need to export the raster with all its bands and its color table
        fileName = os.path.join(outputDir, out)
        outFormat = GrassUtils.getRasterFormatFromFilename(fileName)
        alg.exportRasterLayer(out, fileName, True, outFormat, createOpt, metaOpt)
