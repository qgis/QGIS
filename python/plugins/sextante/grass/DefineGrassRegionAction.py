import os
from PyQt4 import QtGui, QtCore
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.gui.ToolboxAction import ToolboxAction
from sextante.core.QGisLayers import QGisLayers
from sextante.gui.RectangleMapTool import RectangleMapTool
from sextante.core.SextanteConfig import SextanteConfig
from sextante.grass.GrassUtils import GrassUtils

class DefineGrassRegionAction(ToolboxAction):

    def __init__(self):
        self.name="Define GRASS region on canvas"
        self.group="Tools"
        canvas = QGisLayers.iface.mapCanvas()
        self.prevMapTool = canvas.mapTool()
        self.tool = RectangleMapTool(canvas)
        QtCore.QObject.connect(self.tool, SIGNAL("rectangleCreated()"), self.fillCoords)

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/grass.png")

    def execute(self):
        QtGui.QMessageBox.information(None, "GRASS Region", "Click and drag onto map canvas to define GRASS region")
        canvas = QGisLayers.iface.mapCanvas()
        canvas.setMapTool(self.tool)

    def fillCoords(self):
        r = self.tool.rectangle()
        SextanteConfig.setSettingValue(GrassUtils.GRASS_REGION_XMIN, r.xMinimum())
        SextanteConfig.setSettingValue(GrassUtils.GRASS_REGION_YMIN, r.yMinimum())
        SextanteConfig.setSettingValue(GrassUtils.GRASS_REGION_XMAX, r.xMaximum())
        SextanteConfig.setSettingValue(GrassUtils.GRASS_REGION_YMAX, r.yMaximum())
        SextanteConfig.setSettingValue(GrassUtils.GRASS_AUTO_REGION, False)
        s = str(r.xMinimum()) + "," + str(r.xMaximum()) + "," + str(r.yMinimum()) + "," + str(r.yMaximum())
        self.tool.reset()
        canvas = QGisLayers.iface.mapCanvas()
        canvas.setMapTool(self.prevMapTool)
        QtGui.QMessageBox.information(None, "GRASS Region", "GRASS region set to:\n" + s + \
                                      "\nTo set the cellsize or set back the autoregion option,\ngo to the SEXTANTE configuration.")
