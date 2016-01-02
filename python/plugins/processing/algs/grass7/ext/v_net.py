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
Before using v.net you often have to incorporate a points layer into the network
 vector map.

You also have different output depending on some parameters.

This file also contains dedicated ext functions for v.net module...
"""

__author__ = 'Médéric Ribreux'
__date__ = 'December 2015'
__copyright__ = '(C) 2015, Médéric Ribreux'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
from processing.core.parameters import getParameterFromString, ParameterVector, ParameterNumber, ParameterBoolean, ParameterString


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
    parameters:
    { u"output": { u"param" : { 0: u"line", 1: u"point" } }, # parametized output
      u"output3: u"line" # use this type for this output
    }
    """

    # Build the v.out.ogr commands
    for outName in params.keys():
        param = params[outName]
        # There is a dict parameter:
        if isinstance(param, dict):
            paramName = param.keys()[0]
            paramValue = alg.getParameterValue(paramName)
            paramDict = param[paramName]
            geomType = paramDict[paramValue]
        elif isinstance(param, unicode):
            geomType = param
        else:
            continue

        out = alg.getOutputValue(outName)
        command = u"v.out.ogr {} type={} -s -e input={} output=\"{}\" format=ESRI_Shapefile output_layer={}".format(
            u"" if geomType == u"line" and nocats else u"-c",
            geomType,
            alg.exportedLayers[out],
            os.path.dirname(out),
            os.path.basename(out)[:-4]
        )
        alg.commands.append(command)
        alg.outputCommands.append(command)


# v.net dedicated functions
def checkParameterValuesBeforeExecuting(alg):
    """ Verify if we have the right parameters """
    operation = alg.getParameterValue(u'operation')
    pointLayer = alg.getParameterValue(u'points')
    threshold = alg.getParameterValue(u'threshold')
    fileName = alg.getParameterValue(u'file')
    if operation == 1:
        if not (pointLayer and threshold):
            return alg.tr("You need to set an input points layer and a threshold for operation 'connect' !")
    elif operation == 2:
        if not (fileName and pointLayer):
            return alg.tr("You need to set an input points layer and a file for operation 'arcs' !")

    return None


def processCommand(alg):
    """ Handle data preparation for v.net:
    * Integrate point layers into network vector map.
    * Make v.net.distance use those layers.
    * Delete the threshold parameter.
    """
    paramsToDelete = []

    # If we use the node operation, no need for threshold,
    operation = alg.getParameterValue(u'operation')
    if operation == 0:
        paramsToDelete.append(alg.getParameterFromName(u'threshold'))
        paramsToDelete.append(alg.getParameterFromName(u'file'))
    elif operation == 2:
        paramsToDelete.append(alg.getParameterFromName(u'threshold'))
    elif operation in [3, 4]:
        paramsToDelete.append(alg.getParameterFromName(u'threshold'))
        paramsToDelete.append(alg.getParameterFromName(u'points'))
        paramsToDelete.append(alg.getParameterFromName(u'file'))

    # Grab the network layer and tell to v.net.alloc to use the temp layer instead
    lineLayer = alg.getParameterValue(u'input')
    if lineLayer:
        lineLayer = alg.exportedLayers[lineLayer]

    # Delete some unnecessary parameters
    for param in paramsToDelete:
        alg.parameters.remove(param)

    alg.processCommand()

    # Bring back the parameters:
    for param in paramsToDelete:
        alg.parameters.append(param)


def processOutputs(alg):
    """ Handle data output for v.net:
    * use v.out.ogr with type=line on node operation.
    * use v.out.ogr with type=point on articulation method.
    """

    # Find the method used
    operation = alg.getParameterValue(u'operation')

    # Grab the name of the output
    out = alg.getOutputValue(u'output')

    # Build the v.out.ogr command
    command = u"v.out.ogr -c type={} layer={} -e input={} output=\"{}\" format=ESRI_Shapefile output_layer={}".format(
        u"point" if operation == 0 else "line",
        u"2" if operation == 0 else u"1",
        alg.exportedLayers[out],
        os.path.dirname(out),
        os.path.basename(out)[:-4]
    )
    alg.commands.append(command)
