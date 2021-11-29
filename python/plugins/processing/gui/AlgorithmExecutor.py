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

import sys
from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (Qgis,
                       QgsApplication,
                       QgsFeatureSink,
                       QgsProcessingFeedback,
                       QgsProcessingUtils,
                       QgsMessageLog,
                       QgsProcessingException,
                       QgsProcessingFeatureSourceDefinition,
                       QgsProcessingFeatureSource,
                       QgsProcessingParameters,
                       QgsProject,
                       QgsFeatureRequest,
                       QgsFeature,
                       QgsExpression,
                       QgsWkbTypes,
                       QgsGeometry,
                       QgsVectorLayerUtils,
                       QgsVectorLayer)
from processing.gui.Postprocessing import handleAlgorithmResults
from processing.tools import dataobjects
from qgis.utils import iface


def execute(alg, parameters, context=None, feedback=None, catch_exceptions=True):
    """Executes a given algorithm, showing its progress in the
    progress object passed along.

    Return true if everything went OK, false if the algorithm
    could not be completed.
    """

    if feedback is None:
        feedback = QgsProcessingFeedback()
    if context is None:
        context = dataobjects.createContext(feedback)

    if catch_exceptions:
        try:
            results, ok = alg.run(parameters, context, feedback)
            return ok, results
        except QgsProcessingException as e:
            QgsMessageLog.logMessage(str(sys.exc_info()[0]), 'Processing', Qgis.Critical)
            if feedback is not None:
                feedback.reportError(e.msg)
            return False, {}
    else:
        results, ok = alg.run(parameters, context, feedback, {}, False)
        return ok, results


