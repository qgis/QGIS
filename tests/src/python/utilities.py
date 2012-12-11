"""Helper utilities for QGIS python unit tests.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Tim Sutton (tim@linfiniti.com)'
__date__ = '20/01/2011'
__copyright__ = 'Copyright 2012, The Quantum GIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import sys
from PyQt4 import QtGui, QtCore
from qgis.core import (QgsApplication,
                       QgsCoordinateReferenceSystem,
                       QgsVectorFileWriter)
from qgis.gui import QgsMapCanvas
from qgis_interface import QgisInterface
import hashlib

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
GOOGLECRS = 900913  # constant for EPSG:GOOGLECRS Google Mercator id


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
        PARENT = QtGui.QWidget()

    global CANVAS  # pylint: disable=W0603
    if CANVAS is None:
        CANVAS = QgsMapCanvas(PARENT)
        CANVAS.resize(QtCore.QSize(400, 400))

    global IFACE  # pylint: disable=W0603
    if IFACE is None:
        # QgisInterface is a stub implementation of the QGIS plugin interface
        IFACE = QgisInterface(CANVAS)

    return QGISAPP, CANVAS, IFACE, PARENT


def unitTestDataPath(theSubdir=None):
    """Return the absolute path to the InaSAFE unit test data dir.

    .. note:: This is not the same thing as the SVN inasafe_data dir. Rather
       this is a new dataset where the test datasets are all tiny for fast
       testing and the datasets live in the same repo as the code.

    Args:
       * theSubdir: (Optional) Additional subdir to add to the path - typically
         'hazard' or 'exposure'.
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
    myFileName = os.path.join(str(QtCore.QDir.tempPath()), theFileName)
    print myFileName
    # Explicitly giving all options, not really needed but nice for clarity
    myErrorMessage = QtCore.QString()
    myOptions = QtCore.QStringList()
    myLayerOptions = QtCore.QStringList()
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
