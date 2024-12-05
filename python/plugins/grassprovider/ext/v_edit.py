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

__author__ = "Médéric Ribreux"
__date__ = "March 2016"
__copyright__ = "(C) 2016, Médéric Ribreux"

import os
from processing.tools.system import getTempFilename


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    """Verify if we have the right parameters"""
    if alg.parameterAsString(
        parameters, "input_txt", context
    ) and alg.parameterAsString(parameters, "input", context):
        return False, alg.tr(
            "You need to set either an input ASCII file or inline data!"
        )

    return True, None


def processCommand(alg, parameters, context, feedback):
    # Handle inline rules
    txtRules = alg.parameterAsString(parameters, "input_txt", context)
    if txtRules:
        # Creates a temporary txt file
        tempRulesName = getTempFilename(context=context)

        # Inject rules into temporary txt file
        with open(tempRulesName, "w") as tempRules:
            tempRules.write(txtRules)
        alg.removeParameter("input_txt")
        parameters["input"] = tempRulesName

    alg.processCommand(parameters, context, feedback, True)


def processOutputs(alg, parameters, context, feedback):
    # We need to add the from layer to outputs:
    fileName = alg.parameterAsOutputLayer(parameters, "output", context)
    grassName = alg.exportedLayers["map"]
    dataType = "auto"
    alg.exportVectorLayer(grassName, fileName, dataType=dataType)
