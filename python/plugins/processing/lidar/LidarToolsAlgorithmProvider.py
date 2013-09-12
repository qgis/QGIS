# -*- coding: utf-8 -*-

"""
***************************************************************************
    LidarToolsAlgorithmProvider.py
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
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from processing.core.AlgorithmProvider import AlgorithmProvider
from processing.tools.system import *
from processing.lidar.lastools.LasToolsUtils import LasToolsUtils
from processing.core.ProcessingConfig import Setting, ProcessingConfig
from processing.lidar.lastools.las2dem import las2dem
from processing.lidar.lastools.las2iso import las2iso
from processing.lidar.lastools.las2shp import las2shp
from processing.lidar.lastools.lasboundary import lasboundary
from processing.lidar.lastools.lasgrid import lasgrid
from processing.lidar.lastools.lasground import lasground
from processing.lidar.lastools.lasclassify import lasclassify
from processing.lidar.lastools.lasclip import lasclip
from processing.lidar.lastools.lasheight import lasheight
from processing.lidar.lastools.lasinfo import lasinfo
from processing.lidar.lastools.lasprecision import lasprecision
from processing.lidar.lastools.lassplit import lassplit
from processing.lidar.fusion.OpenViewerAction import OpenViewerAction
from processing.lidar.fusion.CanopyMaxima import CanopyMaxima
from processing.lidar.fusion.CanopyModel import CanopyModel
from processing.lidar.fusion.ClipData import ClipData
from processing.lidar.fusion.CloudMetrics import CloudMetrics
from processing.lidar.fusion.Cover import Cover
from processing.lidar.fusion.GridMetrics import GridMetrics
from processing.lidar.fusion.GridSurfaceCreate import GridSurfaceCreate
from processing.lidar.fusion.GroundFilter import GroundFilter
from processing.lidar.fusion.MergeData import MergeData
from processing.lidar.fusion.FilterData import FilterData
from processing.lidar.fusion.FusionUtils import FusionUtils


class LidarToolsAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.activate = False
        self.algsList = []
        if isWindows():
            lastools = [las2shp(), lasboundary(), las2dem(), las2iso(), lasgrid(), lasground(),
                         lasinfo(), lasheight(), lasprecision(), lassplit(), lasclassify(), lasclip()]
        else:
            lastools = [lasinfo(), lasprecision()]
        for alg in lastools:
            alg.group = "LASTools"
        self.algsList.extend(lastools)

        if isWindows():
            self.actions.append(OpenViewerAction())
            fusiontools = [CloudMetrics(), CanopyMaxima(), CanopyModel(), ClipData(), Cover(), FilterData(),
                         GridMetrics(), GroundFilter(), GridSurfaceCreate(), MergeData()]
            for alg in fusiontools:
                alg.group = "Fusion"
            self.algsList.extend(fusiontools)

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)
        ProcessingConfig.addSetting(Setting(self.getDescription(), LasToolsUtils.LASTOOLS_FOLDER, "LASTools folder", LasToolsUtils.LasToolsPath()))
        ProcessingConfig.addSetting(Setting(self.getDescription(), FusionUtils.FUSION_FOLDER, "Fusion folder",
                                          FusionUtils.FusionPath()))

    def getName(self):
        return "lidartools"

    def getDescription(self):
        return "Tools for LiDAR data"

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + "/../images/tool.png")

    def _loadAlgorithms(self):
        self.algs = self.algsList

    def getSupportedOutputTableExtensions(self):
        return ["csv"]