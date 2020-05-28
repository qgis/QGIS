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

from processing.tools.system import getTempFilename


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    """ Verify if we have the right parameters """
    if (alg.parameterAsString(parameters, 'rules', context)
            and alg.parameterAsString(parameters, 'txtrules', context)):
        return False, alg.tr("You need to set either a rules file or write directly the rules!")

    return True, None


def processCommand(alg, parameters, context, feedback):
    """ Handle inline rules """
    txtRules = alg.parameterAsString(parameters, 'txtrules', context)
    if txtRules:
        # Creates a temporary txt file
        tempRulesName = getTempFilename()

        # Inject rules into temporary txt file
        with open(tempRulesName, "w") as tempRules:
            tempRules.write(txtRules)
        alg.removeParameter('txtrules')
        parameters['rules'] = tempRulesName

    alg.processCommand(parameters, context, feedback)
