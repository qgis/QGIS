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
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.modeler.ModelerAlgorithm import  AlgorithmAndParameter
from sextante.modeler.ModelerGraphicItem import ModelerGraphicItem
from sextante.modeler.ModelerArrowItem import ModelerArrowItem
from sextante.parameters.ParameterMultipleInput import ParameterMultipleInput

class ModelerScene(QtGui.QGraphicsScene):

    def __init__(self, parent=None):
        super(ModelerScene, self).__init__(parent)
        self.paramItems = []
        self.algItems = []
        self.setItemIndexMethod(QtGui.QGraphicsScene.NoIndex);

    def getParameterPositions(self):
        pos = []
        for item in self.paramItems:
            pos.append(item.pos())
        return pos

    def getAlgorithmPositions(self):
        pos = []
        for item in self.algItems:
            pos.append(item.pos())
        return pos

    def getLastParameterItem(self):
        return self.paramItems[-1]

    def getLastAlgorithmItem(self):
        if self.algItems:
            return self.algItems[-1]
        else:
            return None

    def getItemsFromAAP(self, aap, isMultiple):
        items = []
        start = int(aap.alg)
        if aap.alg == AlgorithmAndParameter.PARENT_MODEL_ALGORITHM:
            if isMultiple:
                multi = self.model.paramValues[aap.param]
                tokens = multi.split(";")
                for token in tokens:
                    aap = AlgorithmAndParameter(float(token.split("|")[0]), token.split("|")[1])
                    ret = self.getItemsFromAAP(aap, False)
                    if ret:
                        for item in ret:
                            items.append(item)
            else:
                iModelParam=0
                for modelparam in self.model.parameters:
                    if modelparam.name == aap.param:
                        items.append((self.paramItems[iModelParam], -1))
                        break
                    iModelParam+=1
        else:
            idx = 0
            for output in self.model.algs[start].outputs:
                if output.name == aap.param:
                    items.append((self.algItems[start], idx))
                    break
                idx += 1

        return items

    def paintModel(self, model):
        self.model = model
        i=0
        for param in model.parameters:
            item = ModelerGraphicItem(param, i, model)
            item.setFlag(QtGui.QGraphicsItem.ItemIsMovable, True)
            item.setFlag(QtGui.QGraphicsItem.ItemIsSelectable, True)
            self.addItem(item)
            item.setPos(model.paramPos[i])
            self.paramItems.append(item)
            i+=1
        #first we add the algs
        iAlg=0
        for alg in model.algs:
            item = ModelerGraphicItem(alg, iAlg, model)
            item.setFlag(QtGui.QGraphicsItem.ItemIsMovable, True)
            item.setFlag(QtGui.QGraphicsItem.ItemIsSelectable, True)
            self.addItem(item)
            item.setPos(model.algPos[iAlg])
            self.algItems.append(item)
            iAlg+=1
        #and then the arrows
        iAlg=0
        for alg in model.algs:
            params = model.algParameters[iAlg]
            idx = 0
            for parameter in alg.parameters:
                param = params[parameter.name]
                if param:
                    sourceItems = self.getItemsFromAAP(param, isinstance(alg.getParameterFromName(parameter.name), ParameterMultipleInput))
                    for sourceItem in sourceItems:
                        arrow = ModelerArrowItem(sourceItem[0], sourceItem[1], self.algItems[iAlg], idx)
                        self.addItem(arrow)
                idx += 1
            for depend in model.dependencies[iAlg]:
                arrow = ModelerArrowItem(self.algItems[depend], -1, self.algItems[iAlg], -1)
                self.addItem(arrow)
            iAlg+=1

    def mousePressEvent(self, mouseEvent):
        if (mouseEvent.button() != QtCore.Qt.LeftButton):
            return

        super(ModelerScene, self).mousePressEvent(mouseEvent)


