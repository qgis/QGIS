"""
***************************************************************************
    i_gensigset.py
    --------------
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

__author__ = "Médéric Ribreux"
__date__ = "March 2016"
__copyright__ = "(C) 2016, Médéric Ribreux"

import os
from .i import regroupRasters, exportSigFile


def processCommand(alg, parameters, context, feedback):
    # We need to extract the basename of the signature file
    signatureFile = alg.parameterAsString(parameters, "signaturefile", context)
    shortSigFile = os.path.basename(signatureFile)
    parameters["signaturefile"] = shortSigFile

    # Regroup rasters
    group, subgroup = regroupRasters(
        alg, parameters, context, "input", "group", "subgroup"
    )
    alg.processCommand(parameters, context, feedback)

    # Re-add signature files
    parameters["signaturefile"] = signatureFile
    alg.fileOutputs["signaturefile"] = signatureFile

    # Export signature file
    exportSigFile(alg, group, subgroup, signatureFile, "sigset")
