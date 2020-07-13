# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsNewVectorTableDialog

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'Alessandro Pasotti'
__date__ = '12/07/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


import tempfile
import shutil

from qgis.core import QgsApplication, QgsProviderRegistry, QgsVectorLayer
from qgis.gui import QgsNewVectorTableDialog
from qgis.testing import start_app, unittest

from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsNewVectorTableDialog(unittest.TestCase):
    """Test QgsNewVectorTableDialog"""

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        start_app()
        gpkg_original_path = '{}/qgis_server/test_project_wms_grouped_layers.gpkg'.format(TEST_DATA_DIR)
        cls.gpkg_path = tempfile.mktemp('.gpkg')
        shutil.copy(gpkg_original_path, cls.gpkg_path)
        vl = QgsVectorLayer('{}|layername=cdb_lines'.format(cls.gpkg_path), 'test', 'ogr')
        assert vl.isValid()
        cls.uri = cls.gpkg_path

    def test_dialog(self):

        md = QgsProviderRegistry.instance().providerMetadata('ogr')
        conn = md.createConnection(self.uri, {})
        dialog = QgsNewVectorTableDialog(conn)
        dialog.setFields(conn.fields('', 'cdb_lines'))
        dialog.exec_()


if __name__ == '__main__':
    unittest.main()
