# -*- coding: utf-8 -*-

"""
***************************************************************************
    v_rectify.py
    ------------
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

__author__ = 'Médéric Ribreux'
__date__ = 'March 2016'
__copyright__ = '(C) 2016, Médéric Ribreux'

import os
from grassprovider.Grass7Utils import Grass7Utils
from processing.tools.system import getTempFilename


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    """ Verify if we have the right parameters """
    if (alg.parameterAsString(parameters, 'inline_points', context)
            and alg.parameterAsString(parameters, 'points', context)):
        return False, alg.tr("You need to set either an input control point file or inline control points!")

    return True, None


def processCommand(alg, parameters, context, feedback):
    # handle inline points
    inlinePoints = alg.parameterAsString(parameters, 'inline_points', context)
    if inlinePoints:
        # Creates a temporary txt file
        pointsName = getTempFilename()

        # Inject rules into temporary txt file
        with open(pointsName, "w") as tempPoints:
            tempPoints.write(inlinePoints)
        alg.removeParameter('inline_points')
        parameters['points'] = pointsName

    alg.processCommand(parameters, context, feedback)
