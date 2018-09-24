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
                       QgsProcessingParameters,
                       QgsProject,
                       QgsFeatureRequest,
                       QgsFeature,
                       QgsExpression,
                       QgsWkbTypes,
                       QgsGeometry,
                       QgsVectorLayerUtils)
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


def make_features_compatible(new_features, input_layer):
    """Try to make the new features compatible with old features by:

    - converting single to multi part
    - dropping additional attributes
    - adding back M/Z values
    - drop Z/M
    - convert multi part to single part

    :param new_features: new features
    :type new_features: list of QgsFeatures
    :param input_layer: input layer
    :type input_layer: QgsVectorLayer
    :return: modified features
    :rtype: list of QgsFeatures
    """

    input_wkb_type = input_layer.wkbType()
    result_features = []
    for new_f in new_features:
        # Fix attributes
        QgsVectorLayerUtils.matchAttributesToFields(new_f, input_layer.fields())

        # Check if we need geometry manipulation
        new_f_geom_type = QgsWkbTypes.geometryType(new_f.geometry().wkbType())
        new_f_has_geom = new_f_geom_type not in (QgsWkbTypes.UnknownGeometry, QgsWkbTypes.NullGeometry)
        input_layer_has_geom = input_wkb_type not in (QgsWkbTypes.NoGeometry, QgsWkbTypes.Unknown)

        # Drop geometry if layer is geometry-less
        if not input_layer_has_geom and new_f_has_geom:
            f = QgsFeature(input_layer.fields())
            f.setAttributes(new_f.attributes())
            new_f = f
            result_features.append(new_f)
            continue  # skip the rest

        if input_layer_has_geom and new_f_has_geom and \
                new_f.geometry().wkbType() != input_wkb_type:  # Fix geometry
            # Single -> Multi
            if (QgsWkbTypes.isMultiType(input_wkb_type) and not
                    new_f.geometry().isMultipart()):
                new_geom = new_f.geometry()
                new_geom.convertToMultiType()
                new_f.setGeometry(new_geom)
            # Drop Z/M
            if (new_f.geometry().constGet().is3D() and not QgsWkbTypes.hasZ(input_wkb_type)):
                new_geom = new_f.geometry()
                new_geom.get().dropZValue()
                new_f.setGeometry(new_geom)
            if (new_f.geometry().constGet().isMeasure() and not QgsWkbTypes.hasM(input_wkb_type)):
                new_geom = new_f.geometry()
                new_geom.get().dropMValue()
                new_f.setGeometry(new_geom)
            # Add Z/M back (set it to 0)
            if (not new_f.geometry().constGet().is3D() and QgsWkbTypes.hasZ(input_wkb_type)):
                new_geom = new_f.geometry()
                new_geom.get().addZValue(0.0)
                new_f.setGeometry(new_geom)
            if (not new_f.geometry().constGet().isMeasure() and QgsWkbTypes.hasM(input_wkb_type)):
                new_geom = new_f.geometry()
                new_geom.get().addMValue(0.0)
                new_f.setGeometry(new_geom)
            # Multi -> Single
            if (not QgsWkbTypes.isMultiType(input_wkb_type) and
                    new_f.geometry().isMultipart()):
                g = new_f.geometry()
                g2 = g.constGet()
                for i in range(g2.partCount()):
                    # Clone or crash!
                    g4 = QgsGeometry(g2.geometryN(i).clone())
                    f = QgsVectorLayerUtils.createFeature(input_layer, g4, {i: new_f.attribute(i) for i in range(new_f.fields().count())})
                    result_features.append(f)
            else:
                result_features.append(new_f)
        else:
            result_features.append(new_f)
    return result_features


def execute_in_place_run(alg, active_layer, parameters, context=None, feedback=None, raise_exceptions=False):
    """Executes an algorithm modifying features in-place in the input layer.

    The input layer must be editable or an exception is raised.

    :param alg: algorithm to run
    :type alg: QgsProcessingAlgorithm
    :param active_layer: the editable layer
    :type active_layer: QgsVectoLayer
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

    if active_layer is None or not active_layer.isEditable():
        raise QgsProcessingException(tr("Layer is not editable or layer is None."))

    if not alg.supportInPlaceEdit(active_layer):
        raise QgsProcessingException(tr("Selected algorithm and parameter configuration are not compatible with in-place modifications."))

    parameters['OUTPUT'] = 'memory:'

    try:
        new_feature_ids = []

        active_layer.beginEditCommand(alg.displayName())

        req = QgsFeatureRequest(QgsExpression(r"$id < 0"))
        req.setFlags(QgsFeatureRequest.NoGeometry)
        req.setSubsetOfAttributes([])

        # Checks whether the algorithm has a processFeature method
        if hasattr(alg, 'processFeature'):  # in-place feature editing
            # Make a clone or it will crash the second time the dialog
            # is opened and run
            alg = alg.create()
            if not alg.prepare(parameters, context, feedback):
                raise QgsProcessingException(tr("Could not prepare selected algorithm."))
            # Check again for compatibility after prepare
            if not alg.supportInPlaceEdit(active_layer):
                raise QgsProcessingException(tr("Selected algorithm and parameter configuration are not compatible with in-place modifications."))
            field_idxs = range(len(active_layer.fields()))
            feature_iterator = active_layer.getFeatures(QgsFeatureRequest(active_layer.selectedFeatureIds())) if parameters['INPUT'].selectedFeaturesOnly else active_layer.getFeatures()
            for f in feature_iterator:
                # need a deep copy, because python processFeature implementations may return
                # a shallow copy from processFeature
                input_feature = QgsFeature(f)
                new_features = alg.processFeature(input_feature, context, feedback)
                new_features = make_features_compatible(new_features, active_layer)
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
                    if not active_layer.addFeatures(new_features):
                        raise QgsProcessingException(tr("Error adding processed features back into the layer."))
                    new_ids = set([f.id() for f in active_layer.getFeatures(req)])
                    new_feature_ids += list(new_ids - old_ids)

            results, ok = {}, True

        else:  # Traditional 'run' with delete and add features cycle
            results, ok = alg.run(parameters, context, feedback)

            if ok:
                result_layer = QgsProcessingUtils.mapLayerFromString(results['OUTPUT'], context)
                # TODO: check if features have changed before delete/add cycle
                active_layer.deleteFeatures(active_layer.selectedFeatureIds())
                new_features = []
                for f in result_layer.getFeatures():
                    new_features.extend(make_features_compatible([f], active_layer))

                # Get the new ids
                old_ids = set([f.id() for f in active_layer.getFeatures(req)])
                if not active_layer.addFeatures(new_features):
                    raise QgsProcessingException(tr("Error adding processed features back into the layer."))
                new_ids = set([f.id() for f in active_layer.getFeatures(req)])
                new_feature_ids += list(new_ids - old_ids)

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
            feedback.reportError(getattr(e, 'msg', str(e)))

    return False, {}


def execute_in_place(alg, parameters, context=None, feedback=None):
    """Executes an algorithm modifying features in-place in the active layer.

    The input layer must be editable or an exception is raised.

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

    parameters['INPUT'] = QgsProcessingFeatureSourceDefinition(iface.activeLayer().id(), True)
    ok, results = execute_in_place_run(alg, iface.activeLayer(), parameters, context=context, feedback=feedback)
    if ok:
        iface.activeLayer().triggerRepaint()
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
