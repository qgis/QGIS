# -*- coding: utf-8 -*-

"""
***************************************************************************
    LidarToolsAlgorithmProvider.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
    ---------------------
    Date                 : September 2013
    Copyright            : (C) 2013 by Martin Isenburg
    Email                : martin near rapidlasso point com
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
from processing.core.ProcessingConfig import ProcessingConfig, Setting
from processing.tools.system import isWindows

from processing.lidar.lastools.LAStoolsUtils import LAStoolsUtils
from processing.lidar.lastools.lasground import lasground
from processing.lidar.lastools.lasheight import lasheight
from processing.lidar.lastools.lasclassify import lasclassify
from processing.lidar.lastools.laszip import laszip
from processing.lidar.lastools.lasindex import lasindex
from processing.lidar.lastools.lasclip import lasclip
from processing.lidar.lastools.lasthin import lasthin
from processing.lidar.lastools.lasnoise import lasnoise
from processing.lidar.lastools.lassort import lassort
from processing.lidar.lastools.lastile import lastile
from processing.lidar.lastools.lasgrid import lasgrid
from processing.lidar.lastools.lasview import lasview
from processing.lidar.lastools.lasboundary import lasboundary
from processing.lidar.lastools.lasinfo import lasinfo
from processing.lidar.lastools.las2dem import las2dem
from processing.lidar.lastools.blast2dem import blast2dem
from processing.lidar.lastools.las2iso import las2iso
from processing.lidar.lastools.las2las_filter import las2las_filter
from processing.lidar.lastools.las2las_transform import las2las_transform
from processing.lidar.lastools.blast2iso import blast2iso
from processing.lidar.lastools.lasprecision import lasprecision
from processing.lidar.lastools.lasvalidate import lasvalidate
from processing.lidar.lastools.lasduplicate import lasduplicate
from processing.lidar.lastools.las2txt import las2txt
from processing.lidar.lastools.txt2las import txt2las
from processing.lidar.lastools.las2shp import las2shp
from processing.lidar.lastools.shp2las import shp2las
from processing.lidar.lastools.lasmerge import lasmerge
from processing.lidar.lastools.lassplit import lassplit
from processing.lidar.lastools.lascanopy import lascanopy
from processing.lidar.lastools.lasoverage import lasoverage
from processing.lidar.lastools.lasoverlap import lasoverlap
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
            lastools = [
                lasground(), lasheight(), lasclassify(), lasclip(), lastile(),
                lasgrid(), las2dem(), blast2dem(), las2iso(), blast2iso(),
                lasview(), lasboundary(), lasinfo(), lasprecision(),
                lasvalidate(), lasduplicate(), las2txt(), txt2las(), laszip(),
                lasindex(), lasthin(), lassort(), lascanopy(), lasmerge(),
                las2shp(), shp2las(), lasnoise(), lassplit(), las2las_filter(),
                las2las_transform(), lasoverage(), lasoverlap()
                ]
        else:
            lastools = [
                lasinfo(), lasprecision(), lasvalidate(), las2txt(), txt2las(),
                laszip(), lasindex(), lasmerge(), las2las_filter(),
                las2las_transform()
                ]
        for alg in lastools:
            alg.group = 'LAStools'
        self.algsList.extend(lastools)

        if isWindows():
            self.actions.append(OpenViewerAction())
            fusiontools = [
                CloudMetrics(), CanopyMaxima(), CanopyModel(), ClipData(),
                Cover(), FilterData(), GridMetrics(), GroundFilter(),
                GridSurfaceCreate(), MergeData()
                ]
            for alg in fusiontools:
                alg.group = 'Fusion'
            self.algsList.extend(fusiontools)

    def initializeSettings(self):
        AlgorithmProvider.initializeSettings(self)
        ProcessingConfig.addSetting(Setting(self.getDescription(),
                LAStoolsUtils.LASTOOLS_FOLDER,
                'LAStools folder', LAStoolsUtils.LAStoolsPath()))
        ProcessingConfig.addSetting(Setting(self.getDescription(),
                FusionUtils.FUSION_FOLDER,
                'Fusion folder', FusionUtils.FusionPath()))

    def getName(self):
        return 'lidartools'

    def getDescription(self):
        return 'Tools for LiDAR data'

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + '/../images/tool.png')

    def _loadAlgorithms(self):
        self.algs = self.algsList

    def getSupportedOutputTableExtensions(self):
        return ['csv']
