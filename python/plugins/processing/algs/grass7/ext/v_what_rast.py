# -*- coding: utf-8 -*-

"""
***************************************************************************
    v_what_rast.py
    ---------------------
    Date                 : January 2016
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

This Python module handles output for v.what.rast.* GRASS7 modules.
"""

__author__ = 'Médéric Ribreux'
__date__ = 'January 2016'
__copyright__ = '(C) 2016, Médéric Ribreux'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os


def removeOutput(alg):
    """Remove the output fo v.what.rast"""
    # We temporary remove the output 'sequence'
    output = alg.getOutputFromName(u'output')
    alg.removeOutputFromName(u'output')

    # Launch the algorithm
    alg.processCommand()

    # We re-add the previous output
    alg.addOutput(output)


def outputInput(alg):
    """Make output the initial point/polygon layer"""
    output = alg.getOutputValue(u'output')
    command = u"v.out.ogr -c type=auto -s -e input={} output=\"{}\" format=ESRI_Shapefile output_layer={}".format(
        alg.exportedLayers[alg.getParameterValue(u'map')],
        os.path.dirname(output),
        os.path.basename(output)[:-4]
    )
    alg.commands.append(command)
    alg.outputCommands.append(command)
