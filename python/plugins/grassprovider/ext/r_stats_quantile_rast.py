"""
***************************************************************************
    r_stats_quantile_rast.py
    ------------------------
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
from qgis.core import QgsProcessingParameterString
from grassprovider.grass_utils import GrassUtils


def processCommand(alg, parameters, context, feedback):
    # We create the output sequence according to percentiles number
    quantiles = alg.parameterAsInt(parameters, "quantiles", context) - 1
    outputs = []
    for i in range(0, int(quantiles)):
        outputs.append(f"output_{i}")
    param = QgsProcessingParameterString(
        "output", "virtual output", ",".join(outputs), False, False
    )
    alg.addParameter(param)

    # Removes outputs
    alg.processCommand(parameters, context, feedback, True)


def processOutputs(alg, parameters, context, feedback):
    createOpt = alg.parameterAsString(parameters, alg.GRASS_RASTER_FORMAT_OPT, context)
    metaOpt = alg.parameterAsString(parameters, alg.GRASS_RASTER_FORMAT_META, context)
    outputDir = alg.parameterAsString(parameters, "output_dir", context)
    outputParam = alg.parameterAsString(parameters, "output", context)
    outputs = outputParam.split(",")

    # We need to export each of the output
    for output in outputs:
        fileName = os.path.join(outputDir, output)
        outFormat = GrassUtils.getRasterFormatFromFilename(fileName)
        alg.exportRasterLayer(output, fileName, True, outFormat, createOpt, metaOpt)
