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

from qgis.core import (
    QgsApplication,
    QgsProviderRegistry,
    QgsVectorLayer,
    QgsWkbTypes,
    QgsFields
)

from qgis.gui import QgsNewVectorTableDialog
from qgis.testing import start_app, unittest
from qgis.PyQt.QtWidgets import (
    QDialogButtonBox,
    QLineEdit,
    QComboBox,
    QCheckBox
)

from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()


class TestPyQgsNewVectorTableDialog(unittest.TestCase):
    """Test QgsNewVectorTableDialog"""

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        start_app()
        gpkg_original_path = '{}/qgis_server/test_project_wms_grouped_layers.gpkg'.format(
            TEST_DATA_DIR)
        cls.gpkg_path = tempfile.mktemp('.gpkg')
        shutil.copy(gpkg_original_path, cls.gpkg_path)
        vl = QgsVectorLayer('{}|layername=cdb_lines'.format(
            cls.gpkg_path), 'test', 'ogr')
        assert vl.isValid()
        cls.uri = cls.gpkg_path

    def test_dialog(self):

        md = QgsProviderRegistry.instance().providerMetadata('ogr')
        conn = md.createConnection(self.uri, {})
        dialog = QgsNewVectorTableDialog(conn)
        dialog.setFields(conn.fields('', 'cdb_lines'))
        dialog.setTableName('dont_lock_me_down_again')

        # dialog.exec_()

        geom_type_combo = dialog.findChildren(QComboBox, 'mGeomTypeCbo')[0]
        geom_name_le = dialog.findChildren(QLineEdit, 'mGeomColumn')[0]
        has_z_chk = dialog.findChildren(QCheckBox, 'mHasZChk')[0]
        has_m_chk = dialog.findChildren(QCheckBox, 'mHasMChk')[0]
        table_name = dialog.findChildren(QLineEdit, 'mTableName')[0]
        buttons = dialog.findChildren(QDialogButtonBox, 'mButtonBox')[0]
        ok_btn = buttons.button(QDialogButtonBox.Ok)

        # Default is no geometry, let's check if all geom options are disabled
        self.assertFalse(geom_name_le.isEnabled())
        self.assertFalse(has_z_chk.isEnabled())
        self.assertFalse(has_m_chk.isEnabled())

        # 2 is linestring
        geom_type_combo.setCurrentIndex(3)
        self.assertTrue(geom_name_le.isEnabled())
        self.assertTrue(has_z_chk.isEnabled())
        self.assertTrue(has_m_chk.isEnabled())

        self.assertEqual(dialog.geometryType(), QgsWkbTypes.LineString)

        # Set Z and check the type
        has_z_chk.setChecked(True)
        self.assertEqual(dialog.geometryType(), QgsWkbTypes.LineStringZ)
        has_z_chk.setChecked(False)

        # Set M and check the type
        has_m_chk.setChecked(True)
        self.assertEqual(dialog.geometryType(), QgsWkbTypes.LineStringM)

        # Set both
        has_z_chk.setChecked(True)
        self.assertEqual(dialog.geometryType(), QgsWkbTypes.LineStringZM)

        # Test validation (ok button enabled)
        buttons = dialog.findChildren(QDialogButtonBox, 'mButtonBox')[0]
        ok_btn = buttons.button(QDialogButtonBox.Ok)
        self.assertTrue(ok_btn.isEnabled())

        # Duplicate table name
        table_name.setText('cdb_lines')
        self.assertFalse(ok_btn.isEnabled())
        table_name.setText('cdb_lines2')
        self.assertTrue(ok_btn.isEnabled())

        # No fields (but geometry is ok)
        dialog.setFields(QgsFields())
        self.assertTrue(ok_btn.isEnabled())

        # Change to aspatial and check validity
        geom_type_combo.setCurrentIndex(0)
        self.assertFalse(ok_btn.isEnabled())


if __name__ == '__main__':
    unittest.main()
