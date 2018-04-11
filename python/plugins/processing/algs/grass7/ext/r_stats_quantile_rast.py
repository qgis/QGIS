# -*- coding: utf-8 -*-

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

__author__ = 'Médéric Ribreux'
__date__ = 'February 2016'
__copyright__ = '(C) 2016, Médéric Ribreux'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import QgsProcessingParameterString
from processing.algs.grass7.Grass7Utils import Grass7Utils
import os


def processCommand(alg, parameters, context, feedback):
    # We create the output sequence according to percentiles number
    quantiles = alg.parameterAsInt(parameters, 'quantiles', context) - 1
    outputs = []
    for i in range(0, int(quantiles)):
        outputs.append('output_{}'.format(i))
    param = QgsProcessingParameterString(
        'output', 'virtual output',
        ','.join(outputs), False, False)
    alg.addParameter(param)

    # Removes outputs
    alg.processCommand(parameters, context, feedback, True)


def processOutputs(alg, parameters, context, feedback):
    createOpt = alg.parameterAsString(parameters, alg.GRASS_RASTER_FORMAT_OPT, context)
    metaOpt = alg.parameterAsString(parameters, alg.GRASS_RASTER_FORMAT_META, context)
    outputDir = alg.parameterAsString(parameters, 'output_dir', context)
    outputParam = alg.parameterAsString(parameters, 'output', context)
    outputs = outputParam.split(',')

    # We need to export each of the output
    for output in outputs:
        fileName = os.path.join(outputDir, output)
        outFormat = Grass7Utils.getRasterFormatFromFilename(fileName)
        alg.exportRasterLayer(output, fileName, True,
                              outFormat, createOpt, metaOpt)
