# -*- coding: utf-8 -*-

"""
***************************************************************************
    i_cca.py
    --------
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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from .i import verifyRasterNum, regroupRasters


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    return verifyRasterNum(alg, parameters, context, 'input', 2, 8)


def processCommand(alg, parameters, context):
    # Regroup rasters
    regroupRasters(alg, parameters, 'input', 'group', 'subgroup', {'signature': 'sig'})

    # Handle other parameters
    alg.processCommand(alg, parameters, context)
