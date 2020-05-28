# -*- coding: utf-8 -*-

"""
***************************************************************************
    i_evapo_mh.py
    -------------
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
__date__ = 'March 2016'
__copyright__ = '(C) 2016, Médéric Ribreux'


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    if (alg.parameterAsBoolean(parameters, '-h', context)
            and alg.parameterAsLayer(parameters, 'precipitation', context)):
        return False, alg.tr('You can\'t use original Hargreaves flag and precipitation parameter together!')
    if (not alg.parameterAsBoolean(parameters, '-h', context)
            and not alg.parameterAsLayer(parameters, 'precipitation', context)):
        return False, alg.tr('If you don\'t use original Hargreaves flag, you must set the precipitation raster parameter!')
    return True, None
