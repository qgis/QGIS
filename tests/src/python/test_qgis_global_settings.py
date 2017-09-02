# -*- coding: utf-8 -*-
"""
***************************************************************************
    test_qgis_global_settings.py
    ---------------------
    Date                 : August 2017
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
        # qDebug('QgsApplication.pkgDataPath(): {0}'.format(QgsApplication.pkgDataPath()))
        # Path after deployment
        # QgsSettings.setGlobalSettingsPath(QgsApplication.pkgDataPath() + '/qgis_global_settings.ini')
        # Path before deployment
        self.assertTrue(QgsSettings.setGlobalSettingsPath(QgsApplication.pkgDataPath() + '/resources/qgis_global_settings.ini'))
        self.settings = QgsSettings('testqgissettings', 'testqgissettings')

    def test_global_settings_exist(self):
        # qDebug('settings.allKeys(): {0}'.format(self.settings.allKeys()))
        defaulturl = self.settings.value('qgis/connections-xyz/OpenStreetMap/url')
        self.assertEqual(defaulturl, 'http://a.tile.openstreetmap.org/{z}/{x}/{y}.png')
        layer = createXYZLayerFromURL(defaulturl)
        self.assertEqual(layer.name(), 'OpenStreetMap')

    def test_global_settings_group(self):
        self.assertEqual(['qgis'], self.settings.childGroups())
        self.assertEqual(['qgis'], self.settings.globalChildGroups())

        self.settings.beginGroup('qgis')
        self.assertEqual(['connections-xyz'], self.settings.childGroups())
        self.assertEqual(['connections-xyz'], self.settings.globalChildGroups())
        # qDebug('settings.allKeys(): {0}'.format(self.settings.allKeys()))
        self.settings.endGroup()

        self.settings.beginGroup('qgis/connections-xyz')
        self.assertEqual(['OpenStreetMap'], self.settings.childGroups())
        self.assertEqual(['OpenStreetMap'], self.settings.globalChildGroups())
        # qDebug('settings.allKeys(): {0}'.format(self.settings.allKeys()))
        self.settings.endGroup()

        # add group to user's settings
        self.settings.beginGroup('qgis/connections-xyz')
        self.settings.setValue('OSM/url', 'http://c.tile.openstreetmap.org/{z}/{x}/{y}.png')
        self.settings.setValue('OSM/zmax', '19')
        self.settings.setValue('OSM/zmin', '0')
        self.settings.endGroup()

        # groups should be different
        self.settings.beginGroup('qgis/connections-xyz')
        self.assertEqual(['OSM', 'OpenStreetMap'], self.settings.childGroups())
        self.assertEqual(['OpenStreetMap'], self.settings.globalChildGroups())
        # qDebug('settings.allKeys(): {0}'.format(self.settings.allKeys()))
        self.settings.endGroup()

        self.settings.beginGroup('qgis/connections-xyz')
        self.settings.remove('OSM')
        self.assertEqual(['OpenStreetMap'], self.settings.childGroups())
        self.assertEqual(['OpenStreetMap'], self.settings.globalChildGroups())
        self.settings.endGroup()

if __name__ == '__main__':
    unittest.main()

    def test_global_groups(self):
        self.assertEqual(self.settings.allKeys(), [])
        self.assertEqual(self.globalsettings.allKeys(), [])

        self.addToDefaults('testqgissettings/foo/first', 'qgis')
        self.addToDefaults('testqgissettings/foo/last', 'rocks')

        self.settings.beginGroup('testqgissettings')
        self.assertEqual(['foo'], self.settings.childGroups())
        self.assertEqual(['foo'], self.settings.globalChildGroups())
        self.settings.endGroup()

        self.settings.setValue('testqgissettings/bar/first', 'qgis')
        self.settings.setValue('testqgissettings/bar/last', 'rocks')

        self.settings.beginGroup('testqgissettings')
        self.assertEqual(sorted(['bar', 'foo']), sorted(self.settings.childGroups()))
        self.assertEqual(['foo'], self.settings.globalChildGroups())
        self.settings.endGroup()

        self.globalsettings.remove('testqgissettings/foo')

        self.settings.beginGroup('testqgissettings')
        self.assertEqual(['bar'], self.settings.childGroups())
        self.assertEqual([], self.settings.globalChildGroups())
        self.settings.endGroup()
