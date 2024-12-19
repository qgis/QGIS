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

__author__ = "Victor Olaya"
__date__ = "August 2012"
__copyright__ = "(C) 2012, Victor Olaya"

import traceback
from typing import Dict, List, Optional, Tuple

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
    QgsLayerTreeGroup,
    QgsLayerTreeNode,
)
from qgis.utils import iface

from processing.core.ProcessingConfig import ProcessingConfig
from processing.gui.RenderingStyles import RenderingStyles


SORT_ORDER_CUSTOM_PROPERTY = "_processing_sort_order"


def determine_output_name(
    dest_id: str,
    details: QgsProcessingContext.LayerDetails,
    alg: QgsProcessingAlgorithm,
    context: QgsProcessingContext,
    parameters: dict,
) -> str:
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
            output_value = output_value.sink.valueAsString(context.expressionContext())[
                0
            ]
        else:
            output_value = str(output_value)
        if output_value == dest_id:
            return out.name()

    return details.outputName


def post_process_layer(
    output_name: str, layer: QgsMapLayer, alg: QgsProcessingAlgorithm
):
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
            if layer.geometryType() == QgsWkbTypes.GeometryType.PointGeometry:
                style = ProcessingConfig.getSetting(ProcessingConfig.VECTOR_POINT_STYLE)
            elif layer.geometryType() == QgsWkbTypes.GeometryType.LineGeometry:
                style = ProcessingConfig.getSetting(ProcessingConfig.VECTOR_LINE_STYLE)
            else:
                style = ProcessingConfig.getSetting(
                    ProcessingConfig.VECTOR_POLYGON_STYLE
                )
    if style:
        layer.loadNamedStyle(style)

    if layer.type() == Qgis.LayerType.PointCloud:
        try:
            from qgis._3d import QgsPointCloudLayer3DRenderer

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
                    "can't assign a 3d renderer to a layer.",
                )
            )


def create_layer_tree_layer(
    layer: QgsMapLayer, details: QgsProcessingContext.LayerDetails
) -> QgsLayerTreeLayer:
    """
    Applies post-processing steps to a QgsLayerTreeLayer created for
    an algorithm's output
    """
    layer_tree_layer = QgsLayerTreeLayer(layer)

    if (
        ProcessingConfig.getSetting(ProcessingConfig.VECTOR_FEATURE_COUNT)
        and layer.type() == Qgis.LayerType.Vector
    ):
        layer_tree_layer.setCustomProperty("showFeatureCount", True)

    if details.layerSortKey:
        layer_tree_layer.setCustomProperty(
            SORT_ORDER_CUSTOM_PROPERTY, details.layerSortKey
        )
    return layer_tree_layer


def get_layer_tree_results_group(
    details: QgsProcessingContext.LayerDetails, context: QgsProcessingContext
) -> Optional[QgsLayerTreeGroup]:
    """
    Returns the destination layer tree group to store results in, or None
    if there is no specific destination tree group associated with the layer
    """

    destination_project = details.project or context.project()

    results_group: Optional[QgsLayerTreeGroup] = None

    # if a specific results group is specified in Processing settings,
    # respect it (and create if necessary)
    results_group_name = ProcessingConfig.getSetting(
        ProcessingConfig.RESULTS_GROUP_NAME
    )
    if results_group_name:
        results_group = destination_project.layerTreeRoot().findGroup(
            results_group_name
        )
        if not results_group:
            results_group = destination_project.layerTreeRoot().insertGroup(
                0, results_group_name
            )
            results_group.setExpanded(True)

    # if this particular output layer has a specific output group assigned,
    # find or create it now
    if details.groupName:
        if results_group is None:
            results_group = destination_project.layerTreeRoot()

        group = results_group.findGroup(details.groupName)
        if not group:
            group = results_group.insertGroup(0, details.groupName)
            group.setExpanded(True)
    else:
        group = results_group

    return group


