# -*- coding: utf-8 -*-

"""
***************************************************************************
    i_pansharpen.py
    ---------------
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

from i import exportInputRasters
from processing.core.parameters import ParameterRaster, getParameterFromString


def processCommand(alg):

    # Temporary remove outputs:
    outputs = [alg.getOutputFromName('{}output'.format(f)) for f in ['red', 'green', 'blue']]
    for out in outputs:
        alg.removeOutputFromName(out.name)

    # create a false output
    base = getParameterFromString('ParameterString|output|Name for output basename raster map(s)|None|False|False')
    base.value = alg.getTempFilename()
    alg.addParameter(base)
    alg.processCommand()

    # Re-add outputs
    for output in outputs:
        alg.addOutput(output)


def processOutputs(alg):
    base = alg.getParameterValue('output')
    for channel in ['red', 'green', 'blue']:
        command = 'r.out.gdal -c -t -f --overwrite createopt="TFW=YES,COMPRESS=LZW" input={} output=\"{}\"'.format(
            '{}_{}'.format(base, channel),
            alg.getOutputValue('{}output'.format(channel))
        )
        alg.commands.append(command)
        alg.outputCommands.append(command)
