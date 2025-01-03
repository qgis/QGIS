"""
***************************************************************************
    AlgorithmLocatorFilter.py
    -------------------------
    Date                 : May 2017
    Copyright            : (C) 2017 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Nyall Dawson"
__date__ = "May 2017"
__copyright__ = "(C) 2017, Nyall Dawson"

from qgis.core import (
    QgsApplication,
    QgsProcessingAlgorithm,
    QgsProcessingFeatureBasedAlgorithm,
    QgsLocatorFilter,
    QgsLocatorResult,
    QgsProcessing,
    QgsWkbTypes,
    QgsMapLayerType,
    QgsFields,
    QgsStringUtils,
)
from processing.gui.MessageBarProgress import MessageBarProgress
from processing.gui.MessageDialog import MessageDialog
from processing.gui.AlgorithmDialog import AlgorithmDialog
from processing.gui.AlgorithmExecutor import execute_in_place
from qgis.utils import iface
from processing.core.ProcessingConfig import ProcessingConfig


class AlgorithmLocatorFilter(QgsLocatorFilter):

    def __init__(self, parent=None):
        super().__init__(parent)

    def clone(self):
        return AlgorithmLocatorFilter()

    def name(self):
        return "processing_alg"

    def displayName(self):
        return self.tr("Processing Algorithms")

    def priority(self):
        return QgsLocatorFilter.Priority.Low

    def prefix(self):
        return "a"

    def flags(self):
        return QgsLocatorFilter.Flag.FlagFast

    def fetchResults(self, string, context, feedback):
        # collect results in main thread, since this method is inexpensive and
        # accessing the processing registry is not thread safe
        for a in QgsApplication.processingRegistry().algorithms():
            if a.flags() & QgsProcessingAlgorithm.Flag.FlagHideFromToolbox:
                continue
            if (
                not ProcessingConfig.getSetting(
                    ProcessingConfig.SHOW_ALGORITHMS_KNOWN_ISSUES
                )
                and a.flags() & QgsProcessingAlgorithm.Flag.FlagKnownIssues
            ):
                continue

            result = QgsLocatorResult()
            result.filter = self
            result.displayString = a.displayName()
            result.icon = a.icon()
            result.userData = a.id()
            result.score = 0

            if context.usingPrefix and not string:
                self.resultFetched.emit(result)

            if not string:
                return

            string = string.lower()
            tagScore = 0
            if a.provider():
                tags = [*a.tags(), a.provider().name()]
            else:
                tags = a.tags()

            if a.group():
                tags.append(a.group())

            for t in tags:
                if string in t.lower():
                    tagScore = 1
                    break

            result.score = (
                QgsStringUtils.fuzzyScore(result.displayString, string) * 0.5
                + tagScore * 0.5
            )

            if result.score > 0:
                self.resultFetched.emit(result)

    def triggerResult(self, result):
        alg = QgsApplication.processingRegistry().createAlgorithmById(result.userData)
        if alg:
            ok, message = alg.canExecute()
            if not ok:
                dlg = MessageDialog()
                dlg.setTitle(self.tr("Missing dependency"))
                dlg.setMessage(message)
                dlg.exec()
                return
            dlg = alg.createCustomParametersWidget(parent=iface.mainWindow())
            if not dlg:
                dlg = AlgorithmDialog(alg, parent=iface.mainWindow())
            canvas = iface.mapCanvas()
            prevMapTool = canvas.mapTool()
            dlg.show()
            dlg.exec()
            if canvas.mapTool() != prevMapTool:
                try:
                    canvas.mapTool().reset()
                except:
                    pass
                try:
                    canvas.setMapTool(prevMapTool)
                except RuntimeError:
                    pass


class InPlaceAlgorithmLocatorFilter(QgsLocatorFilter):

    def __init__(self, parent=None):
        super().__init__(parent)

    def clone(self):
        return InPlaceAlgorithmLocatorFilter()

    def name(self):
        return "edit_features"

    def displayName(self):
        return self.tr("Edit Selected Features")

    def priority(self):
        return QgsLocatorFilter.Priority.Low

    def prefix(self):
        return "ef"

    def flags(self):
        return QgsLocatorFilter.Flag.FlagFast

    def fetchResults(self, string, context, feedback):
        # collect results in main thread, since this method is inexpensive and
        # accessing the processing registry/current layer is not thread safe

        if (
            iface.activeLayer() is None
            or iface.activeLayer().type() != QgsMapLayerType.VectorLayer
        ):
            return

        for a in QgsApplication.processingRegistry().algorithms():
            if not a.flags() & QgsProcessingAlgorithm.Flag.FlagSupportsInPlaceEdits:
                continue

            if not a.supportInPlaceEdit(iface.activeLayer()):
                continue

            result = QgsLocatorResult()
            result.filter = self
            result.displayString = a.displayName()
            result.icon = a.icon()
            result.userData = a.id()
            result.score = 0

            if context.usingPrefix and not string:
                self.resultFetched.emit(result)

            if not string:
                return

            string = string.lower()
            tagScore = 0
            if a.provider():
                tags = [*a.tags(), a.provider().name()]
            else:
                tags = a.tags()

            if a.group():
                tags.append(a.group())

            for t in tags:
                if string in t.lower():
                    tagScore = 1
                    break

            result.score = (
                QgsStringUtils.fuzzyScore(result.displayString, string) * 0.5
                + tagScore * 0.5
            )

            if result.score > 0:
                self.resultFetched.emit(result)

    def triggerResult(self, result):
        config = {"IN_PLACE": True}
        alg = QgsApplication.processingRegistry().createAlgorithmById(
            result.userData, config
        )
        if alg:
            ok, message = alg.canExecute()
            if not ok:
                dlg = MessageDialog()
                dlg.setTitle(self.tr("Missing dependency"))
                dlg.setMessage(message)
                dlg.exec()
                return

            in_place_input_parameter_name = "INPUT"
            if hasattr(alg, "inputParameterName"):
                in_place_input_parameter_name = alg.inputParameterName()

            if [
                d
                for d in alg.parameterDefinitions()
                if d.name() not in (in_place_input_parameter_name, "OUTPUT")
            ]:
                dlg = alg.createCustomParametersWidget(parent=iface.mainWindow())
                if not dlg:
                    dlg = AlgorithmDialog(alg, True, parent=iface.mainWindow())
                canvas = iface.mapCanvas()
                prevMapTool = canvas.mapTool()
                dlg.show()
                dlg.exec()
                if canvas.mapTool() != prevMapTool:
                    try:
                        canvas.mapTool().reset()
                    except:
                        pass
                    canvas.setMapTool(prevMapTool)
            else:
                feedback = MessageBarProgress(algname=alg.displayName())
                parameters = {}
                execute_in_place(alg, parameters, feedback=feedback)
                feedback.close()
