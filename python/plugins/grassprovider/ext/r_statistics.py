# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_statistics.py
    ---------------
    Date                 : September 2017
    Copyright            : (C) 2017 by Médéric Ribreux
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
__date__ = 'September 2017'
__copyright__ = '(C) 2017, Médéric Ribreux'

from qgis.core import QgsProcessingParameterString
from processing.algs.grass7.Grass7Utils import Grass7Utils


def processCommand(alg, parameters, context, feedback):
    # We had a new "output" parameter
    out = 'output{}'.format(alg.uniqueSuffix)
    p = QgsProcessingParameterString('~output', None, out, False, False)
    alg.addParameter(p)

    # We need to remove all outputs
    alg.processCommand(parameters, context, feedback, True)

    # Then we add a new command for treating results
    calcExpression = 'correctedoutput{}=@{}'.format(
        alg.uniqueSuffix, out)
    command = 'r.mapcalc expression="{}"'.format(calcExpression)
    alg.commands.append(command)


def processOutputs(alg, parameters, context, feedback):
    createOpt = alg.parameterAsString(parameters, alg.GRASS_RASTER_FORMAT_OPT, context)
    metaOpt = alg.parameterAsString(parameters, alg.GRASS_RASTER_FORMAT_META, context)

    # Export the results from correctedoutput
    grassName = 'correctedoutput{}'.format(alg.uniqueSuffix)
    fileName = alg.parameterAsOutputLayer(
        parameters, 'routput', context)
    outFormat = Grass7Utils.getRasterFormatFromFilename(fileName)
    alg.exportRasterLayer(grassName, fileName, True,
                          outFormat, createOpt, metaOpt)
