"""
***************************************************************************
    i_in_spotvgt.py
    ---------------
    Date                 : April 2016
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
__date__ = "April 2016"
__copyright__ = "(C) 2016, Médéric Ribreux"


def processInputs(alg, parameters, context, feedback):
    # Here, we apply directly the algorithm
    # So we just need to get the projection of the layer !
    layer = alg.parameterAsRasterLayer(parameters, "input", context)
    alg.setSessionProjectionFromLayer(layer, context)
