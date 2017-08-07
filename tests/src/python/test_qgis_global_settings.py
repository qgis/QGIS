# -*- coding: utf-8 -*-
"""
***************************************************************************
    test_qgis_global_settings.py
    ---------------------
    Date                 : January 2017
    Copyright            : (C) 2017, Jorge Gustavo Rocha
    Email                : jgr at geomaster dot pt
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
__date__ = 'August 2017'
__copyright__ = '(C) 2017, Jorge Gustavo Rocha'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.testing import start_app, unittest
from qgis.PyQt.QtCore import qDebug
from qgis.core import QgsApplication, QgsRasterLayer, QgsSettings

start_app()

def createXYZLayerFromURL(url):
    typeandurl = "type=xyz&url=" + url
    osm = QgsRasterLayer(typeandurl, "OpenStreetMap", "wms")
    return osm

class TestQgsGlobalSettings(unittest.TestCase):

    def setUp(self):
        """Run before each test."""
        qDebug('setUp')
        pass

    def tearDown(self):
        """Run after each test."""
        qDebug('tearDown')
    pass

    def test_global_settings_exist(self):
        qDebug('QgsApplication.pkgDataPath(): {0}'.format(QgsApplication.pkgDataPath()))
        # Path after deployment: QgsApplication.pkgDataPath() + '/qgis_global_settings.ini'
        # QgsSettings.setGlobalSettingsPath(QgsApplication.pkgDataPath() + '/resources/qgis_global_settings.ini')
        QgsSettings.setGlobalSettingsPath(QgsApplication.pkgDataPath() + '/qgis_global_settings.ini')
        self.settings = QgsSettings('testqgissettings', 'testqgissettings')
        settings = QgsSettings()
        qDebug('settings.allKeys(): {0}'.format(settings.allKeys()))
        defaulturl = settings.value('qgis/connections-xyz/OpenStreetMap/url')

        def testKey():
            self.assertEqual(defaulturl, 'http://a.tile.openstreetmap.org/{z}/{x}/{y}.png')

        def testLayer():
            layer = createXYZLayerFromURL(defaulturl)
            self.assertEqual(layer.name(), 'OpenStreetMap')

        testKey()
        testLayer()

if __name__ == '__main__':
    unittest.main()
