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
from PyQt4.QtCore import *
from PyQt4.QtGui import *

from processing.core.AlgorithmProvider import AlgorithmProvider
from processing.core.ProcessingLog import ProcessingLog
from processing.script.WrongScriptException import WrongScriptException
from processing.algs.gdal.GdalAlgorithm import GdalScriptAlgorithm
from GdalUtils import GdalUtils

from nearblack import nearblack
from information import information
from warp import warp
from rgb2pct import rgb2pct
from translate import translate
from pct2rgb import pct2rgb
from merge import merge
from buildvrt import buildvrt
from polygonize import polygonize
from gdaladdo import gdaladdo
from ClipByExtent import ClipByExtent
from ClipByMask import ClipByMask
from contour import contour
from rasterize import rasterize
from proximity import proximity
from sieve import sieve
from fillnodata import fillnodata
from extractprojection import ExtractProjection
from gdal2xyz import gdal2xyz
from hillshade import hillshade
from slope import slope
from aspect import aspect
from tri import tri
from tpi import tpi
from roughness import roughness
from ColorRelief import ColorRelief
from GridInvDist import GridInvDist
from GridAverage import GridAverage
from GridNearest import GridNearest
from GridDataMetrics import GridDataMetrics

from ogr2ogr import Ogr2Ogr
from ogr2ogrclip import Ogr2OgrClip
from ogr2ogrclipextent import Ogr2OgrClipExtent
from ogrinfo import OgrInfo
from ogrsql import OgrSql


class GdalOgrAlgorithmProvider(AlgorithmProvider):
    """This provider incorporates GDAL-based algorithms into the
    Processing framework.

    Algorithms have been implemented using two different mechanisms,
    which should serve as an example of different ways of extending
    the processing capabilities of QGIS:
      1. when a python script exist for a given process, it has been
         adapted as a Processing python script and loaded using the
         ScriptAlgorithm class. This algorithms call GDAL using its
         Python bindings.
      2. Other algorithms are called directly using the command line
         interface. These have been implemented individually extending
         the GeoAlgorithm class.
    """

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.createAlgsList()

    def scriptsFolder(self):
        """The folder where script algorithms are stored.
        """
        return os.path.dirname(__file__) + '/scripts'

    def getDescription(self):
        return 'GDAL/OGR'

    def getName(self):
        return 'gdalogr'

    def getIcon(self):
        return QIcon(os.path.dirname(__file__) + '/../../images/gdal.png')

    def _loadAlgorithms(self):
        self.algs = self.preloadedAlgs

    def createAlgsList(self):
        # First we populate the list of algorithms with those created
        # extending GeoAlgorithm directly (those that execute GDAL
        # using the console)
        self.preloadedAlgs = [nearblack(), information(), warp(), translate(),
            rgb2pct(), pct2rgb(), merge(), buildvrt(), polygonize(), gdaladdo(),
            ClipByExtent(), ClipByMask(), contour(), rasterize(), proximity(),
            sieve(), fillnodata(), ExtractProjection(), gdal2xyz(),
            hillshade(), slope(), aspect(), tri(), tpi(), roughness(),
            ColorRelief(), GridInvDist(), GridAverage(), GridNearest(),
            GridDataMetrics(),
            # ----- OGR tools -----
            OgrInfo(), Ogr2Ogr(), Ogr2OgrClip(), Ogr2OgrClipExtent(), OgrSql(),
            ]

        # And then we add those that are created as python scripts
        folder = self.scriptsFolder()
        if os.path.exists(folder):
            for descriptionFile in os.listdir(folder):
                if descriptionFile.endswith('py'):
                    try:
                        fullpath = os.path.join(self.scriptsFolder(),
                                descriptionFile)
                        alg = GdalScriptAlgorithm(fullpath)
                        self.preloadedAlgs.append(alg)
                    except WrongScriptException, e:
                        ProcessingLog.addToLog(ProcessingLog.LOG_ERROR, e.msg)

    def getSupportedOutputRasterLayerExtensions(self):
        return GdalUtils.getSupportedRasterExtensions()
