# -*- coding: utf-8 -*-

"""
***************************************************************************
    LidarToolsAlgorithmProvider.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
    ---------------------
    Date                 : April 2014
    Copyright            : (C) 2014 by Martin Isenburg
    Email                : martin near rapidlasso point com
    ---------------------
    Date                 : June 2014
    Copyright            : (C) 2014 by Agresta S. Coop
    Email                : iescamochero at agresta dot org
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

from lastools.LAStoolsUtils import LAStoolsUtils
from lastools.lasground import lasground
from lastools.lasheight import lasheight
from lastools.lasclassify import lasclassify
from lastools.laszip import laszip
from lastools.lasindex import lasindex
from lastools.lasclip import lasclip
from lastools.lasquery import lasquery
from lastools.lascolor import lascolor
from lastools.lasthin import lasthin
from lastools.lasnoise import lasnoise
from lastools.lassort import lassort
from lastools.lastile import lastile
from lastools.lasgrid import lasgrid
from lastools.lasview import lasview
from lastools.lasboundary import lasboundary
from lastools.lasinfo import lasinfo
from lastools.las2dem import las2dem
from lastools.blast2dem import blast2dem
from lastools.las2iso import las2iso
from lastools.las2tin import las2tin
from lastools.las2las_filter import las2las_filter
from lastools.las2las_project import las2las_project
from lastools.las2las_transform import las2las_transform
from lastools.blast2iso import blast2iso
from lastools.lasprecision import lasprecision
from lastools.lasvalidate import lasvalidate
from lastools.lasduplicate import lasduplicate
from lastools.las2txt import las2txt
from lastools.txt2las import txt2las
from lastools.las2shp import las2shp
from lastools.shp2las import shp2las
from lastools.lasmerge import lasmerge
from lastools.lassplit import lassplit
from lastools.lascanopy import lascanopy
from lastools.lasoverage import lasoverage
from lastools.lasoverlap import lasoverlap

from lastools.lastilePro import lastilePro
from lastools.lasgroundPro import lasgroundPro
from lastools.las2demPro import las2demPro

from lastools.flightlinesToDTMandDSM import flightlinesToDTMandDSM
from lastools.flightlinesToCHM import flightlinesToCHM
from lastools.flightlinesToSingleCHMpitFree import flightlinesToSingleCHMpitFree
from lastools.hugeFileClassify import hugeFileClassify
from lastools.hugeFileGroundClassify import hugeFileGroundClassify
from lastools.hugeFileNormalize import hugeFileNormalize

from fusion.OpenViewerAction import OpenViewerAction
from fusion.CanopyMaxima import CanopyMaxima
from fusion.CanopyModel import CanopyModel
from fusion.Catalog import Catalog
from fusion.ClipData import ClipData
from fusion.CloudMetrics import CloudMetrics
from fusion.Cover import Cover
from fusion.GridMetrics import GridMetrics
from fusion.GridSurfaceCreate import GridSurfaceCreate
from fusion.TinSurfaceCreate import TinSurfaceCreate
from fusion.Csv2Grid import Csv2Grid
from fusion.GroundFilter import GroundFilter
from fusion.MergeData import MergeData
from fusion.FilterData import FilterData
from fusion.FusionUtils import FusionUtils


class LidarToolsAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.activate = False
        self.algsList = []

        # LAStools for processing single files

        if isWindows():
            lastools = [
                lasground(), lasheight(), lasclassify(), lasclip(), lastile(),
                lascolor(), lasgrid(), las2dem(), blast2dem(), las2iso(), blast2iso(),
                lasview(), lasboundary(), lasinfo(), lasprecision(), las2tin(),
                lasvalidate(), lasduplicate(), las2txt(), txt2las(), laszip(),
                lasindex(), lasthin(), lassort(), lascanopy(), lasmerge(),
                las2shp(), shp2las(), lasnoise(), lassplit(), las2las_filter(),
                las2las_project(), las2las_transform(), lasoverage(), lasoverlap(),
                lasquery()
                ]
        else:
            lastools = [
                lasinfo(), lasprecision(), lasvalidate(), las2txt(), txt2las(),
                laszip(), lasindex(), lasmerge(), las2las_filter(), las2las_project(),
                las2las_transform(), lasquery()
                ]
        for alg in lastools:
            alg.group = 'LAStools'
        self.algsList.extend(lastools)

        # LAStools Production for processing folders of files

        if isWindows():
            lastoolsPro = [
                lastilePro(), lasgroundPro(), las2demPro()
                ]
        else:
            lastoolsPro = [
                ]
        for alg in lastoolsPro:
            alg.group = 'LAStools Production'
        self.algsList.extend(lastoolsPro)

        # some examples for LAStools Pipelines

        if isWindows():
            lastoolsPipe = [
                flightlinesToDTMandDSM(), flightlinesToCHM(), flightlinesToSingleCHMpitFree(), hugeFileClassify(),
                hugeFileGroundClassify(), hugeFileNormalize()
                ]
        else:
            lastoolsPipe = [
                ]
        for alg in lastoolsPipe:
            alg.group = 'LAStools Pipelines'
        self.algsList.extend(lastoolsPipe)

        # FUSION

        if isWindows():
            self.actions.append(OpenViewerAction())
            fusiontools = [
                Catalog(), CloudMetrics(), CanopyMaxima(), CanopyModel(), ClipData(),
                Csv2Grid(), Cover(), FilterData(), GridMetrics(), GroundFilter(),
                GridSurfaceCreate(), MergeData(), TinSurfaceCreate()
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
        ProcessingConfig.addSetting(Setting(self.getDescription(),
                LAStoolsUtils.WINE_FOLDER,
                'Wine folder', ''))

    def getName(self):
        return 'lidartools'

    def getDescription(self):
        return 'Tools for LiDAR data'

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + '/../../images/tool.png')

    def _loadAlgorithms(self):
        self.algs = self.algsList

    def getSupportedOutputTableExtensions(self):
        return ['csv']
