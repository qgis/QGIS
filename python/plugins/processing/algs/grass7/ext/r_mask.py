# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_mask.py
    ---------
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


def processCommand(alg):
    # Remove output and input
    clipped = alg.getParameterFromName('input')
    out = alg.getOutputFromName('output')
    alg.exportedLayers[out.value] = alg.exportedLayers[clipped.value]
    alg.removeOutputFromName('output')
    alg.parameters.remove(clipped)

    alg.processCommand()

    # Re-add the output !
    alg.addOutput(out)


def processOutputs(alg):
    out = alg.getOutputValue('output')
    inputRaster = alg.exportedLayers[out]
    command = u"r.out.gdal --overwrite -c createopt=\"TFW=YES,COMPRESS=LZW\" input={} output=\"{}\"".format(
        inputRaster, out)
    alg.commands.append(command)
    alg.outputCommands.append(command)