def execute_in_place_run(alg, parameters, context=None, feedback=None, raise_exceptions=False):
    """Executes an algorithm modifying features in-place in the input layer.

    :param alg: algorithm to run
    :type alg: QgsProcessingAlgorithm
    :param parameters: parameters of the algorithm
    :type parameters: dict
    :param context: context, defaults to None
    :type context: QgsProcessingContext, optional
    :param feedback: feedback, defaults to None
    :type feedback: QgsProcessingFeedback, optional
    :param raise_exceptions: useful for testing, if True exceptions are raised, normally exceptions will be forwarded to the feedback
    :type raise_exceptions: boo, default to False
    :raises QgsProcessingException: raised when there is no active layer, or it cannot be made editable
    :return: a tuple with true if success and results
    :rtype: tuple
    """

    if feedback is None:
        feedback = QgsProcessingFeedback()
    if context is None:
        context = dataobjects.createContext(feedback)

    # Only feature based algs have sourceFlags
    try:
        if alg.sourceFlags() & QgsProcessingFeatureSource.FlagSkipGeometryValidityChecks:
            context.setInvalidGeometryCheck(QgsFeatureRequest.GeometryNoCheck)
    except AttributeError:
        pass

    in_place_input_parameter_name = 'INPUT'
    if hasattr(alg, 'inputParameterName'):
        in_place_input_parameter_name = alg.inputParameterName()

    active_layer = parameters[in_place_input_parameter_name]

    # prepare expression context for feature iteration
    alg_context = context.expressionContext()
    alg_context.appendScope(active_layer.createExpressionContextScope())
    context.setExpressionContext(alg_context)

    # Run some checks and prepare the layer for in-place execution by:
    # - getting the active layer and checking that it is a vector
    # - making the layer editable if it was not already
    # - selecting all features if none was selected
    # - checking in-place support for the active layer/alg/parameters
    # If one of the check fails and raise_exceptions is True an exception
    # is raised, else the execution is aborted and the error reported in
    # the feedback
    try:
        if active_layer is None:
            raise QgsProcessingException(tr("There is no active layer."))

        if not isinstance(active_layer, QgsVectorLayer):
            raise QgsProcessingException(tr("Active layer is not a vector layer."))

        if not active_layer.isEditable():
            if not active_layer.startEditing():
                raise QgsProcessingException(tr("Active layer is not editable (and editing could not be turned on)."))

        if not alg.supportInPlaceEdit(active_layer):
            raise QgsProcessingException(tr("Selected algorithm and parameter configuration are not compatible with in-place modifications."))
    except QgsProcessingException as e:
        if raise_exceptions:
            raise e
        QgsMessageLog.logMessage(str(sys.exc_info()[0]), 'Processing', Qgis.Critical)
        if feedback is not None:
            feedback.reportError(getattr(e, 'msg', str(e)), fatalError=True)
        return False, {}

    if not active_layer.selectedFeatureIds():
        active_layer.selectAll()

    # Make sure we are working on selected features only
    parameters[in_place_input_parameter_name] = QgsProcessingFeatureSourceDefinition(active_layer.id(), True)
    parameters['OUTPUT'] = 'memory:'

    req = QgsFeatureRequest(QgsExpression(r"$id < 0"))
    req.setFlags(QgsFeatureRequest.NoGeometry)
    req.setSubsetOfAttributes([])

    # Start the execution
    # If anything goes wrong and raise_exceptions is True an exception
    # is raised, else the execution is aborted and the error reported in
    # the feedback
    try:
        new_feature_ids = []

        active_layer.beginEditCommand(alg.displayName())

        # Checks whether the algorithm has a processFeature method
        if hasattr(alg, 'processFeature'):  # in-place feature editing
            # Make a clone or it will crash the second time the dialog
            # is opened and run
            alg = alg.create({'IN_PLACE': True})
            if not alg.prepare(parameters, context, feedback):
                raise QgsProcessingException(tr("Could not prepare selected algorithm."))
            # Check again for compatibility after prepare
            if not alg.supportInPlaceEdit(active_layer):
                raise QgsProcessingException(tr("Selected algorithm and parameter configuration are not compatible with in-place modifications."))

            # some algorithms have logic in outputFields/outputCrs/outputWkbType which they require to execute before
            # they can start processing features
            _ = alg.outputFields(active_layer.fields())
            _ = alg.outputWkbType(active_layer.wkbType())
            _ = alg.outputCrs(active_layer.crs())

            field_idxs = range(len(active_layer.fields()))
            iterator_req = QgsFeatureRequest(active_layer.selectedFeatureIds())
            iterator_req.setInvalidGeometryCheck(context.invalidGeometryCheck())
            feature_iterator = active_layer.getFeatures(iterator_req)
            step = 100 / len(active_layer.selectedFeatureIds()) if active_layer.selectedFeatureIds() else 1
            current = 0
            for current, f in enumerate(feature_iterator):
                if feedback.isCanceled():
                    break

                # need a deep copy, because python processFeature implementations may return
                # a shallow copy from processFeature
                input_feature = QgsFeature(f)

                context.expressionContext().setFeature(input_feature)

                new_features = alg.processFeature(input_feature, context, feedback)
                new_features = QgsVectorLayerUtils.makeFeaturesCompatible(new_features, active_layer)

                if len(new_features) == 0:
                    active_layer.deleteFeature(f.id())
                elif len(new_features) == 1:
                    new_f = new_features[0]
                    if not f.geometry().equals(new_f.geometry()):
                        active_layer.changeGeometry(f.id(), new_f.geometry())
                    if f.attributes() != new_f.attributes():
                        active_layer.changeAttributeValues(f.id(), dict(zip(field_idxs, new_f.attributes())), dict(zip(field_idxs, f.attributes())))
                    new_feature_ids.append(f.id())
                else:
                    active_layer.deleteFeature(f.id())
                    # Get the new ids
                    old_ids = set([f.id() for f in active_layer.getFeatures(req)])
                    # If multiple new features were created, we need to pass
                    # them to createFeatures to manage constraints correctly
                    features_data = []
                    for f in new_features:
                        features_data.append(QgsVectorLayerUtils.QgsFeatureData(f.geometry(), dict(enumerate(f.attributes()))))
                    new_features = QgsVectorLayerUtils.createFeatures(active_layer, features_data, context.expressionContext())
                    if not active_layer.addFeatures(new_features):
                        raise QgsProcessingException(tr("Error adding processed features back into the layer."))
                    new_ids = set([f.id() for f in active_layer.getFeatures(req)])
                    new_feature_ids += list(new_ids - old_ids)

                feedback.setProgress(int((current + 1) * step))

            results, ok = {'__count': current + 1}, True

        else:  # Traditional 'run' with delete and add features cycle

            # There is no way to know if some features have been skipped
            # due to invalid geometries
            if context.invalidGeometryCheck() == QgsFeatureRequest.GeometrySkipInvalid:
                selected_ids = active_layer.selectedFeatureIds()
            else:
                selected_ids = []

            results, ok = alg.run(parameters, context, feedback, configuration={'IN_PLACE': True})

            if ok:
                result_layer = QgsProcessingUtils.mapLayerFromString(results['OUTPUT'], context)
                # TODO: check if features have changed before delete/add cycle

                new_features = []

                # Check if there are any skipped features
                if context.invalidGeometryCheck() == QgsFeatureRequest.GeometrySkipInvalid:
                    missing_ids = list(set(selected_ids) - set(result_layer.allFeatureIds()))
                    if missing_ids:
                        for f in active_layer.getFeatures(QgsFeatureRequest(missing_ids)):
                            if not f.geometry().isGeosValid():
                                new_features.append(f)

                active_layer.deleteFeatures(active_layer.selectedFeatureIds())

                regenerate_primary_key = result_layer.customProperty('OnConvertFormatRegeneratePrimaryKey', False)
                sink_flags = QgsFeatureSink.SinkFlags(QgsFeatureSink.RegeneratePrimaryKey) if regenerate_primary_key \
                    else QgsFeatureSink.SinkFlags()

                for f in result_layer.getFeatures():
                    new_features.extend(QgsVectorLayerUtils.
                                        makeFeaturesCompatible([f], active_layer, sink_flags))

                # Get the new ids
                old_ids = set([f.id() for f in active_layer.getFeatures(req)])
                if not active_layer.addFeatures(new_features):
                    raise QgsProcessingException(tr("Error adding processed features back into the layer."))
                new_ids = set([f.id() for f in active_layer.getFeatures(req)])
                new_feature_ids += list(new_ids - old_ids)
                results['__count'] = len(new_feature_ids)

        active_layer.endEditCommand()

        if ok and new_feature_ids:
            active_layer.selectByIds(new_feature_ids)
        elif not ok:
            active_layer.rollBack()

        return ok, results

    except QgsProcessingException as e:
        active_layer.endEditCommand()
        active_layer.rollBack()
        if raise_exceptions:
            raise e
        QgsMessageLog.logMessage(str(sys.exc_info()[0]), 'Processing', Qgis.Critical)
        if feedback is not None:
            feedback.reportError(getattr(e, 'msg', str(e)), fatalError=True)

    return False, {}


