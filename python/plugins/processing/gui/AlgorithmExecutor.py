# -*- coding: utf-8 -*-

"""
***************************************************************************
    AlgorithmExecutor.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""
from builtins import str


__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import sys

from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (QgsFeature,
                       QgsVectorFileWriter,
                       QgsProcessingFeedback,
                       QgsSettings,
                       QgsProcessingUtils,
                       QgsMessageLog)
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.gui.Postprocessing import handleAlgorithmResults
from processing.tools import dataobjects
from processing.tools.system import getTempFilename


def execute(alg, parameters, context=None, feedback=None):
    """Executes a given algorithm, showing its progress in the
    progress object passed along.

    Return true if everything went OK, false if the algorithm
    could not be completed.
    """

    if feedback is None:
        feedback = QgsProcessingFeedback()
    if context is None:
        context = dataobjects.createContext()

    try:
        alg.execute(parameters, context, feedback)
        return True
    except GeoAlgorithmExecutionException as e:
        QgsMessageLog.logMessage(str(sys.exc_info()[0]), 'Processing', QgsMessageLog.CRITICAL)
        if feedback is not None:
            feedback.reportError(e.msg)
        return False


def executeIterating(alg, parameters, paramToIter, context, feedback):
    # Generate all single-feature layers
    settings = QgsSettings()
    systemEncoding = settings.value('/UI/encoding', 'System')
    layerfile = parameters[paramToIter]
    layer = QgsProcessingUtils.mapLayerFromString(layerfile, context, False)
    feat = QgsFeature()
    filelist = []
    outputs = {}
    features = QgsProcessingUtils.getFeatures(layer, context)
    for feat in features:
        output = getTempFilename('shp')
        filelist.append(output)
        writer = QgsVectorFileWriter(output, systemEncoding,
                                     layer.fields(), layer.wkbType(), layer.crs())
        writer.addFeature(feat)
        del writer

    # store output values to use them later as basenames for all outputs
    for out in alg.outputs:
        outputs[out.name] = out.value

    # now run all the algorithms
    for i, f in enumerate(filelist):
        parameters[paramToIter] = f
        for out in alg.outputs:
            filename = outputs[out.name]
            if filename:
                filename = filename[:filename.rfind('.')] + '_' + str(i) \
                    + filename[filename.rfind('.'):]
            out.value = filename
        feedback.setProgressText(tr('Executing iteration {0}/{1}...').format(i, len(filelist)))
        feedback.setProgress(i * 100 / len(filelist))
        if execute(alg, parameters, None, feedback):
            handleAlgorithmResults(alg, context, None, False)
        else:
            return False

    return True


def tr(string, context=''):
    if context == '':
        context = 'AlgorithmExecutor'
    return QCoreApplication.translate(context, string)
