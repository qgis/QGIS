# -*- coding: utf-8 -*-

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

__author__ = 'Médéric Ribreux'
__date__ = 'March 2016'
__copyright__ = '(C) 2016, Médéric Ribreux'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    """ Verify if we have the right parameters """
    height = alg.parameterAsDouble(parameters, 'height', context)
    column = alg.parameterAsString(parameters, 'column', context)
    if (height and column) or (not height and not column):
        return alg.tr("You need to set either a fixed height value or the height column!")

    return None


def processInputs(alg, parameters, context):
    if 'input' in alg.exportedLayers:
        return

    # We need to import all the bands and color tables of the input raster
    alg.loadVectorLayerFromParameter('input', parameters, context, False)
    alg.postInputs()
