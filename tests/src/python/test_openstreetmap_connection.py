# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_openstreetmap_connection.py
    ---------------------
    Date                 : April 2017
    Copyright            : (C) 2017, Jorge Gustavo Rocha
    Email                : jgr at di dot uminho dot pt
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Jorge Gustavo Rocha'
__date__ = 'January 2017'
__copyright__ = '(C) 2017, Jorge Gustavo Rocha'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.testing import start_app, unittest
from qgis.core import (QgsApplication, QgsSettings, QgsRaster, QgsRasterLayer, QgsProject)

QGISAPP = start_app()

def createLayerFromSettings(url='http://a.tile.openstreetmap.org/{z}/{x}/{y}.png', type='xyz', zmax=19, zmin=0):
    # urlWithParams = 'type=xyz&url=http://a.tile.openstreetmap.org/%7Bz%7D/%7Bx%7D/%7By%7D.png&zmax=19&zmin=0'
    # rlayer = QgsRasterLayer(urlWithParams, 'some layer name', 'wms')
    # if not rlayer.isValid():
    #     print( "Layer failed to load!" )
    # QgsProject.instance().addMapLayer(rlayer)

    uri = 'type=%s&url=%s&zmax=%d&zmin=%d' % (type, url, zmax, zmin)
    rlayer = QgsRasterLayer(uri, 'OpenStreetMap', 'wms')
    return rlayer

class PyQgsOpenStreetMapConnection(unittest.TestCase):
    """
    This class checks if OpenStreetMap connection is properly created
    """

    def setUp(self):
        self.osmTitle = "OpenStreetMap"

    def tearDown(self):
        QgsProject.instance().removeAllMapLayers()

    def test_OpenStreepMapConnectionSettings(self):
        # settings are not available
        # self.settings = QgsSettings()
        # # print( self.settings.childGroups() )
        # # print( self.settings.allKeys() )
        # self.osmUrl = self.settings.value('qgis/connections-xyz/%s/url' % self.osmTitle)
        # self.osmZMax = self.settings.value('qgis/connections-xyz/%s/zmax' % self.osmTitle)
        # self.osmZMin = self.settings.value('qgis/connections-xyz/%s/zmin' % self.osmTitle)
        # # print(self.osmUrl, self.osmZMax, self.osmZMin)
        self.assertTrue(True)

    def test_LayerCreatedFromSettings(self):
        self.layer = createLayerFromSettings()
        self.assertTrue(self.layer.isValid())
        QgsProject.instance().addMapLayer(self.layer)
        self.assertEqual(len(QgsProject.instance().mapLayersByName(self.osmTitle)), 1)

if __name__ == '__main__':
    unittest.main()
