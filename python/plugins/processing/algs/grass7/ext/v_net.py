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
from qgis.core import QgsProcessingException
from processing.tools.system import getTempFilename


def incorporatePoints(alg, parameters, context, feedback, pointLayerName='points', networkLayerName='input'):
    """
    incorporate points with lines to form a GRASS network
    """

    # Grab the point layer and delete this parameter
    pointLayer = alg.parameterAsVectorLayer(parameters, pointLayerName, context)
    if pointLayer:
        # Create an intermediate GRASS layer which is the combination of network + centers
        intLayer = 'net' + os.path.basename(getTempFilename())

        pointLayer = alg.exportedLayers[pointLayerName]

        # Grab the network layer
        lineLayer = alg.parameterAsVectorLayer(parameters, networkLayerName, context)
        if lineLayer:
            lineLayer = alg.exportedLayers[networkLayerName]
        else:
            raise QgsProcessingException(
                alg.tr('GRASS GIS 7 v.net requires a lines layer!'))

        threshold = alg.parameterAsDouble(parameters, 'threshold', context)

        # Create the v.net connect command for point layer integration
        command = 'v.net -s input={} points={} output={} operation=connect threshold={}'.format(
            lineLayer, pointLayer, intLayer, threshold)
        alg.commands.append(command)

        # Connect the point layer database to the layer 2 of the network
        command = 'v.db.connect -o map={} table={} layer=2'.format(intLayer, pointLayer)
        alg.commands.append(command)

        # remove undesired parameters
        alg.removeParameter(pointLayerName)

        # Use temp layer for input
        alg.exportedLayers[networkLayerName] = intLayer

    # Process the command
    if 'threshold' in parameters:
        alg.removeParameter('threshold')

    alg.processCommand(parameters, context, feedback)


def variableOutput(alg, layers, parameters, context, nocats=True):
    """ Handle variable data output for v.net modules:
    :param layers:
    layers is a dict of outputs:
    { 'outputName': ['srcLayer', 'output_type', output_layer_number, nocats],
    ...
    }
    where:
    - outputName is the name of the output in the description file.
    - srcLayer is the grass name of the layer to export.
    - output_type is the GRASS datatype (point/line/area/etc.).
    - output_layer_number is the GRASS layer number for multiple layers datasets.
    - nocats indicates weither we need to also export without categories items.
    :param parameters:
    :param context:
    :param nocats: do not add categories.
    """
    for outputName, typeList in layers.items():
        if not isinstance(typeList, list):
            continue

        file_name = alg.parameterAsOutputLayer(parameters, outputName, context)

        src_layer = typeList[0]
        output_type = typeList[1]
        output_layer_number = typeList[2]
        no_cats = typeList[3]

        grass_name = '{}{}'.format(src_layer, alg.uniqueSuffix)
        alg.exportVectorLayer(grassName=grass_name,
                              fileName=file_name,
                              layer=output_layer_number,
                              exportnocat=no_cats,
                              dataType=output_type)


def processOutputs(alg, parameters, context, feedback):
    idx = alg.parameterAsInt(parameters, 'operation', context)
    operations = alg.parameterDefinition('operation').options()
    operation = operations[idx]

    if operation == 'nodes':
        outputParameter = {'output': ['output', 'point', 2, True]}
    elif operation == 'connect':
        outputParameter = {'output': ['output', 'line', 1, False]}
    elif operation == 'arcs':
        outputParameter = {'output': ['output', 'line', 1, True]}
    variableOutput(alg, outputParameter, parameters, context)
