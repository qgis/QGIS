# -*- coding: utf-8 -*-

"""
***************************************************************************
    v_build_polylines.py
    --------------------
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


import os


def processOutputs(alg):
    out = alg.getOutputValue('output')
    command = u"v.out.ogr -c -s -e input={} output=\"{}\" format=ESRI_Shapefile output_layer={}".format(
        alg.exportedLayers[out],
        os.path.dirname(out),
        os.path.splitext(os.path.basename(out))[0]
    )
    alg.commands.append(command)
    alg.outputCommands.append(command)
