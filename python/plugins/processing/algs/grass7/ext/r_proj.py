# -*- coding: utf-8 -*-

"""
***************************************************************************
    r_proj.py
    ---------
    Date                 : October 2017
    Copyright            : (C) 2017 by Médéric Ribreux
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
__date__ = 'October 2017'
__copyright__ = '(C) 2017, Médéric Ribreux'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import QgsProcessingParameterString
from ..Grass7Utils import isWindows


def processInputs(alg, parameters, context, feedback):
    # Grab the projection from the input vector layer
    layer = alg.parameterAsLayer(parameters, 'input', context)
    layerCrs = layer.crs().toProj4()

    # Creates a new location with this Crs
    newLocation = 'newProj{}'.format(alg.uniqueSuffix)
    alg.commands.append('g.proj proj4="{}" location={}'.format(
        layerCrs, newLocation))

    # Go to the newly created location
    alg.commands.append('g.mapset mapset=PERMANENT location={}'.format(
        newLocation))

    # Import the layer
    alg.loadRasterLayerFromParameter(
        'input', parameters, context, False)

    # Go back to default location
    alg.commands.append('g.mapset mapset=PERMANENT location=temp_location')

    # Grab the projected Crs
    crs = alg.parameterAsCrs(parameters, 'crs', context)
    alg.commands.append('g.proj -c proj4="{}"'.format(
        crs.toProj4(), newLocation))

    # Remove crs parameter
    alg.removeParameter('crs')

    # Add the location parameter with proper value
    location = QgsProcessingParameterString(
        'location',
        'new location',
        'newProj{}'.format(alg.uniqueSuffix)
    )
    alg.addParameter(location)

    # And set the region
    grassName = alg.exportedLayers['input']
    # We use the shell to capture the results from r.proj -g
    if isWindows():
        # TODO: make some tests under a non POSIX shell
        alg.commands.append('set regVar=')
        alg.commands.append('for /f "delims=" %%a in (\'r.proj -g input="{}" location="{}"\') do @set theValue=%%a'.format(
            grassName, newLocation))
        alg.commands.append('g.region -a %regVar%')
    else:
        alg.commands.append('g.region -a $(r.proj -g input="{}" location="{}")'.format(
            grassName, newLocation))
