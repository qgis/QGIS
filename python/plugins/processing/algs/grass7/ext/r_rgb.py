# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_rgb.py
    --------
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


def processInputs(alg, parameters, context, feedback):
    if 'input' in alg.exportedLayers:
        return

    # We need to import all the bands and color tables of the input raster
    alg.loadRasterLayerFromParameter('input', parameters, context, False, None)
    alg.postInputs(context)


def processCommand(alg, parameters, context, feedback):
    # if the input raster is multiband: export each component directly
    rasterInput = alg.exportedLayers['input']
    raster = alg.parameterAsRasterLayer(parameters, 'input', context)
    for color in ['red', 'green', 'blue']:
        alg.exportedLayers[color] = color + alg.uniqueSuffix

    # If the raster is not multiband, really do r.rgb
    if raster.bandCount() == 1:
        alg.commands.append("  r.rgb input={} red={} green={} blue={} --overwrite".format(
            rasterInput,
            alg.exportedLayers['red'],
            alg.exportedLayers['green'],
            alg.exportedLayers['blue']
        ))


def processOutputs(alg, parameters, context, feedback):
    raster = alg.parameterAsRasterLayer(parameters, 'input', context)

    # if the raster was monoband, export from r.rgb
    for color in ['red', 'green', 'blue']:
        fileName = alg.parameterAsOutputLayer(parameters, color, context)
        if raster.bandCount() == 1:
            grassName = '{}{}'.format(color, alg.uniqueSuffix)
        else:
            grassName = '{}{}'.format(alg.exportedLayers['input'], color)
        alg.exportRasterLayer(grassName, fileName, True)
