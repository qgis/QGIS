# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_statistics.py
    ---------------
    Date                 : September 2017
    Copyright            : (C) 2017 by Médéric Ribreux
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
__date__ = 'September 2017'
__copyright__ = '(C) 2017, Médéric Ribreux'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import QgsProcessingParameterString


def processCommand(alg, parameters, context):
    # We had a new "output" parameter
    out = 'output{}'.format(alg.uniqueSuffix)
    p = QgsProcessingParameterString('output', None, out, False, False)
    alg.addParameter(p)

    # We need to remove all outputs
    alg.processCommand(parameters, context, True)

    # Then we add a new command for treating results
    calcExpression = 'correctedoutput{}=@{}'.format(
        alg.uniqueSuffix, out)
    command = 'r.mapcalc expression="{}"'.format(calcExpression)
    alg.commands.append(command)


def processOutputs(alg, parameters, context):
    # Export the results from correctedoutput
    grassName = 'correctedoutput{}'.format(alg.uniqueSuffix)
    fileName = alg.parameterAsOutputLayer(
        parameters, 'routput', context)
    alg.exportRasterLayer(grassName, fileName)
