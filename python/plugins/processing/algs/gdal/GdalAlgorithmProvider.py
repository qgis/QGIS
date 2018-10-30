# -*- coding: utf-8 -*-

"""
***************************************************************************
    GdalAlgorithmProvider.py
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

from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (QgsApplication,
                       QgsProcessingProvider)
from processing.core.ProcessingConfig import ProcessingConfig, Setting
from .GdalUtils import GdalUtils

from .AssignProjection import AssignProjection
from .aspect import aspect
from .buildvrt import buildvrt
from .ClipRasterByExtent import ClipRasterByExtent
from .ClipRasterByMask import ClipRasterByMask
from .ColorRelief import ColorRelief
from .contour import contour
from .fillnodata import fillnodata
from .gdalinfo import gdalinfo
from .gdal2tiles import gdal2tiles
from .gdal2xyz import gdal2xyz
from .gdaladdo import gdaladdo
from .gdalcalc import gdalcalc
from .gdaltindex import gdaltindex
from .GridAverage import GridAverage
from .GridDataMetrics import GridDataMetrics
from .GridInverseDistance import GridInverseDistance
from .GridInverseDistanceNearestNeighbor import GridInverseDistanceNearestNeighbor
from .GridLinear import GridLinear
from .GridNearestNeighbor import GridNearestNeighbor
from .hillshade import hillshade
from .merge import merge
from .nearblack import nearblack
from .pct2rgb import pct2rgb
from .polygonize import polygonize
from .proximity import proximity
from .rasterize import rasterize
from .rearrange_bands import rearrange_bands
from .retile import retile
from .rgb2pct import rgb2pct
from .roughness import roughness
from .sieve import sieve
from .slope import slope
from .translate import translate
from .tpi import tpi
from .tri import tri
from .warp import warp

from .extractprojection import ExtractProjection
# from .rasterize_over import rasterize_over

from .Buffer import Buffer
from .ClipVectorByExtent import ClipVectorByExtent
from .ClipVectorByMask import ClipVectorByMask
from .Dissolve import Dissolve
from .ExecuteSql import ExecuteSql
from .OffsetCurve import OffsetCurve
from .ogr2ogr import ogr2ogr
from .ogrinfo import ogrinfo
from .OgrToPostGis import OgrToPostGis
from .ogr2ogrtopostgislist import Ogr2OgrToPostGisList
from .OneSideBuffer import OneSideBuffer
from .PointsAlongLines import PointsAlongLines

# from .ogr2ogrtabletopostgislist import Ogr2OgrTableToPostGisList

pluginPath = os.path.normpath(os.path.join(
    os.path.split(os.path.dirname(__file__))[0], os.pardir))


class GdalAlgorithmProvider(QgsProcessingProvider):

    def __init__(self):
        super().__init__()
        self.algs = []

    def load(self):
        ProcessingConfig.settingIcons[self.name()] = self.icon()
        ProcessingConfig.addSetting(Setting(self.name(), 'ACTIVATE_GDAL',
                                            self.tr('Activate'), True))
        ProcessingConfig.addSetting(Setting(
            self.name(),
            GdalUtils.GDAL_HELP_PATH,
            self.tr('Location of GDAL docs'),
            GdalUtils.gdalHelpPath()))
        ProcessingConfig.readSettings()
        self.refreshAlgorithms()
        return True

    def unload(self):
        ProcessingConfig.removeSetting('ACTIVATE_GDAL')
        ProcessingConfig.removeSetting(GdalUtils.GDAL_HELP_PATH)

    def isActive(self):
        return ProcessingConfig.getSetting('ACTIVATE_GDAL')

    def setActive(self, active):
        ProcessingConfig.setSettingValue('ACTIVATE_GDAL', active)

    def name(self):
        return 'GDAL'

    def longName(self):
        version = GdalUtils.readableVersion()
        return 'GDAL ({})'.format(version)

    def id(self):
        return 'gdal'

    def helpId(self):
        return 'gdal'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerGdal.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerGdal.svg")

    def loadAlgorithms(self):
        self.algs = [
            AssignProjection(),
            aspect(),
            buildvrt(),
            ClipRasterByExtent(),
            ClipRasterByMask(),
            ColorRelief(),
            contour(),
            fillnodata(),
            gdalinfo(),
            gdal2tiles(),
            gdal2xyz(),
            gdaladdo(),
            gdalcalc(),
            gdaltindex(),
            GridAverage(),
            GridDataMetrics(),
            GridInverseDistance(),
            GridInverseDistanceNearestNeighbor(),
            GridLinear(),
            GridNearestNeighbor(),
            hillshade(),
            merge(),
            nearblack(),
            pct2rgb(),
            polygonize(),
            proximity(),
            rasterize(),
            rearrange_bands(),
            retile(),
            rgb2pct(),
            roughness(),
            sieve(),
            slope(),
            translate(),
            tpi(),
            tri(),
            warp(),
            # rasterize(),
            ExtractProjection(),
            # rasterize_over(),
            # ----- OGR tools -----
            Buffer(),
            ClipVectorByExtent(),
            ClipVectorByMask(),
            Dissolve(),
            ExecuteSql(),
            OffsetCurve(),
            ogr2ogr(),
            ogrinfo(),
            OgrToPostGis(),
            Ogr2OgrToPostGisList(),
            OneSideBuffer(),
            PointsAlongLines(),
            # Ogr2OgrTableToPostGisList(),
        ]
        for a in self.algs:
            self.addAlgorithm(a)

    def supportedOutputRasterLayerExtensions(self):
        return GdalUtils.getSupportedRasterExtensions()

    def supportsNonFileBasedOutput(self):
        """
        GDAL Provider doesn't support non file based outputs
        """
        return False

    def tr(self, string, context=''):
        if context == '':
            context = 'GdalAlgorithmProvider'
        return QCoreApplication.translate(context, string)
