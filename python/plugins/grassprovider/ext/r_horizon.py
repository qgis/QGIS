# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_horizon.py
    ------------
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

import os


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    """ Verify if we have the right parameters """
    start = alg.parameterAsDouble(parameters, 'start', context)
    end = alg.parameterAsDouble(parameters, 'end', context)
    step = alg.parameterAsDouble(parameters, 'step', context)

    if start >= end:
        return False, alg.tr("The start position must be inferior to the end position!")
    if step == 0.0:
        return False, alg.tr("The step must be greater than zero!")
    return True, None


def processOutputs(alg, parameters, context, feedback):
    # There will be as many outputs as the difference between start and end divided by steps
    start = alg.parameterAsDouble(parameters, 'start', context)
    end = alg.parameterAsDouble(parameters, 'end', context)
    step = alg.parameterAsDouble(parameters, 'step', context)
    num = start
    directory = alg.parameterAsString(parameters, 'output', context)
    while num < end:
        grassName = '{}_{}'.format('output{}'.format(alg.uniqueSuffix), int(num))
        fileName = '{}.tif'.format(os.path.join(directory, '{0:0>3}'.format(int(num))))
        alg.exportRasterLayer(grassName, fileName)
        num += step
