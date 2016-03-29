# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_series_interp.py
    ------------------
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

import codecs
from processing.tools.system import getTempFilename
from os import path


def checkParameterValuesBeforeExecuting(alg):
    """ Verify if we have the right parameters """
    datapos = alg.getParameterValue(u'datapos')
    infile = alg.getParameterValue(u'infile')
    output = alg.getParameterValue(u'output')
    outfile = alg.getParameterValue(u'outfile')

    if datapos and infile:
        return alg.tr("You need to set either inline data positions or an input data positions file!")
    if output and outfile:
        return alg.tr("You need to set either sampling data positions or an output sampling data positions file!")
    if not (datapos or infile or output or outfile):
        return alg.tr("You need to set input and output data positions parameters!")
    return None


def processCommand(alg):
    # We temporary remove the output directory
    outdir = alg.getOutputFromName('output_dir')
    alg.removeOutputFromName('output_dir')

    alg.processCommand()

    # We re-add the new output
    alg.addOutput(outdir)


def processOutputs(alg):
    # We take all the outputs and we export them to the output directory
    outdir = alg.getOutputFromName('output_dir')
    output = alg.getParameterValue('output')
    outfile = alg.getParameterValue('outfile')
    outs = []
    if output:
        outs = output.split(',')
    elif outfile:
        # Handle file manually to find the name of the layers
        with open(outfile) as f:
            for line in f:
                if '|' in line:
                    outs.append(line.split('|')[0])

    for out in outs:
        command = u"r.out.gdal --overwrite -t -c createopt=\"TFW=YES,COMPRESS=LZW\" input={} output=\"{}\"".format(
            out, path.join(outdir.value, '{}.tif'.format(out)))
        alg.commands.append(command)
        alg.outputCommands.append(command)
