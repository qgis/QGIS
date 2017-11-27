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
from builtins import str

__author__ = 'Médéric Ribreux'
__date__ = 'February 2016'
__copyright__ = '(C) 2016, Médéric Ribreux'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    """ Verify if we have the right parameters """
    if (alg.parameterAsString(parameters, 'setnull', context)
            or alg.parameterAsString(parameters, 'null', context)):
        return None

    return alg.tr("You need to set at least 'setnull' or 'null' parameters for this algorithm!")


def processInputs(alg, parameters, context):
    """Prepare the GRASS import commands"""
    if 'map' in alg.exportedLayers:
        return

    # We need to import all the bands and color tables of the input raster
    alg.loadRasterLayerFromParameter('map', parameters, context, False)
    alg.postInputs()


def processCommand(alg, parameters, context):
    # We temporary remove the output 'sequence'
    alg.processCommand(parameters, context, True)


def processOutputs(alg, parameters, context):
    fileName = alg.parameterAsOutputLayer(parameters, 'output', context)
    grassName = '{}{}'.format('map', alg.uniqueSuffix)
    alg.exportRasterLayer(grassName, fileName, False)
