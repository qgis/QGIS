# -*- coding: utf-8 -*-

"""
***************************************************************************
    v_net_connectivity.py
    ---------------------
    Date                 : December 2015
    Copyright            : (C) 2015 by Médéric Ribreux
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
__date__ = 'December 2015'
__copyright__ = '(C) 2015, Médéric Ribreux'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from .v_net import incorporatePoints, variableOutput


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    """ Verify if we have the right parameters """
    params = ['where', 'cats']
    values = []
    for param in params:
        for i in range(1, 3):
            values.append(
                alg.parameterAsString(
                    parameters,
                    'set{}_{}'.format(i, param),
                    context
                )
            )

    if (values[0] or values[2]) and (values[1] or values[3]):
        return True, None

    return False, alg.tr('You need to set at least setX_where or setX_cats parameters for each set!')


def processCommand(alg, parameters, context, feedback):
    incorporatePoints(alg, parameters, context, feedback)


def processOutputs(alg, parameters, context, feedback):
    outputParameter = {'output': ['output', 'point', 2, True]}
    variableOutput(alg, outputParameter, parameters, context)
