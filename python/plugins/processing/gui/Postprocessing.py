"""
***************************************************************************
    Postprocessing.py
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

import traceback
from typing import (
    Dict,
    Optional
)

from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (
    Qgis,
    QgsProcessingFeedback,
    QgsProcessingUtils,
    QgsMapLayer,
    QgsWkbTypes,
    QgsMessageLog,
    QgsProcessingContext,
    QgsProcessingAlgorithm,
    QgsLayerTreeLayer,
    QgsLayerTreeGroup
)

from processing.core.ProcessingConfig import ProcessingConfig
from processing.gui.RenderingStyles import RenderingStyles


def determine_output_name(dest_id: str,
                          details: QgsProcessingContext.LayerDetails,
                          alg: QgsProcessingAlgorithm,
                          context: QgsProcessingContext,
                          parameters: Dict) -> str:
    """
    If running a model, the execution will arrive here when an
    algorithm that is part of that model is executed. We check if
    its output is a final output of the model, and adapt the output
    name accordingly
    """
    for out in alg.outputDefinitions():
        if out.name() not in parameters:
            continue
        output_value = parameters[out.name()]
        if hasattr(output_value, "sink"):
            output_value = output_value.sink.valueAsString(
                context.expressionContext()
            )[0]
        else:
            output_value = str(output_value)
        if output_value == dest_id:
            return out.name()

    return details.outputName


def post_process_layer(output_name: str,
                       layer: QgsMapLayer,
                       alg: QgsProcessingAlgorithm):
    """
    Applies post-processing steps to a layer
    """
    style = None
    if output_name:
        style = RenderingStyles.getStyle(alg.id(), output_name)

    if style is None:
        if layer.type() == Qgis.LayerType.Raster:
            style = ProcessingConfig.getSetting(ProcessingConfig.RASTER_STYLE)
        elif layer.type() == Qgis.LayerType.Vector:
            if layer.geometryType() == QgsWkbTypes.PointGeometry:
                style = ProcessingConfig.getSetting(
                    ProcessingConfig.VECTOR_POINT_STYLE)
            elif layer.geometryType() == QgsWkbTypes.LineGeometry:
                style = ProcessingConfig.getSetting(
                    ProcessingConfig.VECTOR_LINE_STYLE)
            else:
                style = ProcessingConfig.getSetting(
                    ProcessingConfig.VECTOR_POLYGON_STYLE)
    if style:
        layer.loadNamedStyle(style)

    try:
        from qgis._3d import QgsPointCloudLayer3DRenderer
        if layer.type() == Qgis.LayerType.PointCloud:
            if layer.renderer3D() is None:
                # If the layer has no 3D renderer and syncing 3D to 2D
                # renderer is enabled, we create a renderer and set it up
                # with the 2D renderer
                if layer.sync3DRendererTo2DRenderer():
                    renderer_3d = QgsPointCloudLayer3DRenderer()
                    renderer_3d.convertFrom2DRenderer(layer.renderer())
                    layer.setRenderer3D(renderer_3d)
    except ImportError:
        QgsMessageLog.logMessage(
            QCoreApplication.translate(
                "Postprocessing",
                "3D library is not available, "
                "can't assign a 3d renderer to a layer."
            )
        )


def post_process_layer_tree_layer(layer_tree_layer: QgsLayerTreeLayer):
    """
    Applies post-processing steps to a QgsLayerTreeLayer created for
    an algorithm's output
    """
    layer = layer_tree_layer.layer()
    if ProcessingConfig.getSetting(ProcessingConfig.VECTOR_FEATURE_COUNT) and \
            layer.type() == Qgis.LayerType.Vector:
        layer_tree_layer.setCustomProperty("showFeatureCount", True)


def get_layer_tree_results_group(details: QgsProcessingContext.LayerDetails,
                                 context: QgsProcessingContext) \
        -> QgsLayerTreeGroup:
    """
    Returns the destination layer tree group to store results in
    """

    destination_project = details.project or context.project()

    # default to placing results in the top level of the layer tree
    results_group = details.project.layerTreeRoot()

    # if a specific results group is specified in Processing settings,
    # respect it (and create if necessary)
    results_group_name = ProcessingConfig.getSetting(
        ProcessingConfig.RESULTS_GROUP_NAME)
    if results_group_name:
        results_group = destination_project.layerTreeRoot().findGroup(
            results_group_name)
        if not results_group:
            results_group = destination_project.layerTreeRoot().insertGroup(
                0, results_group_name)
            results_group.setExpanded(True)

    # if this particular output layer has a specific output group assigned,
    # find or create it now
    if details.groupName:
        group = results_group.findGroup(details.groupName)
        if not group:
            group = results_group.insertGroup(
                0, details.groupName)
            group.setExpanded(True)
    else:
        group = results_group

    return group


def handleAlgorithmResults(alg: QgsProcessingAlgorithm,
                           context: QgsProcessingContext,
                           feedback: Optional[QgsProcessingFeedback] = None,
                           parameters: Optional[Dict] = None):
    if not parameters:
        parameters = {}
    if feedback is None:
        feedback = QgsProcessingFeedback()
    wrong_layers = []

    feedback.setProgressText(
        QCoreApplication.translate(
            'Postprocessing',
            'Loading resulting layers'
        )
    )
    i = 0

    for dest_id, details in context.layersToLoadOnCompletion().items():
        if feedback.isCanceled():
            return False

        if len(context.layersToLoadOnCompletion()) > 2:
            # only show progress feedback if we're loading a bunch of layers
            feedback.setProgress(
                100 * i / float(len(context.layersToLoadOnCompletion()))
            )

        try:
            layer = QgsProcessingUtils.mapLayerFromString(
                dest_id,
                context,
                typeHint=details.layerTypeHint
            )
            if layer is not None:
                details.setOutputLayerName(layer)

                output_name = determine_output_name(
                    dest_id, details, alg, context, parameters
                )
                post_process_layer(output_name, layer, alg)

                # Load layer to layer tree root or to a specific group
                results_group = get_layer_tree_results_group(details, context)

                map_layer = context.temporaryLayerStore().takeMapLayer(layer)
                details.project.addMapLayer(map_layer, False)
                layer_tree_layer = results_group.insertLayer(0, map_layer)
                if layer_tree_layer:
                    post_process_layer_tree_layer(layer_tree_layer)

                if details.postProcessor():
                    details.postProcessor().postProcessLayer(
                        layer,
                        context,
                        feedback)

            else:
                wrong_layers.append(str(dest_id))
        except Exception:
            QgsMessageLog.logMessage(
                QCoreApplication.translate(
                    'Postprocessing',
                    "Error loading result layer:"
                ) + "\n" + traceback.format_exc(),
                'Processing',
                Qgis.Critical)
            wrong_layers.append(str(dest_id))
        i += 1

    feedback.setProgress(100)

    if wrong_layers:
        msg = QCoreApplication.translate(
            'Postprocessing',
            "The following layers were not correctly generated."
        )
        msg += "\n" + "\n".join([f"• {lay}" for lay in wrong_layers]) + '\n'
        msg += QCoreApplication.translate(
            'Postprocessing',
            "You can check the 'Log Messages Panel' in QGIS main window "
            "to find more information about the execution of the algorithm.")
        feedback.reportError(msg)

    return len(wrong_layers) == 0
