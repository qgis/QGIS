# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_drain.py
    ----------
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


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    """ Verify if we have the right parameters """
    # start_coordinates and start_points are mutually exclusives
    if (alg.parameterAsString(parameters, 'start_coordinates', context)
            and alg.parameterAsVectorLayer(parameters, 'start_points', context)):
        return False, alg.tr("You need to set either start coordinates OR a start points vector layer!")

    # You need to set at least one parameter
    if (not alg.parameterAsString(parameters, 'start_coordinates', context)
            and not alg.parameterAsVectorLayer(parameters, 'start_points', context)):
        return False, alg.tr("You need to set either start coordinates OR a start points vector layer!")

    paramscore = [f for f in ['-c', '-a', '-n']
                  if alg.parameterAsBoolean(parameters, f, context)]
    if len(paramscore) > 1:
        return False, alg.tr("-c, -a, -n parameters are mutually exclusive!")
    return True, None
