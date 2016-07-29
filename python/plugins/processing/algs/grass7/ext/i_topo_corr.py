# -*- coding: utf-8 -*-

"""
***************************************************************************
    i_topo_corr.py
    --------------
    Date                 : April 2016
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
__date__ = 'April 2016'
__copyright__ = '(C) 2016, Médéric Ribreux'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from i import multipleOutputDir, verifyRasterNum
from processing.core.parameters import getParameterFromString


def processCommand(alg):
    # Remove output
    output = alg.getOutputFromName('output')
    alg.removeOutputFromName('output')

    # Create output parameter
    param = getParameterFromString("ParameterString|output|output basename|None|False|False")
    param.value = alg.getTempFilename()
    alg.addParameter(param)

    alg.processCommand()
    # re-add output
    alg.addOutput(output)


def processOutputs(alg):
    param = alg.getParameterFromName('output')
    multipleOutputDir(alg, 'output', param.value)

    # Delete output parameter
    alg.parameters.remove(param)
