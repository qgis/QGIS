# -*- coding: utf-8 -*-

"""
***************************************************************************
    DefineGrassRegionFromLayerAction.py
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

import os
from PyQt4 import QtGui, QtCore
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.gui.ToolboxAction import ToolboxAction
from sextante.core.QGisLayers import QGisLayers
from sextante.core.SextanteConfig import SextanteConfig
from sextante.grass.GrassUtils import GrassUtils

class DefineGrassRegionFromLayerAction(ToolboxAction):

    def __init__(self):
        self.name="Define GRASS region from layer"
        self.group="Tools"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/grass.png")

    def execute(self):
        layers = QGisLayers.getAllLayers();
        layersMap = dict([(layer.name(), layer) for layer in layers])
        layerNames = [layer.name() for layer in layers]
        item, ok = QtGui.QInputDialog.getItem(None, "Select a layer", "Layer selection", layerNames, editable=False)
        if ok:
            layer = layersMap[item]
            SextanteConfig.setSettingValue(GrassUtils.GRASS_REGION_XMIN, layer.extent().xMinimum())
            SextanteConfig.setSettingValue(GrassUtils.GRASS_REGION_YMIN, layer.extent().yMinimum())
            SextanteConfig.setSettingValue(GrassUtils.GRASS_REGION_XMAX, layer.extent().xMaximum())
            SextanteConfig.setSettingValue(GrassUtils.GRASS_REGION_YMAX, layer.extent().yMaximum())
            SextanteConfig.setSettingValue(GrassUtils.GRASS_AUTO_REGION, False)
            s = str(layer.extent().xMinimum()) + "," + str(layer.extent().xMaximum()) + "," + str(layer.extent().yMinimum()) + "," + str(layer.extent().yMaximum())
            QtGui.QMessageBox.information(None, "GRASS Region", "GRASS region set to:\n" + s + \
                                      "\nTo set the cellsize or set back the autoregion option,\ngo to the SEXTANTE configuration.")
