"""
***************************************************************************
    v_voronoi.py
    ------------
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


def processInputs(alg, parameters, context, feedback):
    if "input" in alg.exportedLayers:
        return

    # We need to use v.in.ogr instead of v.external
    alg.loadVectorLayerFromParameter("input", parameters, context, feedback, False)
    alg.processInputs(parameters, context, feedback)


def processOutputs(alg, parameters, context, feedback):
    fileName = alg.parameterAsOutputLayer(parameters, "output", context)
    grassName = "{}{}".format("output", alg.uniqueSuffix)
    dataType = "auto"
    # if we export a graph, output type will be a line
    if alg.parameterAsBoolean(parameters, "-l", context):
        dataType = "line"

    alg.exportVectorLayer(grassName, fileName, dataType=dataType)
