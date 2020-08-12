# -*- coding: utf-8 -*-

"""
***************************************************************************
    i_landsat_acca.py
    -----------------
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

from .i import verifyRasterNum, orderedInput


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    return verifyRasterNum(alg, parameters, context, 'rasters', 5, 5)


def processInputs(alg, parameters, context, feedback):
    orderedInput(alg, parameters, context, 'rasters', 'input',
                 [2, 3, 4, 5, 61])


def processCommand(alg, parameters, context, feedback):
    alg.processCommand(parameters, context, feedback)
