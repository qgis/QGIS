from PyQt4 import QtGui, QtCore
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.gui.RectangleMapTool import RectangleMapTool
from sextante.core.QGisLayers import QGisLayers


class ExtentSelectionPanel(QtGui.QWidget):

    def __init__(self, dialog, default):
        super(ExtentSelectionPanel, self).__init__(None)
        self.dialog = dialog
        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.text = QtGui.QLineEdit()
        self.text.setText(default)
        self.text.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        self.horizontalLayout.addWidget(self.text)
        self.pushButton = QtGui.QPushButton()
        self.pushButton.setText("...")
        self.pushButton.clicked.connect(self.buttonPushed)
        self.horizontalLayout.addWidget(self.pushButton)
        self.setLayout(self.horizontalLayout)
        canvas = QGisLayers.iface.mapCanvas()
        self.prevMapTool = canvas.mapTool()
        self.tool = RectangleMapTool(canvas)
        self.connect(self.tool, SIGNAL("rectangleCreated()"), self.fillCoords)

    def buttonPushed(self):
        popupmenu = QMenu()
        useLayerExtentAction = QtGui.QAction("Use layer/canvas extent", self.pushButton)
        useLayerExtentAction.triggered.connect(self.useLayerExtent)
        popupmenu.addAction(useLayerExtentAction)
        selectOnCanvasAction = QtGui.QAction("Select extent on canvas", self.pushButton)
        selectOnCanvasAction.triggered.connect(self.selectOnCanvas)
        popupmenu.addAction(selectOnCanvasAction)
        popupmenu.exec_(QtGui.QCursor.pos())

    def useLayerExtent(self):
        extentsDict = {}
        extentsDict["[Canvas]"] = QGisLayers.iface.mapCanvas().extent()
        extents = ["[Canvas]"]
        layers = QGisLayers.getAllLayers()
        for layer in layers:
            extents.append(layer.name())
            extentsDict[str(layer.name())] = layer.extent()
        item, ok = QtGui.QInputDialog.getItem(self, "Select extent", "Use extent from", extents, False)
        if ok:
            item = str(item)
            self.setValueFromRect(extentsDict[item])

    def selectOnCanvas(self):
        canvas = QGisLayers.iface.mapCanvas()
        canvas.setMapTool(self.tool)
        self.dialog.showMinimized()

    def fillCoords(self):
        r = self.tool.rectangle()
        self.setValueFromRect(r)

    def setValueFromRect(self,r):
        s = str(r.xMinimum()) + "," + str(r.xMaximum()) + "," + str(r.yMinimum()) + "," + str(r.yMaximum())
        self.text.setText(s)
        self.tool.reset()
        canvas = QGisLayers.iface.mapCanvas()
        canvas.setMapTool(self.prevMapTool)
        self.dialog.showNormal()
        self.dialog.raise_()
        self.dialog.activateWindow()


    def getValue(self):
        return str(self.text.text())
