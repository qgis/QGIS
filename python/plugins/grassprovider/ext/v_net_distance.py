"""
***************************************************************************
    v_net_distance.py
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

import os
from .v_net import variableOutput
from processing.tools.system import getTempFilename
from qgis.core import QgsProcessingParameterString


def processCommand(alg, parameters, context, feedback):
    """Handle data preparation for v.net.distance:
    * Integrate point layers into network vector map.
    * Make v.net.distance use those layers.
    * Delete the threshold parameter.
    * If where statement, connect to the db
    """
    # Grab the point layer and delete this parameter
    lineLayer = alg.exportedLayers["input"]
    fromLayer = alg.exportedLayers["flayer"]
    toLayer = alg.exportedLayers["tlayer"]
    intLayer = "bufnet" + os.path.basename(getTempFilename(context=context))
    netLayer = "net" + os.path.basename(getTempFilename(context=context))
    threshold = alg.parameterAsDouble(parameters, "threshold", context)

    # Create the v.net connect command for from_layer integration
    command = "v.net -s input={} points={} output={} operation=connect threshold={} arc_layer=1 node_layer=2".format(
        lineLayer, fromLayer, intLayer, threshold
    )
    alg.commands.append(command)

    # Do it again with to_layer
    command = "v.net -s input={} points={} output={} operation=connect threshold={} arc_layer=1 node_layer=3".format(
        intLayer, toLayer, netLayer, threshold
    )
    alg.commands.append(command)

    # Connect the point layer database to the layer 2 of the network
    command = f"v.db.connect -o map={netLayer} table={fromLayer} layer=2"
    alg.commands.append(command)

    command = f"v.db.connect -o map={netLayer} table={toLayer} layer=3"
    alg.commands.append(command)

    # remove undesired parameters
    alg.removeParameter("flayer")
    alg.removeParameter("tlayer")
    alg.removeParameter("threshold")
    alg.exportedLayers["input"] = netLayer

    # Add the two new parameters
    fLayer = QgsProcessingParameterString("from_layer", None, 2, False, False)
    alg.addParameter(fLayer)
    tLayer = QgsProcessingParameterString("to_layer", None, 3, False, False)
    alg.addParameter(tLayer)
    alg.processCommand(parameters, context, feedback)


def processOutputs(alg, parameters, context, feedback):
    outputParameter = {"output": ["output", "line", 1, True]}
    variableOutput(alg, outputParameter, parameters, context)
