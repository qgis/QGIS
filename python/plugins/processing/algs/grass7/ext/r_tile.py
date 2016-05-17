# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_tile.py
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
    # Remove output for command
    output_dir = alg.getOutputFromName('output_dir')
    alg.removeOutputFromName('output_dir')
    alg.processCommand()
    alg.addOutput(output_dir)


def processOutputs(alg):
    # All the named rasters should be extracted to output_dir
    basename = alg.getParameterValue('output')
    output_dir = alg.getOutputValue('output_dir')

    # Get the list of rasters matching the basename
    commands = ["for r in $(g.list type=rast pattern='{}*'); do".format(basename)]
    commands.append("  r.out.gdal -t input=${{r}} output={}/${{r}}.tif createopt=\"TFW=YES,COMPRESS=LZW\"".format(output_dir))
    commands.append("done")
    alg.commands.extend(commands)
