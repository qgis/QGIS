# -*- coding: utf-8 -*-

"""
***************************************************************************
    v_sample.py
    -----------
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


def processInputs(alg, parameters, context, feedback):
    if 'input' in alg.exportedLayers:
        return

    # We need to import the vector with v.in.ogr
    # and we can use r.external for the raster
    alg.loadVectorLayerFromParameter('input', parameters, context, feedback, False)
    alg.loadRasterLayerFromParameter('raster', parameters, context, True)
    alg.postInputs(context)
