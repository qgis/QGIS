# -*- coding: utf-8 -*-

"""
***************************************************************************
    v_rectify.py
    ------------
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

from processing.core.parameters import getParameterFromString


def checkParameterValuesBeforeExecuting(alg):
    """ Verify if we have the right parameters """
    if alg.getParameterValue('inline_points') and alg.getParameterValue(u'points'):
        return alg.tr("You need to set either an input control point file or inline control points!")

    return None


def processCommand(alg):
    # handle inline add data
    input_txt = alg.getParameterFromName('inline_points')
    inputParameter = alg.getParameterFromName('points')
    if input_txt.value:
        # Creates a temporary txt file
        ruleFile = alg.getTempFilename()

        # Inject rules into temporary txt file
        with open(ruleFile, "w") as tempRules:
            tempRules.write(input_txt.value)
        inputParameter.value = ruleFile
        alg.parameters.remove(input_txt)

    # exclude output for from_output
    output = alg.getOutputFromName('rmsfile')
    alg.removeOutputFromName('rmsfile')

    # Create a false input parameter for rmsfile
    param = getParameterFromString(u"ParameterString|rmsfile|the file|None|False|False")
    param.value = output.value
    alg.addParameter(param)

    alg.processCommand()
    alg.parameters.remove(param)
    alg.addOutput(output)
    if input_txt.value:
        inputParameter.value = None
        alg.addParameter(input_txt)
