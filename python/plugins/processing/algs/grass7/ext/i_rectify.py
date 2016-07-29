# -*- coding: utf-8 -*-

"""
***************************************************************************
    i_rectify.py
    ------------
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

__author__ = 'Médéric Ribreux'
__date__ = 'April 2016'
__copyright__ = '(C) 2016, Médéric Ribreux'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from i import regroupRasters, copyFile, multipleOutputDir
from qgis.core import QgsMessageLog
from qgis.core import QgsCoordinateReferenceSystem
from ..Grass7Utils import Grass7Utils
from processing.core.parameters import getParameterFromString
from os import path


def processCommand(alg):
    # Creates a new location with the CRS
    crsParam = alg.getParameterFromName('crs')
    crsId = int(crsParam.value[5:])
    #QgsMessageLog.logMessage('crs = {}'.format(crs), 'DEBUG', QgsMessageLog.INFO)
    crs = QgsCoordinateReferenceSystem()
    crs.createFromId(crsId, QgsCoordinateReferenceSystem.EpsgCrsId)
    command = "g.proj proj4=\"{}\" location=TARGET".format(crs.toProj4())
    alg.commands.append(command)
    alg.parameters.remove(crsParam)

    # Regroup rasters
    rasters = alg.getParameterFromName('rasters')
    rastersList = rasters.value.split(';')
    alg.parameters.remove(rasters)

    # Insert a i.group command
    group = getParameterFromString("ParameterString|group|group of rasters|None|False|False")
    group.value = alg.getTempFilename()
    alg.addParameter(group)

    command = 'i.group group={} input={}'.format(
        group.value,
        ','.join([alg.exportedLayers[f] for f in rastersList])
    )
    alg.commands.append(command)

    # Handle POINT File
    gcp = alg.getParameterFromName('gcp')
    extFileName = gcp.value
    destPath = path.join(Grass7Utils.grassMapsetFolder(),
                         'PERMANENT',
                         'group', group.value,
                         'POINTS')
    copyFile(alg, extFileName, destPath)
    alg.parameters.remove(gcp)

    # Add a target destination for our group
    command = "i.target group={} location=TARGET mapset=PERMANENT".format(group.value)
    alg.commands.append(command)

    # remove output
    output = alg.getOutputFromName('output')
    alg.removeOutputFromName('output')

    # Add an extension
    #extension = getParameterFromString("ParameterString|extension|Output raster map(s) suffix|None|False|False")
    #extension.value = "rectified"
    #alg.addParameter(extension)

    # modify parameters values
    alg.processCommand()

    # Re-add input rasters
    alg.addParameter(rasters)
    alg.addParameter(gcp)
    alg.addParameter(crs)

    # Re-add output
    alg.addOutput(output)


def processOutputs(alg):
    # We need to export from the TARGET location
    command = "g.mapset location=TARGET mapset=PERMANENT"
    alg.commands.append(command)
    multipleOutputDir(alg, 'output')
