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
                       QgsGeometry)
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


def make_feature_compatible(new_features, input_layer):
    """Try to make new features compatible with old feature by:

    - converting single to multi part
    - dropping additional attributes
    - adding back M/Z values


    :param new_features: new features
    :type new_features: list of QgsFeatures
    :param input_layer: input layer
    :type input_layer: QgsVectorLayer
    :return: modified features
    :rtype: list of QgsFeatures
    """

    result_features = []
    for new_f in new_features:
        if new_f.geometry().wkbType() != input_layer.wkbType():
            # Single -> Multi
            if (QgsWkbTypes.isMultiType(input_layer.wkbType()) and not
                    new_f.geometry().isMultipart()):
                new_geom = new_f.geometry()
                new_geom.convertToMultiType()
                new_f.setGeometry(new_geom)
            # Drop Z/M
            if ((QgsWkbTypes.hasZ(new_f.geometry().wkbType()) and not QgsWkbTypes.hasZ(input_layer.wkbType())) or
                    (QgsWkbTypes.hasM(new_f.geometry().wkbType()) and not QgsWkbTypes.hasM(input_layer.wkbType()))):
                # FIXME: there must be a better way!!!
                if new_f.geometry().type() == QgsWkbTypes.PointGeometry:
                    if new_f.geometry().isMultipart():
                        new_geom = QgsGeometry.fromWkt(new_f.geometry().asMultiPoint().asWkt())
                    else:
                        new_geom = QgsGeometry.fromWkt(new_f.geometry().asPoint().asWkt())
                elif new_f.geometry().type() == QgsWkbTypes.PolygonGeometry:
                    if new_f.geometry().isMultipart():
                        new_geom = QgsGeometry.fromWkt(new_f.geometry().asMultiPolygon().asWkt())
                    else:
                        new_geom = QgsGeometry.fromWkt(new_f.geometry().asPolygon().asWkt())
                elif new_f.geometry().type() == QgsWkbTypes.LineGeometry:  # Linestring
                    if new_f.geometry().isMultipart():
                        new_geom = QgsGeometry.fromWkt(new_f.geometry().asPolyline().asWkt())
                    else:
                        new_geom = QgsGeometry.fromWkt(new_f.geometry().asMultiPolyline().asWkt())
                else:
                    new_geom = QgsGeometry()
                new_f.setGeometry(new_geom)
        if len(new_f.attributes()) > len(input_layer.fields()):
            f = QgsFeature(input_layer.fields())
            f.setGeometry(new_f.geometry())
            f.setAttributes(new_f.attributes()[:len(input_layer.fields())])
            new_f = f
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

        active_layer.beginEditCommand(tr('In-place editing by %s') % alg.name())

        req = QgsFeatureRequest(QgsExpression(r"$id < 0"))
        req.setFlags(QgsFeatureRequest.NoGeometry)
        req.setSubsetOfAttributes([])

        # Checks whether the algorithm has a processFeature method
        if hasattr(alg, 'processFeature'):  # in-place feature editing
            # Make a clone or it will crash the second time the dialog
            # is opened and run
            alg = alg.create()
            alg.prepare(parameters, context, feedback)
            # Check again for compatibility after prepare
            if not alg.supportInPlaceEdit(active_layer):
                raise QgsProcessingException(tr("Selected algorithm and parameter configuration are not compatible with in-place modifications."))
            field_idxs = range(len(active_layer.fields()))
            feature_iterator = active_layer.getFeatures(QgsFeatureRequest(active_layer.selectedFeatureIds())) if parameters['INPUT'].selectedFeaturesOnly else active_layer.getFeatures()
            for f in feature_iterator:
                new_features = alg.processFeature(f, context, feedback)
                new_features = make_feature_compatible(new_features, active_layer)
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

            result_layer = QgsProcessingUtils.mapLayerFromString(results['OUTPUT'], context)
            # TODO: check if features have changed before delete/add cycle
            active_layer.deleteFeatures(active_layer.selectedFeatureIds())
            new_features = []
            for f in result_layer.getFeatures():
                new_features.append(make_feature_compatible([f], active_layer))

            # Get the new ids
            old_ids = set([f.id() for f in active_layer.getFeatures(req)])
            active_layer.addFeatures(new_features)
            new_ids = set([f.id() for f in active_layer.getFeatures(req)])
            new_feature_ids += list(new_ids - old_ids)

        active_layer.endEditCommand()

        if ok and new_feature_ids:
            active_layer.selectByIds(new_feature_ids)
        elif not ok:
            active_layer.rollback()

        return ok, results

    except QgsProcessingException as e:
        active_layer.endEditCommand()
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
