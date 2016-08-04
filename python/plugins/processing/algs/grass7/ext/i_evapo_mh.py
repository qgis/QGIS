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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from i import verifyRasterNum


def checkParameterValuesBeforeExecuting(alg):
    if alg.getParameterValue('-h') and alg.getParameterValue('precipitation'):
        return alg.tr('You can\'t use original Hargreaves flag and precipitation parameter together!')
    if not alg.getParameterValue('-h') and not alg.getParameterValue('precipitation'):
        return alg.tr('If you don\'t use original Hargreaves flag, you must set the precipitation raster parameter!')
    return None
