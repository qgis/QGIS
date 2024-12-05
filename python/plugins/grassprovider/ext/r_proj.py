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

__author__ = "Médéric Ribreux"
__date__ = "October 2017"
__copyright__ = "(C) 2017, Médéric Ribreux"

from qgis.core import QgsProcessingParameterString
from processing.tools.system import isWindows
from grassprovider.grass_utils import GrassUtils


def processInputs(alg, parameters, context, feedback):
    # Grab the projection from the input vector layer
    layer = alg.parameterAsLayer(parameters, "input", context)

    # Creates a new location with this Crs
    wkt_file_name = GrassUtils.exportCrsWktToFile(layer.crs(), context)
    newLocation = f"newProj{alg.uniqueSuffix}"
    alg.commands.append(f'g.proj wkt="{wkt_file_name}" location={newLocation}')

    # Go to the newly created location
    alg.commands.append(f"g.mapset mapset=PERMANENT location={newLocation}")

    # Import the layer
    alg.loadRasterLayerFromParameter("input", parameters, context, False)

    # Go back to default location
    alg.commands.append("g.mapset mapset=PERMANENT location=temp_location")

    # Grab the projected Crs
    crs = alg.parameterAsCrs(parameters, "crs", context)
    wkt_file_name = GrassUtils.exportCrsWktToFile(crs, context)
    alg.commands.append(f'g.proj -c wkt="{wkt_file_name}"')

    # Remove crs parameter
    alg.removeParameter("crs")

    # Add the location parameter with proper value
    location = QgsProcessingParameterString(
        "location", "new location", f"newProj{alg.uniqueSuffix}"
    )
    alg.addParameter(location)

    # And set the region
    grassName = alg.exportedLayers["input"]
    # We use the shell to capture the results from r.proj -g
    if isWindows():
        # TODO: make some tests under a non POSIX shell
        alg.commands.append("set regVar=")
        alg.commands.append(
            'for /f "delims=" %%a in (\'r.proj -g input^="{}" location^="{}"\') do @set regVar=%%a'.format(
                grassName, newLocation
            )
        )
        alg.commands.append("g.region -a %regVar%")
    else:
        alg.commands.append(
            'g.region -a $(r.proj -g input="{}" location="{}")'.format(
                grassName, newLocation
            )
        )
