# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_blend_rgb.py
    --------------
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
from processing.algs.grass7.Grass7Utils import Grass7Utils


def processInputs(alg, parameters, context, feedback):
    if 'first' and 'second' in alg.exportedLayers:
        return

    # Use v.in.ogr
    for name in ['first', 'second']:
        alg.loadRasterLayerFromParameter(name, parameters, context, False, None)
    alg.postInputs(context)


def processCommand(alg, parameters, context, feedback):
    # We need to remove all outputs
    alg.processCommand(parameters, context, feedback, True)


def processOutputs(alg, parameters, context, feedback):
    createOpt = alg.parameterAsString(parameters, alg.GRASS_RASTER_FORMAT_OPT, context)
    metaOpt = alg.parameterAsString(parameters, alg.GRASS_RASTER_FORMAT_META, context)

    # Export each color raster
    colors = ['red', 'green', 'blue']
    for color in colors:
        fileName = os.path.normpath(
            alg.parameterAsOutputLayer(parameters, 'output_{}'.format(color), context))
        outFormat = Grass7Utils.getRasterFormatFromFilename(fileName)
        alg.exportRasterLayer('blended.{}'.format(color[0]),
                              fileName, True, outFormat, createOpt, metaOpt)
