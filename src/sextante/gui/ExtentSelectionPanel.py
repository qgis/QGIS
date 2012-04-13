from PyQt4 import QtGui, QtCore
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.gui.RectangleMapTool import RectangleMapTool
from sextante.core.QGisLayers import QGisLayers


class ExtentSelectionPanel(QtGui.QWidget):

    def __init__(self, default):
        super(ExtentSelectionPanel, self).__init__(None)
        self.setObjectName("ESPanel")
        self.horizontalLayout = QtGui.QHBoxLayout(self)
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.horizontalLayout.setObjectName("hLayout")
        self.text = QtGui.QLineEdit()
        self.text.setObjectName("label")
        self.text.setText(default)
        self.text.setSizePolicy(QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        self.horizontalLayout.addWidget(self.text)
        self.pushButton = QtGui.QPushButton()
        self.pushButton.setObjectName("pushButton")
        self.pushButton.setText("...")
        self.pushButton.clicked.connect(self.buttonPushed)
        self.horizontalLayout.addWidget(self.pushButton)
        self.setLayout(self.horizontalLayout)
        canvas = QGisLayers.iface.mapCanvas()
        self.prevMapTool = canvas.mapTool()
        self.tool = RectangleMapTool(canvas)
        self.connect(self.tool, SIGNAL("rectangleCreated()"), self.fillCoords)

    def buttonPushed(self):
        canvas = QGisLayers.iface.mapCanvas()
        canvas.setMapTool(self.tool)

    def fillCoords(self):
        self.text.setText(str(self.tool.rectangle()))
        canvas = QGisLayers.iface.mapCanvas()
        canvas.setMapTool(self.prevMapTool)

    def getValue(self):
        return str(self.text.text())
