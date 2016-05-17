# -*- coding: utf-8 -*-

"""
***************************************************************************
    v_lrs_where.py
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

__author__ = 'Médéric Ribreux'
__date__ = 'March 2016'
__copyright__ = '(C) 2016, Médéric Ribreux'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


def processInputs(alg):
    # We need to import the rstable
    rstable = alg.getParameterValue('rstable')
    if rstable in alg.exportedLayers.keys():
        return
    alg.exportedLayers[rstable] = alg.getTempFilename()
    command = 'db.in.ogr input=\"{}\" output={} --overwrite'.format(
        rstable,
        alg.exportedLayers[rstable]
    )
    alg.commands.append(command)
    alg.processInputs()


def processCommand(alg):
    command = 'v.lrs.where lines={} points={} rstable={} thresh={} > {} --overwrite'.format(
        alg.exportedLayers[alg.getParameterValue('lines')],
        alg.exportedLayers[alg.getParameterValue('points')],
        alg.exportedLayers[alg.getParameterValue('rstable')],
        alg.getParameterValue('thresh'),
        alg.getOutputValue('output')
    )
    alg.commands.append(command)
