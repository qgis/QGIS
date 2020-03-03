# -*- coding: utf-8 -*-

"""
***************************************************************************
    ModelerScene.py
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

from qgis.PyQt.QtCore import QPointF, Qt
from qgis.core import (QgsProcessingParameterDefinition,
                       QgsProcessingModelChildParameterSource,
                       QgsExpression)
from qgis.gui import (
    QgsModelGraphicsScene,
    QgsModelArrowItem
)
from processing.modeler.ModelerGraphicItem import (
    ModelerInputGraphicItem,
    ModelerOutputGraphicItem,
    ModelerChildAlgorithmGraphicItem
)
from processing.tools.dataobjects import createContext


class ModelerScene(QgsModelGraphicsScene):

    def __init__(self, parent=None):
        super().__init__(parent)
        self.paramItems = {}
        self.algItems = {}
        self.outputItems = {}

    def getItemsFromParamValue(self, value, child_id, context):
        items = []
        if isinstance(value, list):
            for v in value:
                items.extend(self.getItemsFromParamValue(v, child_id, context))
        elif isinstance(value, QgsProcessingModelChildParameterSource):
            if value.source() == QgsProcessingModelChildParameterSource.ModelParameter:
                items.append((self.paramItems[value.parameterName()], None, 0))
            elif value.source() == QgsProcessingModelChildParameterSource.ChildOutput:
                outputs = self.model.childAlgorithm(value.outputChildId()).algorithm().outputDefinitions()
                for i, out in enumerate(outputs):
                    if out.name() == value.outputName():
                        break
                if value.outputChildId() in self.algItems:
                    items.append((self.algItems[value.outputChildId()], Qt.BottomEdge, i))
            elif value.source() == QgsProcessingModelChildParameterSource.Expression:
                variables = self.model.variablesForChildAlgorithm(child_id, context)
                exp = QgsExpression(value.expression())
                for v in exp.referencedVariables():
                    if v in variables:
                        items.extend(self.getItemsFromParamValue(variables[v].source, child_id, context))
        return items

    def paintModel(self, model):
        self.model = model
        context = createContext()
        # Inputs
        for inp in list(model.parameterComponents().values()):
            item = ModelerInputGraphicItem(inp.clone(), model)
            self.addItem(item)
            item.setPos(inp.position().x(), inp.position().y())
            self.paramItems[inp.parameterName()] = item

            item.requestModelRepaint.connect(self.rebuildRequired)
            item.changed.connect(self.componentChanged)

        # Input dependency arrows
        for input_name in list(model.parameterComponents().keys()):
            parameter_def = model.parameterDefinition(input_name)
            for parent_name in parameter_def.dependsOnOtherParameters():
                if input_name in self.paramItems and parent_name in self.paramItems:
                    input_item = self.paramItems[input_name]
                    parent_item = self.paramItems[parent_name]
                    arrow = QgsModelArrowItem(parent_item, input_item)
                    arrow.setPenStyle(Qt.DotLine)
                    self.addItem(arrow)

        # We add the algs
        for alg in list(model.childAlgorithms().values()):
            item = ModelerChildAlgorithmGraphicItem(alg.clone(), model)
            self.addItem(item)
            item.setPos(alg.position().x(), alg.position().y())
            self.algItems[alg.childId()] = item

            item.requestModelRepaint.connect(self.rebuildRequired)
            item.changed.connect(self.componentChanged)

        # And then the arrows

        for alg in list(model.childAlgorithms().values()):
            idx = 0
            for parameter in alg.algorithm().parameterDefinitions():
                if not parameter.isDestination() and not parameter.flags() & QgsProcessingParameterDefinition.FlagHidden:
                    if parameter.name() in alg.parameterSources():
                        sources = alg.parameterSources()[parameter.name()]
                    else:
                        sources = []
                    for source in sources:
                        sourceItems = self.getItemsFromParamValue(source, alg.childId(), context)
                        for sourceItem, sourceEdge, sourceIdx in sourceItems:
                            if sourceEdge is None:
                                arrow = QgsModelArrowItem(sourceItem, self.algItems[alg.childId()], Qt.TopEdge, idx)
                            else:
                                arrow = QgsModelArrowItem(sourceItem, sourceEdge, sourceIdx, self.algItems[alg.childId()], Qt.TopEdge, idx)
                            self.addItem(arrow)
                        idx += 1
            for depend in alg.dependencies():
                arrow = QgsModelArrowItem(self.algItems[depend], self.algItems[alg.childId()])
                self.addItem(arrow)

        # And finally the outputs
        for alg in list(model.childAlgorithms().values()):
            outputs = alg.modelOutputs()
            outputItems = {}

            for key, out in outputs.items():
                if out is not None:
                    item = ModelerOutputGraphicItem(out.clone(), model)
                    item.requestModelRepaint.connect(self.rebuildRequired)
                    item.changed.connect(self.componentChanged)

                    self.addItem(item)
                    pos = out.position()

                    # find the actual index of the linked output from the child algorithm it comes from
                    source_child_alg_outputs = alg.algorithm().outputDefinitions()
                    idx = -1
                    for i, child_alg_output in enumerate(source_child_alg_outputs):
                        if child_alg_output.name() == out.childOutputName():
                            idx = i
                            break

                    if pos is None:
                        pos = (alg.position() + QPointF(alg.size().width(), 0)
                               + self.algItems[alg.childId()].linkPoint(Qt.BottomEdge, idx))
                    item.setPos(pos)
                    outputItems[key] = item

                    arrow = QgsModelArrowItem(self.algItems[alg.childId()], Qt.BottomEdge, idx, item)
                    self.addItem(arrow)
                else:
                    outputItems[key] = None
            self.outputItems[alg.childId()] = outputItems
