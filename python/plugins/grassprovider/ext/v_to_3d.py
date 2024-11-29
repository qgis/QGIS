"""
***************************************************************************
    v_to_3d.py
    ----------
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


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    """Verify if we have the right parameters"""
    height = alg.parameterAsDouble(parameters, "height", context)
    column = alg.parameterAsString(parameters, "column", context)
    if (height and column) or (not height and not column):
        return False, alg.tr(
            "You need to set either a fixed height value or the height column!"
        )

    return True, None


def processInputs(alg, parameters, context, feedback):
    if "input" in alg.exportedLayers:
        return

    # We need to import the vector layer with v.in.ogr
    alg.loadVectorLayerFromParameter("input", parameters, context, feedback, False)
    alg.postInputs(context)
