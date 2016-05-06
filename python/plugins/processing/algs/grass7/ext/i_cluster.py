# -*- coding: utf-8 -*-

"""
***************************************************************************
    i_cluster.py
    ------------
    Date                 : March 2016
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
__date__ = 'March 2016'
__copyright__ = '(C) 2016, Médéric Ribreux'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from i import regroupRasters, file2Output, moveFile, verifyRasterNum
from os import path
from ..Grass7Utils import Grass7Utils


def checkParameterValuesBeforeExecuting(alg):
    return verifyRasterNum(alg, 'input', 2)


def processCommand(alg):
    # We need to extract the basename of the signature file
    signatureFile = alg.getOutputFromName('signaturefile')
    origSigFile = signatureFile.value
    shortSigFile = path.basename(origSigFile)
    alg.setOutputValue('signaturefile', shortSigFile)

    # Transform output files in string parameters
    signatureFile = file2Output(alg, 'signaturefile')
    reportFile = file2Output(alg, 'reportfile')

    # Regroup rasters
    group, subgroup = regroupRasters(alg, 'input', 'group', 'subgroup')

    # Re-add signature files
    alg.addOutput(signatureFile)
    alg.addOutput(reportFile)

    # Find Grass directory
    interSig = path.join(Grass7Utils.grassMapsetFolder(), 'PERMANENT', 'group', group, 'subgroup', subgroup, 'sig', shortSigFile)
    moveFile(alg, interSig, origSigFile)
    alg.setOutputValue('signaturefile', origSigFile)
