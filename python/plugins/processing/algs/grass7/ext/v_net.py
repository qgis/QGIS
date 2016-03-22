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


def incorporatePoints(alg, pointLayerName=u'points', networkLayerName=u'input'):
    """
    incorporate points with lines to form a GRASS network
    """
    paramsToDelete = []

    # Create an intermediate GRASS layer which is the combination of network + centers
    intLayer = alg.getTempFilename()

    # Grab the point layer and delete this parameter (not used by v.net.alloc)
    pointLayer = alg.getParameterValue(pointLayerName)
    if pointLayer:
        pointLayer = alg.exportedLayers[pointLayer]
        paramsToDelete.append(alg.getParameterFromName(u'points'))

    # Grab the network layer and tell to v.net.alloc to use the temp layer instead
    lineLayer = alg.getParameterValue(networkLayerName)
    if lineLayer:
        lineLayer = alg.exportedLayers[lineLayer]
        alg.setParameterValue(networkLayerName, intLayer)

    threshold = alg.getParameterValue(u'threshold')
    paramsToDelete.append(alg.getParameterFromName(u'threshold'))

    # Create the v.net connect command for point layer integration
    command = u"v.net -s input={} points={} out={} op=connect threshold={}".format(
        lineLayer, pointLayer, intLayer, threshold)
    alg.commands.append(command)

    # Connect the point layer database to the layer 2 of the network
    command = u"v.db.connect -o map={} table={} layer=2".format(intLayer, pointLayer)
    alg.commands.append(command)

    # Delete some unnecessary parameters
    for param in paramsToDelete:
        alg.parameters.remove(param)

    alg.processCommand()

    # Bring back the parameters:
    for param in paramsToDelete:
        alg.parameters.append(param)


def variableOutput(alg, params, nocats=True):
    """ Handle variable data output for v.net modules:
    params is like:
    { u"output": [u"point", 1], # One output of type point from layer 1
      u"output2": [u"line", 1], # One output of type line from layer 1
      u"output3: [u"point", 2] # one output of type point from layer 2
    }

    """

    # Build the v.out.ogr commands
    for outputName, typeList in params.iteritems():
        if not isinstance(typeList, list):
            continue

        out = alg.getOutputValue(outputName)
        command = u"v.out.ogr {} type={} layer={} -s -e input={} output=\"{}\" format=ESRI_Shapefile output_layer={}".format(
            u"" if typeList[0] == u"line" and nocats else u"-c",
            typeList[0],
            typeList[1],
            alg.exportedLayers[out],
            os.path.dirname(out),
            os.path.basename(out)[:-4]
        )
        alg.commands.append(command)
        alg.outputCommands.append(command)
