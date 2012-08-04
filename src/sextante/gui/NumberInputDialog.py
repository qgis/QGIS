from qgis.core import *
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtCore, QtGui
from sextante.core.QGisLayers import QGisLayers

class NumberInputDialog(QtGui.QDialog):

    def __init__(self):
        QtGui.QDialog.__init__(self)
        self.setModal(True)
        self.setupUi()
        self.value = None

    def setupUi(self):
        self.resize(500, 350)
        self.buttonBox = QtGui.QDialogButtonBox()
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.label = QtGui.QLabel()
        self.label.setText("Enter expression in the text field.\nDouble click on elements in the tree to add their values to the expression.")
        self.tree = QtGui.QTreeWidget()
        self.tree.setHeaderHidden(True)
        self.fillTree()
        self.formulaText = QtGui.QLineEdit()
        if hasattr(self.formulaText, 'setPlaceholderText'):
            self.formulaText.setPlaceholderText("[Enter your formula here]")
        self.setWindowTitle("Enter number or expression")
        self.verticalLayout = QtGui.QVBoxLayout()
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.addWidget(self.label)
        self.verticalLayout.addWidget(self.tree)
        self.verticalLayout.addWidget(self.formulaText)
        self.verticalLayout.addWidget(self.buttonBox)
        self.setLayout(self.verticalLayout)
        self.tree.doubleClicked.connect(self.addValue)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL("accepted()"), self.okPressed)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL("rejected()"), self.cancelPressed)
        QtCore.QMetaObject.connectSlotsByName(self)

    def fillTree(self):
        layersItem = QtGui.QTreeWidgetItem()
        layersItem.setText(0, "Values from data layers extents")
        self.tree.addTopLevelItem(layersItem)
        layers = QGisLayers.getAllLayers()
        for layer in layers:
            layerItem = QtGui.QTreeWidgetItem()
            layerItem.setText(0, unicode(layer.name()))
            layerItem.addChild(TreeValueItem("Min X", layer.extent().xMinimum()))
            layerItem.addChild(TreeValueItem("Max X", layer.extent().xMaximum()))
            layerItem.addChild(TreeValueItem("Min Y", layer.extent().yMinimum()))
            layerItem.addChild(TreeValueItem("Max y", layer.extent().yMaximum()))
            if isinstance(layer, QgsRasterLayer):
                cellsize = (layer.extent().xMaximum() - layer.extent().xMinimum())/layer.width()
                layerItem.addChild(TreeValueItem("Cellsize", cellsize))
            layersItem.addChild(layerItem)
        layersItem = QtGui.QTreeWidgetItem()
        layersItem.setText(0, "Values from raster layers statistics")
        self.tree.addTopLevelItem(layersItem)
        layers = QGisLayers.getRasterLayers()
        for layer in layers:
            for i in range(layer.bandCount()):
                stats = layer.bandStatistics(i)
                layerItem = QtGui.QTreeWidgetItem()
                layerItem.setText(0, unicode(layer.name()))
                layerItem.addChild(TreeValueItem("Mean", stats.mean))
                layerItem.addChild(TreeValueItem("Std. deviation", stats.stdDev))
                layerItem.addChild(TreeValueItem("Max value", stats.maximumValue))
                layerItem.addChild(TreeValueItem("Min value", stats.minimumValue))
                layersItem.addChild(layerItem)

        canvasItem = QtGui.QTreeWidgetItem()
        canvasItem.setText(0, "Values from QGIS map canvas")
        self.tree.addTopLevelItem(canvasItem)
        extent = QGisLayers.iface.mapCanvas().extent()
        extentItem  = QtGui.QTreeWidgetItem()
        extentItem.setText(0, "Current extent")
        extentItem.addChild(TreeValueItem("Min X", extent.xMinimum()))
        extentItem.addChild(TreeValueItem("Max X", extent.xMaximum()))
        extentItem.addChild(TreeValueItem("Min Y", extent.yMinimum()))
        extentItem.addChild(TreeValueItem("Max y", extent.yMaximum()))
        canvasItem.addChild(extentItem)
        extent = QGisLayers.iface.mapCanvas().fullExtent()
        extentItem  = QtGui.QTreeWidgetItem()
        extentItem.setText(0, "Full extent of all layers in map canvas")
        extentItem.addChild(TreeValueItem("Min X", extent.xMinimum()))
        extentItem.addChild(TreeValueItem("Max X", extent.xMaximum()))
        extentItem.addChild(TreeValueItem("Min Y", extent.yMinimum()))
        extentItem.addChild(TreeValueItem("Max y", extent.yMaximum()))
        canvasItem.addChild(extentItem)

    def addValue(self):
        item = self.tree.currentItem()
        if isinstance(item, TreeValueItem):
            self.formulaText.setText(str(self.formulaText.text()) + " " + str(item.value))


    def okPressed(self):
        try:
            self.value = float(eval(str(self.formulaText.text())))
            self.close()
        except:
            QMessageBox.critical(self, "Wrong expression", "The expression entered is not correct")


    def cancelPressed(self):
        self.value = None
        self.close()

class TreeValueItem(QtGui.QTreeWidgetItem):
    def __init__(self, name, value):
        QTreeWidgetItem.__init__(self)
        self.value = value
        self.setText(0, name + ":" + str(value))