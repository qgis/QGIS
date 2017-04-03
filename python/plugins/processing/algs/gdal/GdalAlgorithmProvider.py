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

from qgis.core import QgsApplication
from processing.core.AlgorithmProvider import AlgorithmProvider
from processing.core.ProcessingConfig import ProcessingConfig, Setting
from .GdalUtils import GdalUtils

from .nearblack import nearblack
from .information import information
from .warp import warp
from .rgb2pct import rgb2pct
from .translate import translate
from .pct2rgb import pct2rgb
from .merge import merge
from .buildvrt import buildvrt
from .polygonize import polygonize
from .gdaladdo import gdaladdo
from .ClipByExtent import ClipByExtent
from .ClipByMask import ClipByMask
from .contour import contour
from .rasterize import rasterize
from .proximity import proximity
from .sieve import sieve
from .fillnodata import fillnodata
from .extractprojection import ExtractProjection
from .gdal2xyz import gdal2xyz
from .hillshade import hillshade
from .slope import slope
from .aspect import aspect
from .tri import tri
from .tpi import tpi
from .roughness import roughness
from .ColorRelief import ColorRelief
from .GridInvDist import GridInvDist
from .GridAverage import GridAverage
from .GridNearest import GridNearest
from .GridDataMetrics import GridDataMetrics
from .gdaltindex import gdaltindex
from .gdalcalc import gdalcalc
from .rasterize_over import rasterize_over
from .retile import retile
from .gdal2tiles import gdal2tiles
from .AssignProjection import AssignProjection

from .ogr2ogr import Ogr2Ogr
from .ogr2ogrclip import Ogr2OgrClip
from .ogr2ogrclipextent import Ogr2OgrClipExtent
from .ogr2ogrtopostgis import Ogr2OgrToPostGis
from .ogr2ogrtopostgislist import Ogr2OgrToPostGisList
from .ogr2ogrpointsonlines import Ogr2OgrPointsOnLines
from .ogr2ogrbuffer import Ogr2OgrBuffer
from .ogr2ogrdissolve import Ogr2OgrDissolve
from .onesidebuffer import OneSideBuffer
from .offsetcurve import OffsetCurve
from .ogr2ogrtabletopostgislist import Ogr2OgrTableToPostGisList
from .ogrinfo import OgrInfo
from .ogrsql import OgrSql

pluginPath = os.path.normpath(os.path.join(
    os.path.split(os.path.dirname(__file__))[0], os.pardir))


class GdalAlgorithmProvider(AlgorithmProvider):

    """This provider incorporates GDAL-based algorithms into the
    Processing framework.

    Algorithms are called directly using the command line interface.
    They implemented individually extending GeoAlgorithm class.
    """

    def __init__(self):
        super().__init__()

    def load(self):
        AlgorithmProvider.load(self)
        ProcessingConfig.addSetting(Setting(
            self.name(),
            GdalUtils.GDAL_HELP_PATH,
            self.tr('Location of GDAL docs'),
            GdalUtils.gdalHelpPath()))
        return True

    def unload(self):
        AlgorithmProvider.unload(self)
        ProcessingConfig.removeSetting(GdalUtils.GDAL_HELP_PATH)

    def name(self):
        version = GdalUtils.readableVersion()
        return 'GDAL ({})'.format(version)

    def id(self):
        return 'gdal'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerGdal.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerGdal.svg")

    def loadAlgorithms(self):
        algs = [nearblack(), information(), warp(), translate(),
                rgb2pct(), pct2rgb(), merge(), buildvrt(), polygonize(), gdaladdo(),
                ClipByExtent(), ClipByMask(), contour(), rasterize(), proximity(),
                sieve(), fillnodata(), ExtractProjection(), gdal2xyz(),
                hillshade(), slope(), aspect(), tri(), tpi(), roughness(),
                ColorRelief(), GridInvDist(), GridAverage(), GridNearest(),
                GridDataMetrics(), gdaltindex(), gdalcalc(), rasterize_over(),
                retile(), gdal2tiles(), AssignProjection(),
                # ----- OGR tools -----
                OgrInfo(), Ogr2Ogr(), Ogr2OgrClip(), Ogr2OgrClipExtent(),
                Ogr2OgrToPostGis(), Ogr2OgrToPostGisList(), Ogr2OgrPointsOnLines(),
                Ogr2OgrBuffer(), Ogr2OgrDissolve(), OneSideBuffer(),
                OffsetCurve(), Ogr2OgrTableToPostGisList(), OgrSql(),
                ]
        for a in algs:
            self.addAlgorithm(a)

    def supportedOutputRasterLayerExtensions(self):
        return GdalUtils.getSupportedRasterExtensions()
