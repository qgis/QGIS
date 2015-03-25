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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from PyQt4 import QtCore, QtGui
from processing.modeler.ModelerGraphicItem import ModelerGraphicItem
from processing.modeler.ModelerArrowItem import ModelerArrowItem
from processing.modeler.ModelerAlgorithm import ValueFromInput, ValueFromOutput

class ModelerScene(QtGui.QGraphicsScene):

    def __init__(self, parent=None):
        super(ModelerScene, self).__init__(parent)
        self.paramItems = {}
        self.algItems = {}
        self.outputItems = {}
        self.setItemIndexMethod(QtGui.QGraphicsScene.NoIndex)

    def getParameterPositions(self):
        return {key : item.pos() for key,item in self.paramItems.iteritems()}

    def getAlgorithmPositions(self):
        return {key : item.pos() for key,item in self.algItems.iteritems()}

    def getOutputPositions(self):
        pos = {}
        for algName, outputs in self.outputItems.iteritems():
            outputPos = {}
            for (key, value) in outputs.iteritems():
                if value is not None:
                    outputPos[key] = value.pos()
                else:
                    outputPos[key] = None
            pos[algName] = outputPos
        return pos

    def getItemsFromParamValue(self, value):
        items = []
        if isinstance(value, list):
            for v in value:
                items.extend(self.getItemsFromParamValue(v))
        elif isinstance(value, ValueFromInput):
            items.append((self.paramItems[value.name], 0))
        elif isinstance(value, ValueFromOutput):
            outputs = self.model.algs[value.alg].algorithm.outputs
            for i, out in enumerate(outputs):
                if out.name == value.output:
                    break
            items.append((self.algItems[value.alg], i))
        return items

    def paintModel(self, model):
        self.model = model
        # Inputs
        for inp in model.inputs.values():
            item = ModelerGraphicItem(inp, model)
            item.setFlag(QtGui.QGraphicsItem.ItemIsMovable, True)
            item.setFlag(QtGui.QGraphicsItem.ItemIsSelectable, True)
            self.addItem(item)
            item.setPos(inp.pos.x(), inp.pos.y())
            self.paramItems[inp.param.name] = item

        # We add the algs
        for alg in model.algs.values():
            item = ModelerGraphicItem(alg, model)
            item.setFlag(QtGui.QGraphicsItem.ItemIsMovable, True)
            item.setFlag(QtGui.QGraphicsItem.ItemIsSelectable, True)
            self.addItem(item)
            item.setPos(alg.pos.x(), alg.pos.y())
            self.algItems[alg.name] = item

        # And then the arrows
        for alg in model.algs.values():
            idx = 0
            for parameter in alg.algorithm.parameters:
                if not parameter.hidden:
                    if parameter.name in alg.params:
                        value = alg.params[parameter.name]
                    else:
                        value = None
                    sourceItems = self.getItemsFromParamValue(value)
                    for sourceItem, sourceIdx in sourceItems:
                        arrow = ModelerArrowItem(sourceItem, sourceIdx, self.algItems[alg.name], idx)
                        self.addItem(arrow)
                    idx += 1
            for depend in alg.dependencies:
                arrow = ModelerArrowItem(self.algItems[depend], -1,
                        self.algItems[alg.name], -1)
                self.addItem(arrow)

        # And finally the outputs
        for alg in model.algs.values():
            outputs = alg.outputs
            outputItems = {}
            idx = 0
            for key in outputs:
                out = outputs[key]
                if out is not None:
                    item = ModelerGraphicItem(out, model)
                    item.setFlag(QtGui.QGraphicsItem.ItemIsMovable, True)
                    item.setFlag(QtGui.QGraphicsItem.ItemIsSelectable, True)
                    self.addItem(item)
                    pos = alg.outputs[key].pos
                    if pos is None:
                        pos = (alg.pos + QtCore.QPointF(ModelerGraphicItem.BOX_WIDTH, 0)
                            + self.algItems[alg.name].getLinkPointForOutput(idx))
                    item.setPos(pos)
                    outputItems[key] = item
                    arrow = ModelerArrowItem(self.algItems[alg.name], idx, item,
                            -1)
                    self.addItem(arrow)
                    idx += 1
                else:
                    outputItems[key] = None
            self.outputItems[alg.name] = outputItems

    def mousePressEvent(self, mouseEvent):
        if mouseEvent.button() != QtCore.Qt.LeftButton:
            return
        super(ModelerScene, self).mousePressEvent(mouseEvent)
