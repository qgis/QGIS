"""Helper utilities for QGIS python unit tests.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Tim Sutton (tim@linfiniti.com)"
__date__ = "20/01/2011"
__copyright__ = "Copyright 2012, The QGIS Project"

import os
import platform
import re
import stat
import sys
import tempfile


try:
    from urllib2 import HTTPError, URLError, urlopen
except ImportError:
    from urllib.request import urlopen, HTTPError, URLError

import hashlib
import subprocess
import webbrowser

from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsFontUtils,
    QgsMapRendererParallelJob,
    QgsMapRendererSequentialJob,
    QgsMapSettings,
    QgsVectorFileWriter,
)
from qgis.PyQt.QtCore import QDir, QUrl, QUrlQuery
from qgis.testing import start_app

GEOCRS = 4326  # constant for EPSG:GEOCRS Geographic CRS id

FONTSLOADED = False


def assertHashesForFile(theHashes, theFilename):
    """Assert that a files has matches one of a list of expected hashes"""
    myHash = hashForFile(theFilename)
    myMessage = (
        "Unexpected hash"
        "\nGot: %s"
        "\nExpected: %s"
        "\nPlease check graphics %s visually "
        "and add to list of expected hashes "
        "if it is OK on this platform." % (myHash, theHashes, theFilename)
    )
    assert myHash in theHashes, myMessage


def assertHashForFile(theHash, theFilename):
    """Assert that a files has matches its expected hash"""
    myHash = hashForFile(theFilename)
    myMessage = "Unexpected hash" "\nGot: %s" "\nExpected: %s" % (myHash, theHash)
    assert myHash == theHash, myMessage


def hashForFile(theFilename):
    """Return an md5 checksum for a file"""
    myPath = theFilename
    myData = open(myPath).read()
    myHash = hashlib.md5()
    myHash.update(myData)
    myHash = myHash.hexdigest()
    return myHash


def unitTestDataPath(theSubdir=None):
    """Return the absolute path to the QGIS unit test data dir.

    Args:
       * theSubdir: (Optional) Additional subdir to add to the path
    """
    myPath = __file__
    tmpPath = os.path.split(os.path.dirname(myPath))
    myPath = os.path.split(tmpPath[0])
    if theSubdir is not None:
        myPath = os.path.abspath(os.path.join(myPath[0], "testdata", theSubdir))
    else:
        myPath = os.path.abspath(os.path.join(myPath[0], "testdata"))
    return myPath


def svgSymbolsPath():
    return os.path.abspath(
        os.path.join(unitTestDataPath(), "..", "..", "images", "svg")
    )


def writeShape(theMemoryLayer, theFileName):
    myFileName = os.path.join(str(QDir.tempPath()), theFileName)
    print(myFileName)
    # Explicitly giving all options, not really needed but nice for clarity
    myOptions = []
    myLayerOptions = []
    mySelectedOnlyFlag = False
    mySkipAttributesFlag = False
    myGeoCrs = QgsCoordinateReferenceSystem("EPSG:4326")
    myResult, myErrorMessage = QgsVectorFileWriter.writeAsVectorFormat(
        theMemoryLayer,
        myFileName,
        "utf-8",
        myGeoCrs,
        "ESRI Shapefile",
        mySelectedOnlyFlag,
        myOptions,
        myLayerOptions,
        mySkipAttributesFlag,
    )
    assert (
        myResult == QgsVectorFileWriter.WriterError.NoError
    ), f"Writing shape failed, Error {myResult} ({myErrorMessage})"

    return myFileName


def doubleNear(a, b, tol=0.0000000001):
    """
    Tests whether two floats are near, within a specified tolerance
    """
    return abs(float(a) - float(b)) < tol


def compareUrl(a, b):
    url_a = QUrl(a)
    url_b = QUrl(b)
    query_a = QUrlQuery(url_a.query()).queryItems()
    query_b = QUrlQuery(url_b.query()).queryItems()

    url_equal = url_a.path() == url_b.path()
    for item in query_a:
        if item not in query_b:
            url_equal = False

    return url_equal


def compareWkt(a, b, tol=0.000001):
    """
    Compares two WKT strings, ignoring allowed differences between strings
    and allowing a tolerance for coordinates
    """
    # ignore case
    a0 = a.lower()
    b0 = b.lower()

    # remove optional spaces before z/m
    r = re.compile(r"\s+([zm])")
    a0 = r.sub(r"\1", a0)
    b0 = r.sub(r"\1", b0)

    # spaces before brackets are optional
    r = re.compile(r"\s*\(\s*")
    a0 = r.sub("(", a0)
    b0 = r.sub("(", b0)
    # spaces after brackets are optional
    r = re.compile(r"\s*\)\s*")
    a0 = r.sub(")", a0)
    b0 = r.sub(")", b0)

    # compare the structure
    r0 = re.compile(r"-?\d+(?:\.\d+)?(?:[eE]\d+)?")
    r1 = re.compile(r"\s*,\s*")
    a0 = r1.sub(",", r0.sub("#", a0))
    b0 = r1.sub(",", r0.sub("#", b0))
    if a0 != b0:
        return False

    # compare the numbers with given tolerance
    a0 = r0.findall(a)
    b0 = r0.findall(b)
    if len(a0) != len(b0):
        return False

    for a1, b1 in zip(a0, b0):
        if not doubleNear(a1, b1, tol):
            return False

    return True


def getTempfilePath(sufx="png"):
    """
    :returns: Path to empty tempfile ending in defined suffix
    Caller should delete tempfile if not used
    """
    tmp = tempfile.NamedTemporaryFile(suffix=f".{sufx}", delete=False)
    filepath = tmp.name
    tmp.close()
    # set read permission for group and other so we can access docker test generated file
    # to build artifact for instance
    os.chmod(filepath, stat.S_IRUSR | stat.S_IWUSR | stat.S_IROTH | stat.S_IRGRP)
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

    s = "MapSettings...\n"
    s += f"  layers(): {[layer.name() for layer in ms.layers()]}\n"
    s += "  backgroundColor(): rgba {},{},{},{}\n".format(
        ms.backgroundColor().red(),
        ms.backgroundColor().green(),
        ms.backgroundColor().blue(),
        ms.backgroundColor().alpha(),
    )
    s += "  selectionColor(): rgba {},{},{},{}\n".format(
        ms.selectionColor().red(),
        ms.selectionColor().green(),
        ms.selectionColor().blue(),
        ms.selectionColor().alpha(),
    )
    s += "  outputSize(): {} x {}\n".format(
        ms.outputSize().width(), ms.outputSize().height()
    )
    s += f"  outputDpi(): {ms.outputDpi()}\n"
    s += f"  mapUnits(): {ms.mapUnits()}\n"
    s += f"  scale(): {ms.scale()}\n"
    s += f"  mapUnitsPerPixel(): {ms.mapUnitsPerPixel()}\n"
    s += "  extent():\n    {}\n".format(ms.extent().toString().replace(" : ", "\n    "))
    s += "  visibleExtent():\n    {}\n".format(
        ms.visibleExtent().toString().replace(" : ", "\n    ")
    )
    s += "  fullExtent():\n    {}\n".format(full_ext.replace(" : ", "\n    "))
    s += f"  destinationCrs(): {ms.destinationCrs().authid()}\n"
    s += "  flag.Antialiasing: {}\n".format(
        ms.testFlag(QgsMapSettings.Flag.Antialiasing)
    )
    s += "  flag.UseAdvancedEffects: {}\n".format(
        ms.testFlag(QgsMapSettings.Flag.UseAdvancedEffects)
    )
    s += "  flag.ForceVectorOutput: {}\n".format(
        ms.testFlag(QgsMapSettings.Flag.ForceVectorOutput)
    )
    s += "  flag.DrawLabeling: {}\n".format(
        ms.testFlag(QgsMapSettings.Flag.DrawLabeling)
    )
    s += "  flag.DrawEditingInfo: {}\n".format(
        ms.testFlag(QgsMapSettings.Flag.DrawEditingInfo)
    )
    s += f"  outputImageFormat(): {ms.outputImageFormat()}\n"
    return s


def getExecutablePath(exe):
    """
    :param exe: Name of executable, e.g. lighttpd
    :returns: Path to executable
    """
    exe_exts = []
    if platform.system().lower().startswith("win") and "PATHEXT" in os.environ:
        exe_exts = os.environ["PATHEXT"].split(os.pathsep)

    for path in os.environ["PATH"].split(os.pathsep):
        exe_path = os.path.join(path, exe)
        if os.path.exists(exe_path):
            return exe_path
        for ext in exe_exts:
            if os.path.exists(exe_path + ext):
                return exe_path
    return ""


def getTestFontFamily():
    return QgsFontUtils.standardTestFontFamily()


def getTestFont(style="Roman", size=12):
    """Only Roman and Bold are loaded by default
    Others available: Oblique, Bold Oblique
    """
    if not FONTSLOADED:
        loadTestFonts()
    return QgsFontUtils.getStandardTestFont(style, size)


def loadTestFonts():
    start_app()

    global FONTSLOADED  # pylint: disable=W0603
    if FONTSLOADED is False:
        QgsFontUtils.loadStandardTestFonts(["Roman", "Bold", "Deja Bold"])
        msg = getTestFontFamily() + " base test font styles could not be loaded"
        res = QgsFontUtils.fontFamilyHasStyle(
            getTestFontFamily(), "Roman"
        ) and QgsFontUtils.fontFamilyHasStyle(getTestFontFamily(), "Bold")
        assert res, msg
        FONTSLOADED = True


def openInBrowserTab(url):
    if sys.platform[:3] in ("win", "dar"):
        webbrowser.open_new_tab(url)
    else:
        # some Linux OS pause execution on webbrowser open, so background it
        cmd = "import webbrowser;" 'webbrowser.open_new_tab("{}")'.format(url)
        subprocess.Popen(
            [sys.executable, "-c", cmd],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )


def printImportant(info):
    """
    Prints important information to stdout and to a file which in the end
    should be printed on test result pages.
    :param info: A string to print
    """

    print(info)
    with open(os.path.join(tempfile.gettempdir(), "ctest-important.log"), "a+") as f:
        f.write(f"{info}\n")


def waitServer(url, timeout=10):
    r"""Wait for a server to be online and to respond
    HTTP errors are ignored
    \param timeout: in seconds
    \return: True of False
    """
    from time import time as now

    end = now() + timeout
    while True:
        try:
            urlopen(url, timeout=1)
            return True
        except (HTTPError, URLError):
            return True
        except Exception as e:
            if now() > end:
                return False
