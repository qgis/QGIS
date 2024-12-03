"""
***************************************************************************
    r_li_renyi.py
    --------------
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

__author__ = "Médéric Ribreux"
__date__ = "February 2016"
__copyright__ = "(C) 2016, Médéric Ribreux"

from .r_li import checkMovingWindow, configFile


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    return checkMovingWindow(alg, parameters, context)


def processCommand(alg, parameters, context, feedback):
    configFile(alg, parameters, context, feedback)
