# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_reclass.py
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


def checkParameterValuesBeforeExecuting(alg):
    """ Verify if we have the right parameters """
    if alg.getParameterValue(u'rules') and alg.getParameterValue(u'txtrules'):
        return alg.tr("You need to set either a rules file or write directly the rules!")

    return None


def processCommand(alg):
    """ Handle inline rules """
    txtRules = alg.getParameterValue(u'txtrules')
    if txtRules:
        # Creates a temporary txt file
        tempRulesName = alg.getTempFilename()

        # Inject rules into temporary txt file
        with open(tempRulesName, "w") as tempRules:
            tempRules.write(txtRules)

        raster = alg.getParameterValue(u'input')
        output = alg.getOutputFromName(u'output')
        alg.exportedLayers[output.value] = output.name + alg.uniqueSufix
        if raster:
            raster = alg.exportedLayers[raster]
        command = u"r.reclass input={} rules=- output={} --overwrite < {}".format(
            raster, output.name + alg.uniqueSufix, tempRulesName)
        alg.commands.append(command)
    else:
        alg.processCommand()