def execute_in_place(alg, parameters, context=None, feedback=None):
    """Executes an algorithm modifying features in-place, if the INPUT
    parameter is not defined, the current active layer will be used as
    INPUT.

    :param alg: algorithm to run
    :type alg: QgsProcessingAlgorithm
    :param parameters: parameters of the algorithm
    :type parameters: dict
    :param context: context, defaults to None
    :param context: QgsProcessingContext, optional
    :param feedback: feedback, defaults to None
    :param feedback: QgsProcessingFeedback, optional
    :raises QgsProcessingException: raised when the layer is not editable or the layer cannot be found in the current project
    :return: a tuple with true if success and results
    :rtype: tuple
    """

    if feedback is None:
        feedback = QgsProcessingFeedback()
    if context is None:
        context = dataobjects.createContext(feedback)

    in_place_input_parameter_name = 'INPUT'
    if hasattr(alg, 'inputParameterName'):
        in_place_input_parameter_name = alg.inputParameterName()
    in_place_input_layer_name = 'INPUT'
    if hasattr(alg, 'inputParameterDescription'):
        in_place_input_layer_name = alg.inputParameterDescription()

    if in_place_input_parameter_name not in parameters or not parameters[in_place_input_parameter_name]:
        parameters[in_place_input_parameter_name] = iface.activeLayer()
    ok, results = execute_in_place_run(alg, parameters, context=context, feedback=feedback)
    if ok:
        if isinstance(parameters[in_place_input_parameter_name], QgsProcessingFeatureSourceDefinition):
            layer = alg.parameterAsVectorLayer({in_place_input_parameter_name: parameters[in_place_input_parameter_name].source}, in_place_input_layer_name, context)
        elif isinstance(parameters[in_place_input_parameter_name], QgsVectorLayer):
            layer = parameters[in_place_input_parameter_name]
        if layer:
            layer.triggerRepaint()
    return ok, results


def executeIterating(alg, parameters, paramToIter, context, feedback):
    # Generate all single-feature layers
    parameter_definition = alg.parameterDefinition(paramToIter)
    if not parameter_definition:
        return False

    iter_source = QgsProcessingParameters.parameterAsSource(parameter_definition, parameters, context)
    sink_list = []
    if iter_source.featureCount() == 0:
        return False

    step = 100.0 / iter_source.featureCount()
    for current, feat in enumerate(iter_source.getFeatures()):
        if feedback.isCanceled():
            return False

        sink, sink_id = QgsProcessingUtils.createFeatureSink('memory:', context, iter_source.fields(), iter_source.wkbType(), iter_source.sourceCrs())
        sink_list.append(sink_id)
        sink.addFeature(feat, QgsFeatureSink.FastInsert)
        del sink

        feedback.setProgress(int((current + 1) * step))

    # store output values to use them later as basenames for all outputs
    outputs = {}
    for out in alg.destinationParameterDefinitions():
        if out.name() in parameters:
            outputs[out.name()] = parameters[out.name()]

    # now run all the algorithms
    for i, f in enumerate(sink_list):
        if feedback.isCanceled():
            return False

        parameters[paramToIter] = f
        for out in alg.destinationParameterDefinitions():
            if out.name() not in outputs:
                continue

            o = outputs[out.name()]
            parameters[out.name()] = QgsProcessingUtils.generateIteratingDestination(o, i, context)
        feedback.setProgressText(QCoreApplication.translate('AlgorithmExecutor', 'Executing iteration {0}/{1}…').format(i + 1, len(sink_list)))
        feedback.setProgress(int((i + 1) * 100 / len(sink_list)))
        ret, results = execute(alg, parameters, context, feedback)
        if not ret:
            return False

    handleAlgorithmResults(alg, context, feedback, False)
    return True


def tr(string, context=''):
    if context == '':
        context = 'AlgorithmExecutor'
    return QCoreApplication.translate(context, string)
