# -*- coding: utf-8 -*-

"""
***************************************************************************
    v_net.py
    --------
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

This Python module handles pre-treatment operations for v.net.* GRASS7 modules.
Before using a v.net module you often have to incorporate a points layer into
the network vector map.
"""

__author__ = 'Médéric Ribreux'
__date__ = 'December 2015'
__copyright__ = '(C) 2015, Médéric Ribreux'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
from copy import deepcopy


def incorporatePoints(alg, parameters, context, pointLayerName=u'points', networkLayerName=u'input'):
    """
    incorporate points with lines to form a GRASS network
    """
    new_parameters = deepcopy(parameters)

    # Create an intermediate GRASS layer which is the combination of network + centers
    intLayer = alg.getTempFilename()

    # Grab the point layer and delete this parameter (not used by v.net.alloc)
    pointLayer = new_parameters[pointLayerName]
    if pointLayer:
        pointLayer = alg.exportedLayers[pointLayer]
        new_parameters['points'] = pointLayer

    # Grab the network layer and tell to v.net.alloc to use the temp layer instead
    lineLayer = new_parameters[networkLayerName]
    if lineLayer:
        lineLayer = alg.exportedLayers[lineLayer]
        new_parameters[networkLayerName] = lineLayer

    threshold = parameters['threshold']

    # Create the v.net connect command for point layer integration
    command = u"v.net -s input={} points={} out={} op=connect threshold={}".format(
        lineLayer, pointLayer, intLayer, threshold)
    alg.commands.append(command)

    # Connect the point layer database to the layer 2 of the network
    command = u"v.db.connect -o map={} table={} layer=2".format(intLayer, pointLayer)
    alg.commands.append(command)

    alg.processCommand(new_parameters, context)


def variableOutput(alg, layers, parameters, context, nocats=True):
    """ Handle variable data output for v.net modules:
    :param layers:
    layers is like:
    { 'output': ['point', 1], # One output of type point from layer 1
      'output2': ['line', 1], # One output of type line from layer 1
      'output3': ['point', 2] # one output of type point from layer 2
    }
    :param parameters:
    :param context:
    :param nocats: do not add categories.
    """
    for outputName, typeList in list(layers.items()):
        if not isinstance(typeList, list):
            continue

        fileName = alg.parameterAsOutputLayer(parameters, outputName, context)
        grassName = '{}{}'.format(outputName, alg.uniqueSuffix)
        alg.exportVectorLayer(
            grassName, fileName, typeList[0], typeList[1],
            False if typeList[0] == u"line" and nocats else nocats)
