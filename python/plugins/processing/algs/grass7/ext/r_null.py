# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_null.py
    ---------------------
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

__author__ = 'Médéric Ribreux'
__date__ = 'February 2016'
__copyright__ = '(C) 2016, Médéric Ribreux'


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    """ Verify if we have the right parameters """
    if (alg.parameterAsString(parameters, 'setnull', context)
            or alg.parameterAsString(parameters, 'null', context)):
        return True, None

    return False, alg.tr("You need to set at least 'setnull' or 'null' parameters for this algorithm!")


def processInputs(alg, parameters, context, feedback):
    """Prepare the GRASS import commands"""
    if 'map' in alg.exportedLayers:
        return

    # We need to import without r.external
    alg.loadRasterLayerFromParameter('map', parameters, context, False)
    alg.postInputs(context)


def processCommand(alg, parameters, context, feedback):
    # We temporary remove the output 'sequence'
    alg.processCommand(parameters, context, feedback, True)


def processOutputs(alg, parameters, context, feedback):
    fileName = alg.parameterAsOutputLayer(parameters, 'output', context)
    grassName = alg.exportedLayers['map']
    alg.exportRasterLayer(grassName, fileName, False)
