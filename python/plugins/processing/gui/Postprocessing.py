# -*- coding: utf-8 -*-

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

import os
import traceback
from qgis.PyQt.QtWidgets import QApplication
from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (Qgis,
                       QgsProject,
                       QgsProcessingFeedback,
                       QgsProcessingUtils,
                       QgsMapLayerType,
                       QgsWkbTypes,
                       QgsMessageLog,
                       QgsProviderRegistry,
                       QgsExpressionContext,
                       QgsExpressionContextScope)

from processing.core.ProcessingConfig import ProcessingConfig
from processing.gui.RenderingStyles import RenderingStyles


def handleAlgorithmResults(alg, context, feedback=None, showResults=True, parameters={}):
    wrongLayers = []
    if feedback is None:
        feedback = QgsProcessingFeedback()
    feedback.setProgressText(QCoreApplication.translate('Postprocessing', 'Loading resulting layers'))
    i = 0

    for l, details in context.layersToLoadOnCompletion().items():
        if feedback.isCanceled():
            return False

        if len(context.layersToLoadOnCompletion()) > 2:
            # only show progress feedback if we're loading a bunch of layers
            feedback.setProgress(100 * i / float(len(context.layersToLoadOnCompletion())))
        try:
            layer = QgsProcessingUtils.mapLayerFromString(l, context, typeHint=details.layerTypeHint)
            if layer is not None:
                details.setOutputLayerName(layer)

                '''If running a model, the execution will arrive here when an algorithm that is part of
                that model is executed. We check if its output is a final otuput of the model, and
                adapt the output name accordingly'''
                outputName = details.outputName
                expcontext = QgsExpressionContext()
                scope = QgsExpressionContextScope()
                expcontext.appendScope(scope)
                for out in alg.outputDefinitions():
                    if out.name() not in parameters:
                        continue
                    outValue = parameters[out.name()]
                    if hasattr(outValue, "sink"):
                        outValue = outValue.sink.valueAsString(expcontext)[0]
                    else:
                        outValue = str(outValue)
                    if outValue == l:
                        outputName = out.name()
                        break
                style = None
                if outputName:
                    style = RenderingStyles.getStyle(alg.id(), outputName)
                if style is None:
                    if layer.type() == QgsMapLayerType.RasterLayer:
                        style = ProcessingConfig.getSetting(ProcessingConfig.RASTER_STYLE)
                    elif layer.type() == QgsMapLayerType.VectorLayer:
                        if layer.geometryType() == QgsWkbTypes.PointGeometry:
                            style = ProcessingConfig.getSetting(ProcessingConfig.VECTOR_POINT_STYLE)
                        elif layer.geometryType() == QgsWkbTypes.LineGeometry:
                            style = ProcessingConfig.getSetting(ProcessingConfig.VECTOR_LINE_STYLE)
                        else:
                            style = ProcessingConfig.getSetting(ProcessingConfig.VECTOR_POLYGON_STYLE)
                if style:
                    layer.loadNamedStyle(style)

                # Load layer to layer tree root or to a specific group
                mapLayer = context.temporaryLayerStore().takeMapLayer(layer)
                group_name = ProcessingConfig.getSetting(ProcessingConfig.RESULTS_GROUP_NAME)
                if group_name:
                    group = details.project.layerTreeRoot().findGroup(group_name)
                    if not group:
                        group = details.project.layerTreeRoot().insertGroup(0, group_name)

                    details.project.addMapLayer(mapLayer, False)  # Add to registry
                    group.insertLayer(0, mapLayer)
                else:
                    details.project.addMapLayer(mapLayer)

                if details.postProcessor():
                    details.postProcessor().postProcessLayer(layer, context, feedback)

            else:
                wrongLayers.append(str(l))
        except Exception:
            QgsMessageLog.logMessage(QCoreApplication.translate('Postprocessing',
                                                                "Error loading result layer:") + "\n" + traceback.format_exc(),
                                     'Processing', Qgis.Critical)
            wrongLayers.append(str(l))
        i += 1

    feedback.setProgress(100)

    if wrongLayers:
        msg = QCoreApplication.translate('Postprocessing', "The following layers were not correctly generated.")
        msg += "\n" + "\n".join(["• {}".format(lay) for lay in wrongLayers]) + '\n'
        msg += QCoreApplication.translate('Postprocessing',
                                          "You can check the 'Log Messages Panel' in QGIS main window to find more information about the execution of the algorithm.")
        feedback.reportError(msg)

    return len(wrongLayers) == 0
