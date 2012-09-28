"""Helper module for gui test suite
"""

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


def getQgisTestApp():
    """ Start one QGis application to test agaist

    Input
        NIL

    Output
        handle to qgis app


    If QGis is already running the handle to that app will be returned
    """

    global QGISAPP

    if QGISAPP is None:
        myGuiFlag = True  # All test will run qgis in gui mode
        QGISAPP = QgsApplication(sys.argv, myGuiFlag)
        if 'QGISPATH' in os.environ:
            myPath = os.environ['QGISPATH']
            myUseDefaultPathFlag = True
            QGISAPP.setPrefixPath(myPath, myUseDefaultPathFlag)
        else:
            print "Warning: QGISPATH is not set"

        QGISAPP.initQgis()
        s = QGISAPP.showSettings()
        print s

    global PARENT
    if PARENT is None:
        PARENT = QtGui.QWidget()

    global CANVAS
    if CANVAS is None:
        CANVAS = QgsMapCanvas(PARENT)
        CANVAS.resize(QtCore.QSize(400, 400))

    global IFACE
    if IFACE is None:
        # QgisInterface is a stub implementation of the QGIS plugin interface
        IFACE = QgisInterface(CANVAS)

    return QGISAPP, CANVAS, IFACE, PARENT

