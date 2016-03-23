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

from processing.core.parameters import getParameterFromString
import os


def processCommand(alg):
    # We create the output sequence according to percentiles number
    base = alg.getParameterValue('base')
    quantiles = alg.getParameterValue('quantiles') - 1
    outputs = []
    for i in range(0, int(quantiles)):
        outputs.append('output_{}'.format(i))

    output = getParameterFromString('ParameterString|output|Output Rasters|None|False|True')
    output.value = ','.join(outputs)
    alg.addParameter(output)

    output_dir = alg.getOutputFromName('output_dir')
    alg.removeOutputFromName('output_dir')

    # Launch the algorithm
    alg.processCommand()

    # We re-add the previous output
    alg.addOutput(output_dir)


def processOutputs(alg):
    # We need to export each of the output
    output_dir = alg.getOutputValue('output_dir')
    outputParam = alg.getParameterFromName('output')
    outputs = outputParam.value.split(',')
    alg.parameters.remove(outputParam)
    for output in outputs:
        command = u"r.out.gdal -c createopt=\"TFW=YES,COMPRESS=LZW\" input={} output=\"{}\" --overwrite".format(
            output,
            os.path.join(output_dir, output + '.tif')
        )
        alg.commands.append(command)
        alg.outputCommands.append(command)
