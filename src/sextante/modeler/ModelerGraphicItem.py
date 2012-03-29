from PyQt4 import QtCore, QtGui
from sextante.parameters.Parameter import Parameter
import os

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

    def contextMenuEvent(self, event):
        popupmenu = QtGui.QMenu()
        removeAction = popupmenu.addAction("Remove")
        removeAction.triggered.connect(self.removeElement)
        popupmenu.exec_(event.screenPos())

    def removeElement(self):
        if isinstance(self.element, Parameter):
            if not self.model.removeParameter(self.elementIndex):
                QtGui.QMessageBox.warning(None, "Could not remove element",
                                   "Other elements depend on this one.\nRemove them before trying to remove it.")
        else:
            if not self.model.removeAlgorithm(self.elementIndex):
                QtGui.QMessageBox.warning(None, "Could not remove element",
                                   "Other elements depend on this one.\nRemove them before trying to remove it.")

    def getAdjustedText(self, text):
        return text


    def paint(self, painter, option, widget=None):
        rect = QtCore.QRectF(-(ModelerGraphicItem.BOX_WIDTH + 2)/2, -(ModelerGraphicItem.BOX_HEIGHT + 2)/2,
                             ModelerGraphicItem.BOX_WIDTH + 2, ModelerGraphicItem.BOX_HEIGHT + 2)
        painter.setPen(QtGui.QPen(QtCore.Qt.gray, 1))
        painter.setBrush(QtGui.QBrush(QtCore.Qt.white, QtCore.Qt.SolidPattern))
        painter.drawRect(rect)
        font = QtGui.QFont("Verdana", 8)
        painter.setFont(font)
        painter.setPen(QtGui.QPen(QtCore.Qt.black))
        fm = QtGui.QFontMetricsF(font)
        w = fm.width(QtCore.QString(self.getAdjustedText(self.text)))
        h = fm.height()
        pt = QtCore.QPointF(-w/2, h/2)
        painter.drawText(pt, self.text)
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

