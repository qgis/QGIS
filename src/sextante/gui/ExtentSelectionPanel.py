from qgis.core import *
from PyQt4 import QtGui, QtCore
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.gui.RectangleMapTool import RectangleMapTool
from sextante.core.QGisLayers import QGisLayers
from sextante.parameters.ParameterRaster import ParameterRaster
from sextante.parameters.ParameterVector import ParameterVector
from sextante.parameters.ParameterMultipleInput import ParameterMultipleInput


class ExtentSelectionPanel(QtGui.QWidget):

    def __init__(self, dialog, alg, default):
        super(ExtentSelectionPanel, self).__init__(None)
        self.dialog = dialog
        self.params = alg.parameters
        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.text = QtGui.QLineEdit()
        #self.text.setText(default)
        self.text.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        if self.canUseAutoExtent():
            if hasattr(self.text, 'setPlaceholderText'):
                self.text.setPlaceholderText("[Leave blank to use min covering extent]")
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

    def canUseAutoExtent(self):
        for param in self.params:
            if isinstance(param, (ParameterRaster, ParameterVector)):
                return True
            if isinstance(param, ParameterMultipleInput):
                return True

        return False

    def buttonPushed(self):
        popupmenu = QMenu()
        useLayerExtentAction = QtGui.QAction("Use layer/canvas extent", self.pushButton)
        useLayerExtentAction.triggered.connect(self.useLayerExtent)
        popupmenu.addAction(useLayerExtentAction)
        selectOnCanvasAction = QtGui.QAction("Select extent on canvas", self.pushButton)
        selectOnCanvasAction.triggered.connect(self.selectOnCanvas)
        popupmenu.addAction(selectOnCanvasAction)
        if self.canUseAutoExtent():
            useMincoveringExtentAction = QtGui.QAction("Use min convering extent from input layers", self.pushButton)
            useMincoveringExtentAction.triggered.connect(self.useMinCoveringExtent)
            popupmenu.addAction(useMincoveringExtentAction)

        popupmenu.exec_(QtGui.QCursor.pos())

    def useMinCoveringExtent(self):
        self.text.setText("");

    def getMinCoveringExtent(self):
        first = True
        found = False
        for param in self.params:
            if param.value:
                if isinstance(param, (ParameterRaster, ParameterVector)):
                    found = True
                    if isinstance(param.value, (QgsRasterLayer, QgsVectorLayer)):
                        layer = param.value
                    else:
                        layer = QGisLayers.getObjectFromUri(param.value)
                    self.addToRegion(layer, first)
                    first = False
                elif isinstance(param, ParameterMultipleInput):
                    found = True
                    layers = param.value.split(";")
                    for layername in layers:
                        layer = QGisLayers.getObjectFromUri(layername, first)
                        self.addToRegion(layer, first)
                        first = False
        if found:
            return str(self.xmin) + "," + str(self.xmax) + "," + str(self.ymin) + "," + str(self.ymax)
        else:
            return None



    def addToRegion(self, layer, first):
        if first:
            self.xmin = layer.extent().xMinimum()
            self.xmax = layer.extent().xMaximum()
            self.ymin = layer.extent().yMinimum()
            self.ymax = layer.extent().yMaximum()
        else:
            self.xmin = min(self.xmin, layer.extent().xMinimum())
            self.xmax = max(self.xmax, layer.extent().xMaximum())
            self.ymin = min(self.ymin, layer.extent().yMinimum())
            self.ymax = max(self.ymax, layer.extent().yMaximum())

    def useLayerExtent(self):
        CANVAS_KEY = QtCore.QString("Use canvas extent")
        extentsDict = {}
        extentsDict[CANVAS_KEY] = QGisLayers.iface.mapCanvas().extent()
        extents = [CANVAS_KEY]
        layers = QGisLayers.getAllLayers()
        for layer in layers:
            extents.append(layer.name())
            extentsDict[layer.name()] = layer.extent()
        item, ok = QtGui.QInputDialog.getItem(self, "Select extent", "Use extent from", extents, False)
        if ok:
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
        if str(self.text.text()).strip() != "":
            return str(self.text.text())
        else:
            return self.getMinCoveringExtent()
