"""
***************************************************************************
    r_proj.py
    ---------
    Date                 : October 2017
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

__author__ = "Médéric Ribreux"
__date__ = "October 2017"
__copyright__ = "(C) 2017, Médéric Ribreux"

from grassprovider.grass_utils import GrassUtils


def processOutputs(alg, parameters, context, feedback):
    crs = alg.parameterAsCrs(parameters, "sourceproj", context)

    wkt_file_name = GrassUtils.exportCrsWktToFile(crs, context)
    alg.commands.insert(0, f'g.proj -c wkt="{wkt_file_name}"')
