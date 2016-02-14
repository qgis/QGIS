# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_support.py
    ------------
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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os


def processCommand(alg):
    # We temporary remove the output
    out = alg.getOutputFromName('output')
    mapParam = alg.getParameterValue('map')
    alg.exportedLayers[out.value] = alg.exportedLayers[mapParam]
    alg.removeOutputFromName('output')
    timestamp = alg.getParameterValue('timestamp')
    if timestamp:
        command = "r.timestamp map={} date='{}'".format(alg.exportedLayers[mapParam],
                                                        timestamp)
        alg.commands.append(command)
        alg.parameters.remove(alg.getParameterFromName('timestamp'))
    alg.processCommand()

    # We re-add the new output
    alg.addOutput(out)


def processOutputs(alg):
    tags = {'title': 'TIFFTAG_DOCUMENTNAME', 'description': 'TIFFTAG_IMAGEDESCRIPTION',
            'creator': 'TIFFTAG_ARTIST', 'timestamp': 'TIFFTAG_DATETIME',
            'source1': 'TIFFTAG_COPYRIGHT', 'units': 'TIFFTAG_RESOLUTIONUNIT',
            'source2': 'GRASS_SOURCE2', 'comments': 'GRASS_HISTORY', 'vdatum': 'GRASS_VDATUM'}
    awk = "awk -F '=' '"
    for support, tag in tags.items():
        awk = '{0} /{1}=".+"/{{ print \"{2}=\"substr($0,{3},length($0) - {3})"," }}'.format(
            awk, support, tag, len(support) + 3)

    # Output results ('from' table and output table)
    out = alg.getOutputValue('output')
    command = u"SDF=$(r.info -e map={} | {}')".format(alg.exportedLayers[out], awk)
    alg.commands.append(command)
    command = u"r.out.gdal --overwrite -c createopt=\"TFW=YES,COMPRESS=LZW\" input={} output=\"{}\" metaopt=\"${{SDF%%,}}\"".format(
        alg.exportedLayers[out], out)
    alg.commands.append(command)
    alg.outputCommands.append(command)
