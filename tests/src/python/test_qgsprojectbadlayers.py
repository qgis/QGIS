# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsProject.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
from builtins import chr
from builtins import range
__author__ = 'Alessandro Pasotti'
__date__ = '20/10/2018'
__copyright__ = 'Copyright 2018, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os

import qgis  # NOQA

from qgis.core import (QgsProject,
                       QgsVectorLayer,
                       QgsRasterLayer,
                       QgsMapLayer,
                       )
from qgis.gui import (QgsLayerTreeMapCanvasBridge,
                      QgsMapCanvas)

from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtCore import QT_VERSION_STR, QTemporaryDir

from qgis.testing import start_app, unittest
from utilities import (unitTestDataPath)
from shutil import copyfile

app = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsProjectBadLayers(unittest.TestCase):

    def test_project_roundtrip(self):
        temp_dir = QTemporaryDir()
        p = QgsProject.instance()
        for ext in ('shp', 'dbf', 'shx', 'prj'):
            copyfile(os.path.join(TEST_DATA_DIR, 'lines.%s' % ext), os.path.join(temp_dir.path(), 'lines.%s' % ext))
        copyfile(os.path.join(TEST_DATA_DIR, 'raster', 'band1_byte_ct_epsg4326.tif'), os.path.join(temp_dir.path(), 'band1_byte_ct_epsg4326.tif'))
        l = QgsVectorLayer(os.path.join(temp_dir.path(), 'lines.shp'), 'lines', 'ogr')
        self.assertTrue(l.isValid())

        rl = QgsRasterLayer(os.path.join(temp_dir.path(), 'band1_byte_ct_epsg4326.tif'), 'raster', 'gdal')
        self.assertTrue(rl.isValid())
        self.assertTrue(p.addMapLayers([l, rl]))

        # Save project
        project_path = os.path.join(temp_dir.path(), 'project.qgs')
        self.assertTrue(p.write(project_path))

        # Now create and invalid project:
        bad_project_path = os.path.join(temp_dir.path(), 'project_bad.qgs')
        with open(project_path, 'r') as infile:
            with open(bad_project_path, 'w+') as outfile:
                outfile.write(infile.read().replace('./lines.shp', './lines-BAD_SOURCE.shp'))

        # Load the bad project
        self.assertTrue(p.read(bad_project_path))
        # Check layer is invalid
        self.assertFalse(list(p.mapLayers().values())[0].isValid())
        self.assertTrue(list(p.mapLayers().values())[1].isValid())
        # Save the project
        bad_project_path2 = os.path.join(temp_dir.path(), 'project_bad2.qgs')
        p.write(bad_project_path2)
        # Re-save the project, with fixed paths
        good_project_path = os.path.join(temp_dir.path(), 'project_good.qgs')
        with open(bad_project_path2, 'r') as infile:
            with open(good_project_path, 'w+') as outfile:
                outfile.write(infile.read().replace('./lines-BAD_SOURCE.shp', './lines.shp'))

        # Load the good project
        self.assertTrue(p.read(good_project_path))
        # Check layer is valid
        self.assertTrue(list(p.mapLayers().values())[0].isValid())
        self.assertTrue(list(p.mapLayers().values())[1].isValid())


if __name__ == '__main__':
    unittest.main()