def handleAlgorithmResults(
    alg: QgsProcessingAlgorithm,
    context: QgsProcessingContext,
    feedback: Optional[QgsProcessingFeedback] = None,
    parameters: Optional[dict] = None,
):
    if not parameters:
        parameters = {}
    if feedback is None:
        feedback = QgsProcessingFeedback()
    wrong_layers = []

    feedback.setProgressText(
        QCoreApplication.translate("Postprocessing", "Loading resulting layers")
    )
    i = 0

    added_layers: list[tuple[Optional[QgsLayerTreeGroup], QgsLayerTreeLayer]] = []
    layers_to_post_process: list[
        tuple[QgsMapLayer, QgsProcessingContext.LayerDetails]
    ] = []

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
                dest_id, context, typeHint=details.layerTypeHint
            )
            if layer is not None:
                details.setOutputLayerName(layer)

                output_name = determine_output_name(
                    dest_id, details, alg, context, parameters
                )
                post_process_layer(output_name, layer, alg)

                # Load layer to layer tree root or to a specific group
                results_group = get_layer_tree_results_group(details, context)

                # note here that we may not retrieve an owned layer -- eg if the
                # output layer already exists in the destination project
                owned_map_layer = context.temporaryLayerStore().takeMapLayer(layer)
                if owned_map_layer:
                    details.project.addMapLayer(owned_map_layer, False)

                    # we don't add the layer to the tree yet -- that's done
                    # later, after we've sorted all added layers
                    layer_tree_layer = create_layer_tree_layer(owned_map_layer, details)
                    added_layers.append((results_group, layer_tree_layer))

                if details.postProcessor():
                    # we defer calling the postProcessor set in the context
                    # until the layer has been added to the project's layer
                    # tree, just in case the postProcessor contains logic
                    # relating to layer tree handling
                    layers_to_post_process.append((layer, details))

            else:
                wrong_layers.append(str(dest_id))
        except Exception:
            QgsMessageLog.logMessage(
                QCoreApplication.translate(
                    "Postprocessing", "Error loading result layer:"
                )
                + "\n"
                + traceback.format_exc(),
                "Processing",
                Qgis.MessageLevel.Critical,
            )
            wrong_layers.append(str(dest_id))
        i += 1

    # sort added layer tree layers
    sorted_layer_tree_layers = sorted(
        added_layers, key=lambda x: x[1].customProperty(SORT_ORDER_CUSTOM_PROPERTY, 0)
    )
    have_set_active_layer = False

    current_selected_node: Optional[QgsLayerTreeNode] = None
    if iface is not None:
        current_selected_node = iface.layerTreeView().currentNode()
        iface.layerTreeView().setUpdatesEnabled(False)

    for group, layer_node in sorted_layer_tree_layers:
        layer_node.removeCustomProperty(SORT_ORDER_CUSTOM_PROPERTY)
        if group is not None:
            group.insertChildNode(0, layer_node)
        else:
            # no destination group for this layer, so should be placed
            # above the current layer
            if isinstance(current_selected_node, QgsLayerTreeLayer):
                current_node_group = current_selected_node.parent()
                current_node_index = current_node_group.children().index(
                    current_selected_node
                )
                current_node_group.insertChildNode(current_node_index, layer_node)
            elif isinstance(current_selected_node, QgsLayerTreeGroup):
                current_selected_node.insertChildNode(0, layer_node)
            elif context.project():
                context.project().layerTreeRoot().insertChildNode(0, layer_node)

        if not have_set_active_layer and iface is not None:
            iface.setActiveLayer(layer_node.layer())
            have_set_active_layer = True

    # all layers have been added to the layer tree, so safe to call
    # postProcessors now
    for layer, details in layers_to_post_process:
        details.postProcessor().postProcessLayer(layer, context, feedback)

    if iface is not None:
        iface.layerTreeView().setUpdatesEnabled(True)

    feedback.setProgress(100)

    if wrong_layers:
        msg = QCoreApplication.translate(
            "Postprocessing", "The following layers were not correctly generated."
        )
        msg += "\n" + "\n".join([f"• {lay}" for lay in wrong_layers]) + "\n"
        msg += QCoreApplication.translate(
            "Postprocessing",
            "You can check the 'Log Messages Panel' in QGIS main window "
            "to find more information about the execution of the algorithm.",
        )
        feedback.reportError(msg)

    return len(wrong_layers) == 0
