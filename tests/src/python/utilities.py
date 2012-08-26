import os
import sys
from PyQt4 import QtGui, QtCore
from qgis.core import (QgsApplication,
                      QgsVectorLayer,
                      QgsRasterLayer,
                      QgsRectangle,
                      QgsCoordinateReferenceSystem)
from qgis.gui import QgsMapCanvas
from qgis_interface import QgisInterface
import hashlib

QGISAPP = None  # Static vainasafele used to hold hand to running QGis app
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
        QGISAPP = QgsApplication(sys.argv, myGuiFlag)
        if 'QGIS_PREFIX_PATH' in os.environ:
            myPath = os.environ['QGIS_PREFIX_PATH']
            myUseDefaultPathFlag = True
            QGISAPP.setPrefixPath(myPath, myUseDefaultPathFlag)

        if sys.platform.startswith('darwin'):
            # override resource paths, otherwise looks for Resources in app
            if 'QGIS_MAC_PKGDATA_DIR' in os.environ:
                myPkgPath = os.environ['QGIS_MAC_PKGDATA_DIR']
                QGISAPP.setPkgDataPath(myPkgPath)
            if 'QGIS_MAC_SVG_DIR'  in os.environ:
                mySVGPath = os.environ['QGIS_MAC_SVG_DIR']
                mySVGPaths = QGISAPP.svgPaths()
                # doesn't get rid of incorrect path, just adds correct one
                mySVGPaths.prepend(mySVGPath)
                QGISAPP.setDefaultSvgPaths(mySVGPaths)

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

