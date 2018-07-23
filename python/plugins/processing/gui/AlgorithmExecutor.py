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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import sys
from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (Qgis,
                       QgsFeatureSink,
                       QgsProcessingFeedback,
                       QgsProcessingUtils,
                       QgsMessageLog,
                       QgsProcessingException,
                       QgsProcessingFeatureSourceDefinition,
                       QgsProcessingParameters)
from processing.gui.Postprocessing import handleAlgorithmResults
from processing.tools import dataobjects
from qgis.utils import iface


def execute(alg, parameters, context=None, feedback=None):
    """Executes a given algorithm, showing its progress in the
    progress object passed along.

    Return true if everything went OK, false if the algorithm
    could not be completed.
    """

    if feedback is None:
        feedback = QgsProcessingFeedback()
    if context is None:
        context = dataobjects.createContext(feedback)

    try:
        results, ok = alg.run(parameters, context, feedback)
        return ok, results
    except QgsProcessingException as e:
        QgsMessageLog.logMessage(str(sys.exc_info()[0]), 'Processing', Qgis.Critical)
        if feedback is not None:
            feedback.reportError(e.msg)
        return False, {}


def execute_in_place(alg, parameters, context=None, feedback=None):

    if feedback is None:
        feedback = QgsProcessingFeedback()
    if context is None:
        context = dataobjects.createContext(feedback)

    parameters['INPUT'] = QgsProcessingFeatureSourceDefinition(iface.activeLayer().id(), True)

    parameters['OUTPUT'] = 'memory:'

    try:
        results, ok = alg.run(parameters, context, feedback)

        layer = QgsProcessingUtils.mapLayerFromString(results['OUTPUT'], context)
        iface.activeLayer().beginEditCommand('Edit features')
        iface.activeLayer().deleteFeatures(iface.activeLayer().selectedFeatureIds())
        features = []
        for f in layer.getFeatures():
            features.append(f)
        iface.activeLayer().addFeatures(features)
        new_selection = [f.id() for f in features]
        iface.activeLayer().endEditCommand()
        #iface.activeLayer().selectByIds(new_selection)
        iface.activeLayer().triggerRepaint()

        return ok, results
    except QgsProcessingException as e:
        QgsMessageLog.logMessage(str(sys.exc_info()[0]), 'Processing', Qgis.Critical)
        if feedback is not None:
            feedback.reportError(e.msg)
        return False, {}


def executeIterating(alg, parameters, paramToIter, context, feedback):
    # Generate all single-feature layers
    parameter_definition = alg.parameterDefinition(paramToIter)
    if not parameter_definition:
        return False

    iter_source = QgsProcessingParameters.parameterAsSource(parameter_definition, parameters, context)
    sink_list = []
    if iter_source.featureCount() == 0:
        return False

    total = 100.0 / iter_source.featureCount()
    for current, feat in enumerate(iter_source.getFeatures()):
        if feedback.isCanceled():
            return False

        sink, sink_id = QgsProcessingUtils.createFeatureSink('memory:', context, iter_source.fields(), iter_source.wkbType(), iter_source.sourceCrs())
        sink_list.append(sink_id)
        sink.addFeature(feat, QgsFeatureSink.FastInsert)
        del sink

        feedback.setProgress(int(current * total))

    # store output values to use them later as basenames for all outputs
    outputs = {}
    for out in alg.destinationParameterDefinitions():
        outputs[out.name()] = parameters[out.name()]

    # now run all the algorithms
    for i, f in enumerate(sink_list):
        if feedback.isCanceled():
            return False

        parameters[paramToIter] = f
        for out in alg.destinationParameterDefinitions():
            o = outputs[out.name()]
            parameters[out.name()] = QgsProcessingUtils.generateIteratingDestination(o, i, context)
        feedback.setProgressText(QCoreApplication.translate('AlgorithmExecutor', 'Executing iteration {0}/{1}…').format(i, len(sink_list)))
        feedback.setProgress(i * 100 / len(sink_list))
        ret, results = execute(alg, parameters, context, feedback)
        if not ret:
            return False

    handleAlgorithmResults(alg, context, feedback, False)
    return True


def tr(string, context=''):
    if context == '':
        context = 'AlgorithmExecutor'
    return QCoreApplication.translate(context, string)
