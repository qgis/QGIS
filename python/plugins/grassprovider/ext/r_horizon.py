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
import math


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
        based on provided number and number of decimals
        """
        number += 0.0001
        # adapted from GRASS https://github.com/OSGeo/grass/blob/6253da1bd6ce48d23419e99e8b503edf46178490/lib/gis/basename.c#L97-L101
        if nDecimals == 0:
            return f'{int(number):03}'
        int_part = int(number)
        dec_part = int((number - int_part) * pow(10, nDecimals))
        return f'{int_part:03}_{str(dec_part).rjust(nDecimals, "0")}'

    # There will be as many outputs as the difference between start and end divided by steps
    start = alg.parameterAsDouble(parameters, 'start', context)
    end = alg.parameterAsDouble(parameters, 'end', context)
    step = alg.parameterAsDouble(parameters, 'step', context)
    direction = alg.parameterAsDouble(parameters, 'direction', context)

    first_rad = math.radians(start + direction)
    nDecimals = getNumberDecimals(step)
    dfr_rad = math.radians(step)
    arrayNumInt = int((end - start) / abs(step))

    directory = alg.parameterAsString(parameters, 'output', context)
    # Needed if output to a temporary directory
    os.makedirs(directory, exist_ok=True)
    for k in range(arrayNumInt):
        angle_deg = math.degrees(first_rad + dfr_rad * k)
        baseName = doubleToBaseName(angle_deg, nDecimals)
        grassName = f'output{alg.uniqueSuffix}_{baseName}'
        fileName = f'{os.path.join(directory, baseName)}.tif'
        alg.exportRasterLayer(grassName, fileName)
