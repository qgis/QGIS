# -*- coding: utf-8 -*-

"""
***************************************************************************
    i_colors_enhance.py
    -------------------
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

from .i import exportInputRasters


def processCommand(alg, parameters, context, feedback):
    # Temporary remove outputs:
    alg.processCommand(parameters, context, feedback, True)


def processOutputs(alg, parameters, context, feedback):
    # Input rasters are output rasters
    rasterDic = {'red': 'redoutput', 'green': 'greenoutput', 'blue': 'blueoutput'}
    exportInputRasters(alg, parameters, context, rasterDic)
