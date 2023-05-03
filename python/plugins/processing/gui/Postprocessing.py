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

from typing import (
    Dict,
    Optional
)
import traceback


from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (Qgis,
                       QgsProcessingFeedback,
                       QgsProcessingUtils,
                       QgsMapLayer,
                       QgsWkbTypes,
                       QgsMessageLog,
                       QgsProcessingContext,
                       QgsProcessingAlgorithm,
                       QgsLayerTreeLayer)

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
        if layer.type() == Qgis.MapLayerType.Raster:
            style = ProcessingConfig.getSetting(ProcessingConfig.RASTER_STYLE)
        elif layer.type() == Qgis.MapLayerType.Vector:
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
        if layer.type() == Qgis.MapLayerType.PointCloud:
            if layer.renderer3D() is None:
                # If the layer has no 3D renderer and syncing 3D to 2D
                # renderer is enabled, we create a renderer and set it up
                # with the 2D renderer
                if layer.sync3DRendererTo2DRenderer():
                    renderer3D = QgsPointCloudLayer3DRenderer()
                    renderer3D.convertFrom2DRenderer(layer.renderer())
                    layer.setRenderer3D(renderer3D)
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
            layer.type() == Qgis.MapLayerType.Vector:
        layer_tree_layer.setCustomProperty("showFeatureCount", True)


def handleAlgorithmResults(alg: QgsProcessingAlgorithm,
                           context: QgsProcessingContext,
                           feedback: Optional[QgsProcessingFeedback] = None,
                           showResults: bool = True,
                           parameters: Optional[Dict] = None):
    if not parameters:
        parameters = {}

    wrongLayers = []
    if feedback is None:
        feedback = QgsProcessingFeedback()
    feedback.setProgressText(QCoreApplication.translate('Postprocessing', 'Loading resulting layers'))
    i = 0

    for dest_id, details in context.layersToLoadOnCompletion().items():
        if feedback.isCanceled():
            return False

        if len(context.layersToLoadOnCompletion()) > 2:
            # only show progress feedback if we're loading a bunch of layers
            feedback.setProgress(100 * i / float(len(context.layersToLoadOnCompletion())))

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
                map_layer = context.temporaryLayerStore().takeMapLayer(layer)
                group_name = ProcessingConfig.getSetting(ProcessingConfig.RESULTS_GROUP_NAME)
                if group_name:
                    group = details.project.layerTreeRoot().findGroup(group_name)
                    if not group:
                        group = details.project.layerTreeRoot().insertGroup(0, group_name)

                    details.project.addMapLayer(map_layer, False)  # Add to registry
                    group.insertLayer(0, map_layer)
                else:
                    details.project.addMapLayer(map_layer)

                layer_tree_layer = details.project.layerTreeRoot().findLayer(
                    layer.id())
                if layer_tree_layer:
                    post_process_layer_tree_layer(layer_tree_layer)

                if details.postProcessor():
                    details.postProcessor().postProcessLayer(layer, context, feedback)

            else:
                wrongLayers.append(str(dest_id))
        except Exception:
            QgsMessageLog.logMessage(QCoreApplication.translate('Postprocessing',
                                                                "Error loading result layer:") + "\n" + traceback.format_exc(),
                                     'Processing', Qgis.Critical)
            wrongLayers.append(str(dest_id))
        i += 1

    feedback.setProgress(100)

    if wrongLayers:
        msg = QCoreApplication.translate('Postprocessing', "The following layers were not correctly generated.")
        msg += "\n" + "\n".join([f"• {lay}" for lay in wrongLayers]) + '\n'
        msg += QCoreApplication.translate('Postprocessing',
                                          "You can check the 'Log Messages Panel' in QGIS main window to find more information about the execution of the algorithm.")
        feedback.reportError(msg)

    return len(wrongLayers) == 0
