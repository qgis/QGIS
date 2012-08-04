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
                        items.append(self.paramItems[iModelParam])
                        break
                    iModelParam+=1
        else:
            items.append(self.algItems[start])

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
            for key in params.keys():
                param = params[key]
                if param:
                    sourceItems = self.getItemsFromAAP(param, isinstance(alg.getParameterFromName(key), ParameterMultipleInput))
                    for sourceItem in sourceItems:
                        arrow = ModelerArrowItem(sourceItem, self.algItems[iAlg])
                        self.addItem(arrow)
            iAlg+=1


    def mousePressEvent(self, mouseEvent):
        if (mouseEvent.button() != QtCore.Qt.LeftButton):
            return

        super(ModelerScene, self).mousePressEvent(mouseEvent)


