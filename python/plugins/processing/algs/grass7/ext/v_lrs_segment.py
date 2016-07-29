# -*- coding: utf-8 -*-

"""
***************************************************************************
    v_lrs_segment.py
    ----------------
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


def checkParameterValuesBeforeExecuting(alg):
    """ Verify if we have the right parameters """
    if alg.getParameterValue('in_file') and alg.getParameterValue(u'file'):
        return alg.tr("You need to set either a segment rules file or write directly the rules!")

    return None


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
    in_file = alg.getParameterValue('in_file')
    if in_file:
        # Creates a temporary txt file
        ruleFile = alg.getTempFilename()

        # Inject rules into temporary txt file
        with open(ruleFile, "w") as tempRules:
            tempRules.write(in_file)
    else:
        ruleFile = alg.getParameterValue('file')

    output = alg.getOutputFromName(u'output')
    alg.exportedLayers[output.value] = output.name + alg.uniqueSufix

    command = 'v.lrs.segment input={} file={} rstable={} output={} --overwrite'.format(
        alg.exportedLayers[alg.getParameterValue('input')],
        ruleFile,
        alg.exportedLayers[alg.getParameterValue('rstable')],
        alg.exportedLayers[output.value]
    )
    alg.commands.append(command)
