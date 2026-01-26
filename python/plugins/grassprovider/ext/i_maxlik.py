"""
***************************************************************************
    i_maxlik.py
    -----------
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

from .i import regroupRasters, importSigFile


def processCommand(alg, parameters, context, feedback):
    group, subgroup = regroupRasters(
        alg, parameters, context, "input", "group", "subgroup"
    )

    # import signature
    signatureFile = alg.parameterAsString(parameters, "signaturefile", context)
    shortSigFile = importSigFile(alg, group, subgroup, signatureFile)
    parameters["signaturefile"] = shortSigFile

    # Handle other parameters
    alg.processCommand(parameters, context, feedback)
