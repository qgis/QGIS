# -*- coding: utf-8 -*-

"""
InaSAFE Disaster risk assessment tool developed by AusAid -
**QGIS plugin implementation.**

Contact : ole.moller.nielsen@gmail.com

.. note:: This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

.. note:: This source code was copied from the 'postgis viewer' application
     with original authors:
     Copyright (c) 2010 by Ivan Mincik, ivan.mincik@gista.sk
     Copyright (c) 2011 German Carrillo, geotux_tuxman@linuxmail.org

"""

__author__ = 'tim@linfiniti.com'
__version__ = '0.3.0'
__date__ = '10/01/2011'
__copyright__ = 'Copyright (c) 2010 by Ivan Mincik, ivan.mincik@gista.sk \
        and Copyright (c) 2011 German Carrillo, geotux_tuxman@linuxmail.org'

from PyQt4.QtCore import QObject
from qgis.core import QgsMapLayerRegistry
from qgis.core import QgsRasterLayer, QgsVectorLayer


class QgisInterface(QObject):
    """Class to expose qgis objects and functionalities to plugins.

    This class is here for enabling us to run unit tests only,
    so most methods are simply stubs.
    """

    def __init__(self, canvas):
        """Constructor"""
        QObject.__init__(self)
        self.canvas = canvas
        self.testRaster = QgsRasterLayer('data/raster', 'raster')
        self.testVector = QgsVectorLayer('data/vector', 'vector', 'ogr')
        QgsMapLayerRegistry.instance().addMapLayers([self.testRaster,
                self.testVector])

        self.statusBar = type('FakeStatusBar', (), {'showMessage': lambda _,
                              m: None})()

    def zoomFull(self):
        """Zoom to the map full extent"""
        pass

    def zoomToPrevious(self):
        """Zoom to previous view extent"""
        pass

    def zoomToNext(self):
        """Zoom to next view extent"""
        pass

    def addVectorLayer(self, vectorLayerPath, baseName, providerKey):
        """Add a vector layer"""
        pass

    def addRasterLayer(self, rasterLayerPath, baseName):
        """Add a raster layer given a raster layer file name"""
        pass

    def activeLayer(self):
        """Get pointer to the active layer (layer selected in the legend)"""
        myLayers = QgsMapLayerRegistry.instance().mapLayers()
        for myItem in myLayers:
            return myLayers[myItem]

    def addToolBarIcon(self, qAction):
        """Add an icon to the plugins toolbar"""
        pass

    def removeToolBarIcon(self, qAction):
        """Remove an action (icon) from the plugin toolbar"""
        pass

    def addToolBar(self, name):
        """Add toolbar with specified name"""
        pass

    def mapCanvas(self):
        """Return a pointer to the map canvas"""
        return self.canvas

    def mainWindow(self):
        """Return a pointer to the main window.

        In case of QGIS it returns an instance of QgisApp
        """

        return type('FakeMainWindow', (), {'statusBar': lambda _:
                    self.statusBar})()

    def addDockWidget(self, area, dockwidget):
        """Add a dock widget to the main window"""
        pass

    def legendInterface(self):
        return type('FakeLInterface', (), {'layers': lambda _:
                    [self.testRaster, self.testVector]})()
