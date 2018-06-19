# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_mapcalc.py
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

import os


def checkParameterValuesBeforeExecuting(alg, parameters, context):
    """ Verify if we have the right parameters """
    if (alg.parameterAsString(parameters, 'expression', context)
            and alg.parameterAsString(parameters, 'file', context)):
        return False, alg.tr("You need to set either inline expression or a rules file!")

    return True, None


def processInputs(alg, parameters, context, feedback):
    # We will use the same raster names than in QGIS to name the rasters in GRASS
    rasters = alg.parameterAsLayerList(parameters, 'maps', context)
    for idx, raster in enumerate(rasters):
        rasterName = os.path.splitext(
            os.path.basename(raster.source()))[0]
        alg.inputLayers.append(raster)
        alg.setSessionProjectionFromLayer(raster)
        command = 'r.in.gdal input="{0}" output="{1}" --overwrite -o'.format(
            os.path.normpath(raster.source()),
            rasterName)
        alg.commands.append(command)

    alg.removeParameter('maps')
    alg.postInputs()


def processCommand(alg, parameters, context, feedback):
    alg.processCommand(parameters, context, feedback, True)


def processOutputs(alg, parameters, context, feedback):
    # We need to export every raster from the GRASSDB
    alg.exportRasterLayersIntoDirectory('output_dir',
                                        parameters, context,
                                        wholeDB=True)
