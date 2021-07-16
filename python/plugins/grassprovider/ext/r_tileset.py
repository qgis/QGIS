# -*- coding: utf-8 -*-

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

__author__ = 'Médéric Ribreux'
__date__ = 'October 2017'
__copyright__ = '(C) 2017, Médéric Ribreux'

from grassprovider.Grass7Utils import Grass7Utils


def processOutputs(alg, parameters, context, feedback):
    crs = alg.parameterAsCrs(parameters, 'sourceproj', context)

    wkt_file_name = Grass7Utils.exportCrsWktToFile(crs)
    alg.commands.insert(0, 'g.proj -c wkt="{}"'.format(wkt_file_name))
