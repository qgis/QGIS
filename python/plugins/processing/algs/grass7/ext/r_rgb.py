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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


def processInputs(alg, parameters, context):
    # We need to import all the bands and color tables of the input raster
    if 'input' in alg.exportedLayers:
        return
    raster = alg.parameterAsRasterLayer(parameters, 'input', context)
    alg.setSessionProjectionFromLayer(raster)
    alg.prepareInputs()
    
def processCommand(alg, parameters, context):
    # We need to introduce something clever:
    # if the input raster is multiband: export each component directly
    raster = alg.exportedLayers[alg.getParameterValue('input')]
    for color in ['red', 'green', 'blue']:
        alg.exportedLayers[alg.getOutputValue(color)] = color + alg.uniqueSuffix

    commands = ["if [ $(g.list type=rast pattern='{}.*' | wc -l) -eq \"0\" ]; then".format(raster)]
    commands.append("  r.rgb input={} red={} green={} blue={} --overwrite".format(
        raster,
        alg.exportedLayers[alg.getOutputValue('red')],
        alg.exportedLayers[alg.getOutputValue('green')],
        alg.exportedLayers[alg.getOutputValue('blue')]
    ))
    commands.append("fi")
    alg.commands.extend(commands)


def processOutputs(alg, parameters, context):
    raster = alg.exportedLayers[alg.getParameterValue('input')]
    commands = ["if [ $(g.list type=rast pattern='{}.*' | wc -l) -eq \"0\" ]; then".format(raster)]
    for color in ['red', 'green', 'blue']:
        commands.append("  r.out.gdal -t input={} output={} createopt=\"TFW=YES,COMPRESS=LZW\" --overwrite".format(
            alg.exportedLayers[alg.getOutputValue(color)],
            alg.getOutputValue(color)
        ))
    commands.append("else")
    for color in ['red', 'green', 'blue']:
        commands.append("  r.out.gdal -t input={} output={} createopt=\"TFW=YES,COMPRESS=LZW\" --overwrite".format(
            '{}.{}'.format(raster, color),
            alg.getOutputValue(color)
        ))
    commands.append("fi")
    alg.commands.extend(commands)
