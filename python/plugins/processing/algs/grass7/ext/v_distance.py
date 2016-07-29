# -*- coding: utf-8 -*-

"""
***************************************************************************
    v_distance.py
    -------------
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


def checkParameterValuesBeforeExecuting(alg):
    """ Verify if we have the right parameters """
    # Verify upload value
    upload = alg.getParameterValue(u'upload')
    if upload:
        uploadList = [f for f in upload.split(",") if f not in ['cat', 'dist', 'to_x', 'to_y', 'to_along', 'to_angle', 'to_attr']]
        if len(uploadList) > 0:
            return alg.tr(u"Upload parameters should be a list of elements taken in the following values:\n'cat', 'dist', 'to_x', 'to_y', 'to_along', 'to_angle', 'to_attr'")
        # Verifiy that we have the good number of columns
        column = alg.getParameterValue(u'column')
        if ((column is None or len(column) == 0) and upload) or (len(column.split(",")) != len(upload.split(","))):
            return alg.tr(u"The number of columns and the number of upload parameters should be equal!")

    # Verify from_type and to_type values
    for geom in [u'from', u'to']:
        geoType = alg.getParameterValue(u'{}_type'.format(geom))
        if geoType:
            geoTypeList = [f for f in geoType.split(",") if f not in ['point', 'line', 'boundary', 'centroid', 'area']]
            if len(geoTypeList) > 0:
                return alg.tr(u"Feature type for '{}' should be a list of elements taken in the following values:\n'point', 'line', 'boundary', 'centroid', 'area'".format(geom))

    return None


def processCommand(alg):
    # We temporary remove the output 'from_output'
    fromOutput = alg.getOutputFromName(u'from_output')
    fromParam = alg.getParameterValue(u'from')
    alg.exportedLayers[fromOutput.value] = alg.exportedLayers[fromParam]
    alg.removeOutputFromName(u'from_output')
    alg.processCommand()

    # We re-add the new output
    alg.addOutput(fromOutput)


def processOutputs(alg):
    # Output results ('from' table and output table)
    for output in [u'from_output', u'output']:
        out = alg.getOutputValue(output)
        command = u"v.out.ogr -c type=auto -s -e input={} output=\"{}\" format=ESRI_Shapefile output_layer={}".format(
            alg.exportedLayers[out],
            os.path.dirname(out),
            os.path.basename(out)[:-4]
        )
        alg.commands.append(command)
        alg.outputCommands.append(command)
