"""
***************************************************************************
    v_rast_stats.py
    ---------------
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

__author__ = "Médéric Ribreux"
__date__ = "March 2016"
__copyright__ = "(C) 2016, Médéric Ribreux"


def processCommand(alg, parameters, context, feedback):
    # Exclude outputs from commands
    alg.processCommand(parameters, context, feedback, True)


def processOutputs(alg, parameters, context, feedback):
    # We need to add the initial vector layer to outputs:
    fileName = alg.parameterAsOutputLayer(parameters, "output", context)
    grassName = alg.exportedLayers["map"]
    dataType = "auto"
    alg.exportVectorLayer(grassName, fileName, dataType=dataType)
