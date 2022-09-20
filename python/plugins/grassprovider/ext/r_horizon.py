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
    # Inspired from GRASS implementation
    def getNumberDecimals(number):
        """ Return the number of decimals of a number (int or float) """
        if int(number) == number:
            return 0
        return len(str(number).split(".")[-1])

    def doubleToBaseName(number, nDecimals):
        """
        Format filename, according to GRASS implementation, 
        based on provided filename and number of decimals
        """
        if nDecimals == 0:
            return f'{int(number):03}'
        return f'{int(number):03}_{str(number).split(".")[-1].ljust(nDecimals, "0")}'

    # There will be as many outputs as the difference between start and end divided by steps
    start = alg.parameterAsDouble(parameters, 'start', context)
    end = alg.parameterAsDouble(parameters, 'end', context)
    step = alg.parameterAsDouble(parameters, 'step', context)
    direction = alg.parameterAsDouble(parameters, 'direction', context)

    num = start + direction
    nDecimals = getNumberDecimals(step)

    directory = alg.parameterAsString(parameters, 'output', context)
    # Needed if output to a temporary directory
    os.makedirs(directory, exist_ok=True)
    while num < end + direction:
        baseName = doubleToBaseName(num, nDecimals)
        grassName = f'output{alg.uniqueSuffix}_{baseName}'
        fileName = f'{os.path.join(directory, baseName)}.tif'
        alg.exportRasterLayer(grassName, fileName)
        # Weird issue was generating weird num like 0.12000000000000001 or 0.27999999999999997 for step = 0.2
        num = round(num + step, nDecimals)
