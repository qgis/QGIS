"""
***************************************************************************
    i_pansharpen.py
    ---------------
    Date                 : March 2016
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
__date__ = "March 2016"
__copyright__ = "(C) 2016, Médéric Ribreux"

import os
from qgis.core import QgsProcessingParameterString
from processing.tools.system import getTempFilename
from grassprovider.grass_utils import GrassUtils


def processCommand(alg, parameters, context, feedback):
    # Temporary remove outputs and add a virtual output parameter
    outputName = f"output_{os.path.basename(getTempFilename(context=context))}"
    param = QgsProcessingParameterString(
        "output", "virtual output", outputName, False, False
    )
    alg.addParameter(param)
    alg.processCommand(parameters, context, feedback, True)


def processOutputs(alg, parameters, context, feedback):
    outputName = alg.parameterAsString(parameters, "output", context)
    createOpt = alg.parameterAsString(parameters, alg.GRASS_RASTER_FORMAT_OPT, context)
    metaOpt = alg.parameterAsString(parameters, alg.GRASS_RASTER_FORMAT_META, context)
    for channel in ["red", "green", "blue"]:
        fileName = alg.parameterAsOutputLayer(parameters, f"{channel}output", context)
        grassName = f"{outputName}_{channel}"
        outFormat = GrassUtils.getRasterFormatFromFilename(fileName)
        alg.exportRasterLayer(grassName, fileName, True, outFormat, createOpt, metaOpt)
