# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_resamp_filter.py
    ------------------
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


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    """ Verify if we have the right parameters """
    radius = alg.parameterAsString(parameters, 'radius', context)
    x_radius = alg.parameterAsString(parameters, 'x_radius', context)
    y_radius = alg.parameterAsString(parameters, 'y_radius', context)

    if (not radius and not x_radius and not y_radius) or (radius and (x_radius or y_radius)):
        return False, alg.tr("You need to set either radius or x_radius and y_radius!")
    elif (x_radius and not y_radius) or (y_radius and not x_radius):
        return False, alg.tr("You need to set x_radius and y_radius!")
    return True, None
