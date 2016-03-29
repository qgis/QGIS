# -*- coding: utf-8 -*-

"""
***************************************************************************
    v_net_salesman.py
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

__author__ = 'Médéric Ribreux'
__date__ = 'December 2015'
__copyright__ = '(C) 2015, Médéric Ribreux'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from processing.core.parameters import getParameterFromString
from .v_net import incorporatePoints


def processCommand(alg):
    # We temporary remove the output 'sequence'
    sequence = alg.getOutputFromName(u'sequence')
    sequenceFile = alg.getOutputValue(u'sequence')
    alg.exportedLayers[sequence.value] = sequence.name + alg.uniqueSufix
    alg.removeOutputFromName(u'sequence')

    # We create a new parameter with the same name
    param = getParameterFromString(u"ParameterString|sequence|sequence|None|False|False")
    param.setValue(sequenceFile)
    alg.addParameter(param)

    # Let's do the incorporation and command generation
    incorporatePoints(alg)

    # then we delete the input parameter and add the old output
    alg.parameters.remove(param)
    alg.addOutput(sequence)
