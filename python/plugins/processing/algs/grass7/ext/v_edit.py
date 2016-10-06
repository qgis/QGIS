# -*- coding: utf-8 -*-

"""
***************************************************************************
    v_edit.py
    ---------
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


def checkParameterValuesBeforeExecuting(alg):
    """ Verify if we have the right parameters """
    if alg.getParameterValue('input_txt') and alg.getParameterValue(u'input'):
        return alg.tr("You need to set either an input ASCII file or inline data!")

    return None


def processCommand(alg):
    # handle inline add data
    input_txt = alg.getParameterFromName('input_txt')
    inputParameter = alg.getParameterFromName('input')
    if input_txt.value:
        # Creates a temporary txt file
        ruleFile = alg.getTempFilename()

        # Inject rules into temporary txt file
        with open(ruleFile, "w") as tempRules:
            tempRules.write(input_txt.value)
        inputParameter.value = ruleFile
        alg.parameters.remove(input_txt)

    # exclude output for from_output
    output = alg.getOutputFromName('output')
    alg.removeOutputFromName('output')

    alg.processCommand()
    alg.addOutput(output)
    if input_txt.value:
        inputParameter.value = None
        alg.addParameter(input_txt)


def processOutputs(alg):
    # We need to add the from layer to outputs:
    out = alg.exportedLayers[alg.getParameterValue('map')]
    from_out = alg.getOutputValue('output')
    command = u"v.out.ogr -s -e input={} output=\"{}\" format=ESRI_Shapefile output_layer={}".format(
        out, os.path.dirname(from_out),
        os.path.splitext(os.path.basename(from_out))[0]
    )
    alg.commands.append(command)
    alg.outputCommands.append(command)
