"""Helper utilities for QGIS python unit tests.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Tim Sutton (tim@linfiniti.com)'
__date__ = '20/01/2011'
__copyright__ = 'Copyright 2012, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis
import os
import sys
import platform
import tempfile

from PyQt4.QtCore import QSize, QDir
from PyQt4.QtGui import QWidget

from qgis.core import (
    QgsApplication,
    QgsCoordinateReferenceSystem,
    QgsVectorFileWriter,
    QgsMapLayerRegistry,
    QgsMapSettings,
    QgsMapRendererParallelJob,
    QgsMapRendererSequentialJob,
    QgsFontUtils
)
from qgis.gui import QgsMapCanvas
from qgis_interface import QgisInterface
import hashlib
import re
from itertools import izip

import webbrowser
import subprocess

# Support python < 2.7 via unittest2 needed for expected failure decorator.
# Note that you should ignore unused import warnings here as these are imported
# from this module by other tests.
if sys.version_info[0:2] < (2, 7):
    try:
        from unittest2 import TestCase, expectedFailure
        import unittest2 as unittest
    except ImportError:
        print "You should install unittest2 to run the salt tests"
        sys.exit(0)
else:
    from unittest import TestCase, expectedFailure
    import unittest

QGISAPP = None  # Static variable used to hold hand to running QGis app
CANVAS = None
PARENT = None
IFACE = None
GEOCRS = 4326  # constant for EPSG:GEOCRS Geographic CRS id

FONTSLOADED = False


def assertHashesForFile(theHashes, theFilename):
    """Assert that a files has matches one of a list of expected hashes"""
    myHash = hashForFile(theFilename)
    myMessage = ('Unexpected hash'
                 '\nGot: %s'
                 '\nExpected: %s'
                 '\nPlease check graphics %s visually '
                 'and add to list of expected hashes '
                 'if it is OK on this platform.'
                 % (myHash, theHashes, theFilename))
    assert myHash in theHashes, myMessage


def assertHashForFile(theHash, theFilename):
    """Assert that a files has matches its expected hash"""
    myHash = hashForFile(theFilename)
    myMessage = ('Unexpected hash'
                 '\nGot: %s'
                 '\nExpected: %s' % (myHash, theHash))
    assert myHash == theHash, myMessage


def hashForFile(theFilename):
    """Return an md5 checksum for a file"""
    myPath = theFilename
    myData = file(myPath).read()
    myHash = hashlib.md5()
    myHash.update(myData)
    myHash = myHash.hexdigest()
    return myHash


def getQgisTestApp():
    """ Start one QGis application to test agaist

    Input
        NIL

    Output
        handle to qgis app


    If QGis is already running the handle to that app will be returned
    """

    global QGISAPP  # pylint: disable=W0603

    if QGISAPP is None:
        myGuiFlag = True  # All test will run qgis in gui mode

        # Note: QGIS_PREFIX_PATH is evaluated in QgsApplication -
        # no need to mess with it here.
        QGISAPP = QgsApplication(sys.argv, myGuiFlag)

        QGISAPP.initQgis()
        s = QGISAPP.showSettings()
        print s

    global PARENT  # pylint: disable=W0603
    if PARENT is None:
        PARENT = QWidget()

    global CANVAS  # pylint: disable=W0603
    if CANVAS is None:
        CANVAS = QgsMapCanvas(PARENT)
        CANVAS.resize(QSize(400, 400))

    global IFACE  # pylint: disable=W0603
    if IFACE is None:
        # QgisInterface is a stub implementation of the QGIS plugin interface
        IFACE = QgisInterface(CANVAS)

    return QGISAPP, CANVAS, IFACE, PARENT


def unitTestDataPath(theSubdir=None):
    """Return the absolute path to the QGIS unit test data dir.

    Args:
       * theSubdir: (Optional) Additional subdir to add to the path
    """
    myPath = __file__
    tmpPath = os.path.split(os.path.dirname(myPath))
    myPath = os.path.split(tmpPath[0])
    if theSubdir is not None:
        myPath = os.path.abspath(os.path.join(myPath[0],
                                              'testdata',
                                              theSubdir))
    else:
        myPath = os.path.abspath(os.path.join(myPath[0], 'testdata'))
    return myPath


def svgSymbolsPath():
    return os.path.abspath(
        os.path.join(unitTestDataPath(), '..', '..', 'images', 'svg'))


def setCanvasCrs(theEpsgId, theOtfpFlag=False):
    """Helper to set the crs for the CANVAS before a test is run.

    Args:

        * theEpsgId  - Valid EPSG identifier (int)
        * theOtfpFlag - whether on the fly projections should be enabled
                        on the CANVAS. Default to False.
    """
    # Enable on-the-fly reprojection
    CANVAS.mapRenderer().setProjectionsEnabled(theOtfpFlag)

    # Create CRS Instance
    myCrs = QgsCoordinateReferenceSystem()
    myCrs.createFromId(theEpsgId, QgsCoordinateReferenceSystem.EpsgCrsId)

    # Reproject all layers to WGS84 geographic CRS
    CANVAS.mapRenderer().setDestinationCrs(myCrs)


def writeShape(theMemoryLayer, theFileName):
    myFileName = os.path.join(str(QDir.tempPath()), theFileName)
    print myFileName
    # Explicitly giving all options, not really needed but nice for clarity
    myErrorMessage = ''
    myOptions = []
    myLayerOptions = []
    mySelectedOnlyFlag = False
    mySkipAttributesFlag = False
    myGeoCrs = QgsCoordinateReferenceSystem()
    myGeoCrs.createFromId(4326, QgsCoordinateReferenceSystem.EpsgCrsId)
    myResult = QgsVectorFileWriter.writeAsVectorFormat(
        theMemoryLayer,
        myFileName,
        'utf-8',
        myGeoCrs,
        'ESRI Shapefile',
        mySelectedOnlyFlag,
        myErrorMessage,
        myOptions,
        myLayerOptions,
        mySkipAttributesFlag)
    assert myResult == QgsVectorFileWriter.NoError


def compareWkt(a, b, tol=0.000001):
    r0 = re.compile( "-?\d+(?:\.\d+)?(?:[eE]\d+)?" )
    r1 = re.compile( "\s*,\s*" )

    # compare the structure
    a0 = r1.sub( ",", r0.sub( "#", a ) )
    b0 = r1.sub( ",", r0.sub( "#", b ) )
    if a0 != b0:
        return False

    # compare the numbers with given tolerance
    a0 = r0.findall( a )
    b0 = r0.findall( b )
    if len(a0) != len(b0):
        return False

    for (a1,b1) in izip(a0,b0):
        if abs(float(a1)-float(b1))>tol:
            return False

    return True


def getTempfilePath(sufx='png'):
    """
    :returns: Path to empty tempfile ending in defined suffix
    Caller should delete tempfile if not used
    """
    tmp = tempfile.NamedTemporaryFile(
        suffix=".{0}".format(sufx), delete=False)
    filepath = tmp.name
    tmp.close()
    return filepath


def renderMapToImage(mapsettings, parallel=False):
    """
    Render current map to an image, via multi-threaded renderer
    :param QgsMapSettings mapsettings:
    :param bool parallel: Do parallel or sequential render job
    :rtype: QImage
    """
    if parallel:
        job = QgsMapRendererParallelJob(mapsettings)
    else:
        job = QgsMapRendererSequentialJob(mapsettings)
    job.start()
    job.waitForFinished()

    return job.renderedImage()


def mapSettingsString(ms):
    """
    :param QgsMapSettings mapsettings:
    :rtype: str
    """
    # fullExtent() causes extra call in middle of output flow; get first
    full_ext = ms.visibleExtent().toString()

    s = 'MapSettings...\n'
    s += '  layers(): {0}\n'.format(
        [unicode(QgsMapLayerRegistry.instance().mapLayer(i).name())
         for i in ms.layers()])
    s += '  backgroundColor(): rgba {0},{1},{2},{3}\n'.format(
        ms.backgroundColor().red(), ms.backgroundColor().green(),
        ms.backgroundColor().blue(), ms.backgroundColor().alpha())
    s += '  selectionColor(): rgba {0},{1},{2},{3}\n'.format(
        ms.selectionColor().red(), ms.selectionColor().green(),
        ms.selectionColor().blue(), ms.selectionColor().alpha())
    s += '  outputSize(): {0} x {1}\n'.format(
        ms.outputSize().width(), ms.outputSize().height())
    s += '  outputDpi(): {0}\n'.format(ms.outputDpi())
    s += '  mapUnits(): {0}\n'.format(ms.mapUnits())
    s += '  scale(): {0}\n'.format(ms.scale())
    s += '  mapUnitsPerPixel(): {0}\n'.format(ms.mapUnitsPerPixel())
    s += '  extent():\n    {0}\n'.format(
        ms.extent().toString().replace(' : ', '\n    '))
    s += '  visibleExtent():\n    {0}\n'.format(
        ms.visibleExtent().toString().replace(' : ', '\n    '))
    s += '  fullExtent():\n    {0}\n'.format(full_ext.replace(' : ', '\n    '))
    s += '  hasCrsTransformEnabled(): {0}\n'.format(
        ms.hasCrsTransformEnabled())
    s += '  destinationCrs(): {0}\n'.format(
        ms.destinationCrs().authid())
    s += '  flag.Antialiasing: {0}\n'.format(
        ms.testFlag(QgsMapSettings.Antialiasing))
    s += '  flag.UseAdvancedEffects: {0}\n'.format(
        ms.testFlag(QgsMapSettings.UseAdvancedEffects))
    s += '  flag.ForceVectorOutput: {0}\n'.format(
        ms.testFlag(QgsMapSettings.ForceVectorOutput))
    s += '  flag.DrawLabeling: {0}\n'.format(
        ms.testFlag(QgsMapSettings.DrawLabeling))
    s += '  flag.DrawEditingInfo: {0}\n'.format(
        ms.testFlag(QgsMapSettings.DrawEditingInfo))
    s += '  outputImageFormat(): {0}\n'.format(ms.outputImageFormat())
    return s


def getExecutablePath(exe):
    """
    :param exe: Name of executable, e.g. lighttpd
    :returns: Path to executable
    """
    exe_exts = []
    if (platform.system().lower().startswith('win') and
            "PATHEXT" in os.environ):
        exe_exts = os.environ["PATHEXT"].split(os.pathsep)

    for path in os.environ["PATH"].split(os.pathsep):
        exe_path = os.path.join(path, exe)
        if os.path.exists(exe_path):
            return exe_path
        for ext in exe_exts:
            if os.path.exists(exe_path + ext):
                return exe_path
    return ''


def getTestFontFamily():
    return QgsFontUtils.standardTestFontFamily()


def getTestFont(style='Roman', size=12):
    """Only Roman and Bold are loaded by default
    Others available: Oblique, Bold Oblique
    """
    if not FONTSLOADED:
        loadTestFonts()
    return QgsFontUtils.getStandardTestFont(style, size)


def loadTestFonts():
    if QGISAPP is None:
        getQgisTestApp()

    global FONTSLOADED  # pylint: disable=W0603
    if FONTSLOADED is False:
        QgsFontUtils.loadStandardTestFonts(['Roman', 'Bold'])
        msg = getTestFontFamily() + ' base test font styles could not be loaded'
        res = (QgsFontUtils.fontFamilyHasStyle(getTestFontFamily(), 'Roman')
               and QgsFontUtils.fontFamilyHasStyle(getTestFontFamily(), 'Bold'))
        assert res, msg
        FONTSLOADED = True


def openInBrowserTab(url):
    if sys.platform[:3] in ('win', 'dar'):
        webbrowser.open_new_tab(url)
    else:
        # some Linux OS pause execution on webbrowser open, so background it
        cmd = 'import webbrowser;' \
              'webbrowser.open_new_tab("{0}")'.format(url)
        subprocess.Popen([sys.executable, "-c", cmd],
                         stdout=subprocess.PIPE,
                         stderr=subprocess.STDOUT)
