# -*- coding: utf-8 -*-

"""
***************************************************************************
    v_distance.py
    -------------
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

from qgis.core import QgsProcessingParameterDefinition


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    """ Verify if we have the right parameters """
    # Verifiy that we have the good number of columns
    uploads = alg.parameterAsEnums(parameters, 'upload', context)
    columns = alg.parameterAsFields(parameters, 'column', context)
    if len(columns) != len(uploads):
        return False, alg.tr("The number of columns and the number of upload parameters should be equal!")

    return True, None


def processCommand(alg, parameters, context, feedback):
    # We need to disable only from_output parameter
    fromOutput = alg.parameterDefinition('from_output')
    fromOutput.setFlags(fromOutput.flags() | QgsProcessingParameterDefinition.FlagHidden)
    alg.processCommand(parameters, context, feedback, False)
    fromOutput.setFlags(fromOutput.flags() | QgsProcessingParameterDefinition.FlagHidden)


def processOutputs(alg, parameters, context, feedback):
    alg.vectorOutputType(parameters, context)
    alg.exportVectorLayerFromParameter('output', parameters, context)
    # for from_output, we export the initial layer
    fileName = alg.parameterAsOutputLayer(parameters, 'from_output', context)
    grassName = alg.exportedLayers['from']
    alg.exportVectorLayer(grassName, fileName)
