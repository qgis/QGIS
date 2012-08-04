from PyQt4 import QtCore, QtGui
from sextante.parameters.Parameter import Parameter
import os
from sextante.core.GeoAlgorithm import GeoAlgorithm
from sextante.modeler.ModelerParametersDialog import ModelerParametersDialog
from sextante.modeler.ModelerParameterDefinitionDialog import ModelerParameterDefinitionDialog

class ModelerGraphicItem(QtGui.QGraphicsItem):

    BOX_HEIGHT = 70
    BOX_WIDTH = 200

    def __init__(self, element, elementIndex, model):
        super(ModelerGraphicItem, self).__init__(None, None)
        self.model = model
        self.element = element
        self.elementIndex = elementIndex
        if isinstance(element, Parameter):
            icon = QtGui.QIcon(os.path.dirname(__file__) + "/../images/input.png")
            self.pixmap = icon.pixmap(20, 20, state=QtGui.QIcon.On)
            self.text = element.description
        else:
            self.text = element.name
            self.pixmap = element.getIcon().pixmap(20, 20, state=QtGui.QIcon.On)
        self.arrows = []
        self.setFlag(QtGui.QGraphicsItem.ItemIsMovable, True)
        self.setFlag(QtGui.QGraphicsItem.ItemIsSelectable, True)
        self.setZValue(1000)

    def addArrow(self, arrow):
        self.arrows.append(arrow)

    def boundingRect(self):
        rect = QtCore.QRectF(-(ModelerGraphicItem.BOX_WIDTH + 2)/2, -(ModelerGraphicItem.BOX_HEIGHT + 2)/2,
                             ModelerGraphicItem.BOX_WIDTH + 2, ModelerGraphicItem.BOX_HEIGHT + 2)
        return rect

    def mouseDoubleClickEvent(self, event):
        self.editElement()

    def contextMenuEvent(self, event):
        popupmenu = QtGui.QMenu()
        removeAction = popupmenu.addAction("Remove")
        removeAction.triggered.connect(self.removeElement)
        editAction = popupmenu.addAction("Edit")
        editAction.triggered.connect(self.editElement)
        if isinstance(self.element, GeoAlgorithm):
            if self.elementIndex in self.model.deactivated:
                removeAction = popupmenu.addAction("Activate")
                removeAction.triggered.connect(self.activateAlgorithm)
            else:
                deactivateAction = popupmenu.addAction("Deactivate")
                deactivateAction.triggered.connect(self.deactivateAlgorithm)
        popupmenu.exec_(event.screenPos())

    def deactivateAlgorithm(self):
        self.model.deactivateAlgorithm(self.elementIndex, True)

    def activateAlgorithm(self):
        if not self.model.activateAlgorithm(self.elementIndex, True):
            QtGui.QMessageBox.warning(None, "Could not activate Algorithm",
                                   "The selected algorithm depends on other currently non-active algorithms.\nActivate them them before trying to activate it.")

    def editElement(self):
        self.model.setPositions(self.scene().getParameterPositions(), self.scene().getAlgorithmPositions())
        if isinstance(self.element, Parameter):
            dlg = ModelerParameterDefinitionDialog(self.model, param = self.element)
            dlg.exec_()
            if dlg.param != None:
                self.model.updateParameter(self.elementIndex, dlg.param)
                self.element = dlg.param
                self.text = self.element.description
                self.update()

        else:
            dlg = self.element.getCustomModelerParametersDialog(self.model, self.elementIndex)
            if not dlg:
                dlg = ModelerParametersDialog(self.element, self.model, self.elementIndex)
            dlg.exec_()
            if dlg.params != None:
                self.model.updateAlgorithm(self.elementIndex, dlg.params, dlg.values, dlg.outputs)

    def removeElement(self):
        if isinstance(self.element, Parameter):
            if not self.model.removeParameter(self.elementIndex):
                QtGui.QMessageBox.warning(None, "Could not remove element",
                                   "Other elements depend on the selected one.\nRemove them before trying to remove it.")
        else:
            if not self.model.removeAlgorithm(self.elementIndex):
                QtGui.QMessageBox.warning(None, "Could not remove element",
                                   "Other elements depend on the selected one.\nRemove them before trying to remove it.")

    def getAdjustedText(self, text):
        font = QtGui.QFont("Verdana", 8)
        fm = QtGui.QFontMetricsF(font)
        w = fm.width(text)
        if w < self.BOX_WIDTH:
            return text

        text = text[0:-3] + "..."
        w = fm.width(text)
        while(w > self.BOX_WIDTH):
            text = text[0:-4] + "..."
            w = fm.width(text)
        return text


    def paint(self, painter, option, widget=None):

        rect = QtCore.QRectF(-(ModelerGraphicItem.BOX_WIDTH + 2)/2, -(ModelerGraphicItem.BOX_HEIGHT + 2)/2,
                             ModelerGraphicItem.BOX_WIDTH + 2, ModelerGraphicItem.BOX_HEIGHT + 2)
        painter.setPen(QtGui.QPen(QtCore.Qt.gray, 1))
        painter.setBrush(QtGui.QBrush(QtCore.Qt.white, QtCore.Qt.SolidPattern))
        painter.drawRect(rect)
        font = QtGui.QFont("Verdana", 8)
        painter.setFont(font)
        if self.isSelected():
            painter.setPen(QtGui.QPen(QtCore.Qt.blue))
        else:
            painter.setPen(QtGui.QPen(QtCore.Qt.black))
        fm = QtGui.QFontMetricsF(font)
        text = self.getAdjustedText(self.text)
        w = fm.width(QtCore.QString(text))
        h = fm.height()
        pt = QtCore.QPointF(-w/2, h/2)
        painter.drawText(pt, text)
        if isinstance(self.element, GeoAlgorithm):
            if self.elementIndex in self.model.deactivated:
                painter.setPen(QtGui.QPen(QtCore.Qt.red))
                w = fm.width(QtCore.QString("[deactivated]"))
                pt = QtCore.QPointF(-w/2, h+h/2)
                painter.drawText(pt, "[deactivated]")
        painter.drawPixmap(-10 , -(ModelerGraphicItem.BOX_HEIGHT )/3,self.pixmap)


    def itemChange(self, change, value):
        if change == QtGui.QGraphicsItem.ItemPositionChange:
            for arrow in self.arrows:
                arrow.updatePosition()

        return value

    def polygon(self):
        pol = QtGui.QPolygonF([
                    QtCore.QPointF(-(ModelerGraphicItem.BOX_WIDTH + 2)/2, -(ModelerGraphicItem.BOX_HEIGHT + 2)/2),
                    QtCore.QPointF(-(ModelerGraphicItem.BOX_WIDTH + 2)/2, (ModelerGraphicItem.BOX_HEIGHT + 2)/2),
                    QtCore.QPointF((ModelerGraphicItem.BOX_WIDTH + 2)/2, (ModelerGraphicItem.BOX_HEIGHT + 2)/2),
                    QtCore.QPointF((ModelerGraphicItem.BOX_WIDTH + 2)/2, -(ModelerGraphicItem.BOX_HEIGHT + 2)/2),
                    QtCore.QPointF(-(ModelerGraphicItem.BOX_WIDTH + 2)/2, -(ModelerGraphicItem.BOX_HEIGHT + 2)/2)])
        return pol

