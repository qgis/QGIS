"""
***************************************************************************
    v_net_bridge.py
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

__author__ = "Médéric Ribreux"
__date__ = "December 2015"
__copyright__ = "(C) 2015, Médéric Ribreux"

from .v_net import incorporatePoints, variableOutput


def processCommand(alg, parameters, context, feedback):
    incorporatePoints(alg, parameters, context, feedback)


def processOutputs(alg, parameters, context, feedback):
    idx = alg.parameterAsInt(parameters, "method", context)
    operations = alg.parameterDefinition("method").options()
    operation = operations[idx]

    if operation == "articulation":
        outputParameter = {"output": ["output", "point", 2, True]}
    elif operation == "bridge":
        outputParameter = {"output": ["output", "line", 1, False]}
    variableOutput(alg, outputParameter, parameters, context)
