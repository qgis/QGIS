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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import traceback
from qgis.PyQt.QtWidgets import QApplication
from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (QgsProject,
                       QgsProcessingFeedback,
                       QgsProcessingUtils,
                       QgsMapLayer,
                       QgsWkbTypes,
                       QgsMessageLog)

from processing.core.ProcessingConfig import ProcessingConfig
from processing.gui.RenderingStyles import RenderingStyles


def handleAlgorithmResults(alg, context, feedback=None, showResults=True):
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
            layer = QgsProcessingUtils.mapLayerFromString(l, context)
            if layer is not None:
                if not ProcessingConfig.getSetting(ProcessingConfig.USE_FILENAME_AS_LAYER_NAME):
                    layer.setName(details.name)

                style = None
                if details.outputName:
                    style = RenderingStyles.getStyle(alg.id(), details.outputName)
                if style is None:
                    if layer.type() == QgsMapLayer.RasterLayer:
                        style = ProcessingConfig.getSetting(ProcessingConfig.RASTER_STYLE)
                    else:
                        if layer.geometryType() == QgsWkbTypes.PointGeometry:
                            style = ProcessingConfig.getSetting(ProcessingConfig.VECTOR_POINT_STYLE)
                        elif layer.geometryType() == QgsWkbTypes.LineGeometry:
                            style = ProcessingConfig.getSetting(ProcessingConfig.VECTOR_LINE_STYLE)
                        else:
                            style = ProcessingConfig.getSetting(ProcessingConfig.VECTOR_POLYGON_STYLE)
                if style:
                    layer.loadNamedStyle(style)
                details.project.addMapLayer(context.temporaryLayerStore().takeMapLayer(layer))
        except Exception:
            QgsMessageLog.logMessage("Error loading result layer:\n" + traceback.format_exc(), 'Processing', QgsMessageLog.CRITICAL)
            wrongLayers.append(str(l))
        i += 1

    feedback.setProgress(100)

    if wrongLayers:
        msg = "The following layers were not correctly generated.<ul>"
        msg += "".join(["<li>%s</li>" % lay for lay in wrongLayers]) + "</ul>"
        msg += "You can check the log messages to find more information about the execution of the algorithm"
        feedback.reportError(msg)

    return len(wrongLayers) == 0
