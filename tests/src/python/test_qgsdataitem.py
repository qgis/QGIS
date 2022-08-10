# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsDataItem

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Even Rouault'
__date__ = '14/11/2020 late in the night'
__copyright__ = 'Copyright 2020, The QGIS Project'


import os
from qgis.core import QgsDataCollectionItem, QgsDirectoryItem
from utilities import unitTestDataPath
from qgis.testing import start_app, unittest

app = start_app()


class TestQgsDataItem(unittest.TestCase):

    def test_databaseConnection(self):

        dataitem = QgsDataCollectionItem(None, 'name', '/invalid_path', 'ogr')
        self.assertIsNone(dataitem.databaseConnection())
        dataitem = QgsDirectoryItem(None, 'name', os.path.join(unitTestDataPath(), 'provider'))
        children = dataitem.createChildren()
        # Check spatialite and gpkg
        spatialite_item = [i for i in children if i.path().endswith('spatialite.db')][0]
        geopackage_item = [i for i in children if i.path().endswith('geopackage.gpkg')][0]
        textfile_item = [i for i in children if i.path().endswith('.xml')][0]

        self.assertIsNotNone(spatialite_item.databaseConnection())
        self.assertIsNotNone(geopackage_item.databaseConnection())
        self.assertIsNone(textfile_item.databaseConnection())


if __name__ == '__main__':
    unittest.main()
