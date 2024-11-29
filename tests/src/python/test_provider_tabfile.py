"""QGIS Unit tests for the OGR/MapInfo tab provider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "2016-01-28"
__copyright__ = "Copyright 2016, The QGIS Project"

import os
import shutil
import tempfile

import osgeo.gdal  # NOQA
from qgis.PyQt.QtCore import QDate, QDateTime, QDir, QTime, QVariant
from qgis.core import (
    QgsFeatureRequest,
    QgsField,
    QgsVectorDataProvider,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


# Note - doesn't implement ProviderTestCase as OGR provider is tested by the shapefile provider test


class TestPyQgsTabfileProvider(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        cls.basetestpath = tempfile.mkdtemp()
        cls.dirs_to_cleanup = [cls.basetestpath]

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        for dirname in cls.dirs_to_cleanup:
            shutil.rmtree(dirname, True)
        super().tearDownClass()

    def testDateTimeFormats(self):
        # check that date and time formats are correctly interpreted
        basetestfile = os.path.join(TEST_DATA_DIR, "tab_file.tab")
        vl = QgsVectorLayer(f"{basetestfile}|layerid=0", "test", "ogr")

        fields = vl.dataProvider().fields()
        self.assertEqual(fields.at(fields.indexFromName("date")).type(), QVariant.Date)
        self.assertEqual(fields.at(fields.indexFromName("time")).type(), QVariant.Time)
        self.assertEqual(
            fields.at(fields.indexFromName("date_time")).type(), QVariant.DateTime
        )

        f = next(vl.getFeatures(QgsFeatureRequest()))

        date_idx = vl.fields().lookupField("date")
        assert isinstance(f.attributes()[date_idx], QDate)
        self.assertEqual(f.attributes()[date_idx], QDate(2004, 5, 3))
        time_idx = vl.fields().lookupField("time")
        assert isinstance(f.attributes()[time_idx], QTime)
        self.assertEqual(f.attributes()[time_idx], QTime(13, 41, 00))
        datetime_idx = vl.fields().lookupField("date_time")
        assert isinstance(f.attributes()[datetime_idx], QDateTime)
        self.assertEqual(
            f.attributes()[datetime_idx],
            QDateTime(QDate(2004, 5, 3), QTime(13, 41, 00)),
        )

    def testUpdateMode(self):
        """Test that on-the-fly re-opening in update/read-only mode works"""

        basetestfile = os.path.join(TEST_DATA_DIR, "tab_file.tab")
        vl = QgsVectorLayer(f"{basetestfile}|layerid=0", "test", "ogr")
        caps = vl.dataProvider().capabilities()
        self.assertTrue(caps & QgsVectorDataProvider.Capability.AddFeatures)

        # We should be really opened in read-only mode even if write capabilities are declared
        self.assertEqual(vl.dataProvider().property("_debug_open_mode"), "read-only")

        # Test that startEditing() / commitChanges() plays with enterUpdateMode() / leaveUpdateMode()
        self.assertTrue(vl.startEditing())
        self.assertEqual(vl.dataProvider().property("_debug_open_mode"), "read-write")
        self.assertTrue(vl.dataProvider().isValid())

        self.assertTrue(vl.commitChanges())
        self.assertEqual(vl.dataProvider().property("_debug_open_mode"), "read-only")
        self.assertTrue(vl.dataProvider().isValid())

    def testInteger64WriteTabfile(self):
        """Check writing Integer64 fields to an MapInfo tabfile (which does not support that type)."""

        base_dest_file_name = os.path.join(str(QDir.tempPath()), "integer64")
        dest_file_name = base_dest_file_name + ".tab"
        shutil.copy(
            os.path.join(TEST_DATA_DIR, "tab_file.tab"), base_dest_file_name + ".tab"
        )
        shutil.copy(
            os.path.join(TEST_DATA_DIR, "tab_file.dat"), base_dest_file_name + ".dat"
        )
        shutil.copy(
            os.path.join(TEST_DATA_DIR, "tab_file.map"), base_dest_file_name + ".map"
        )
        shutil.copy(
            os.path.join(TEST_DATA_DIR, "tab_file.id"), base_dest_file_name + ".id"
        )
        vl = QgsVectorLayer(f"{dest_file_name}|layerid=0", "test", "ogr")
        self.assertTrue(vl.isValid())
        self.assertTrue(
            vl.dataProvider().addAttributes(
                [QgsField("int8", QVariant.LongLong, "integer64")]
            )
        )

    def testEmbeddedSymbols(self):
        """
        Test retrieving embedded symbols from a TAB file
        """
        layer = QgsVectorLayer(
            os.path.join(TEST_DATA_DIR, "embedded_symbols", "lines.TAB"), "Lines"
        )
        self.assertTrue(layer.isValid())

        # symbols should not be fetched by default
        self.assertFalse(any(f.embeddedSymbol() for f in layer.getFeatures()))

        symbols = [
            f.embeddedSymbol().clone()
            for f in layer.getFeatures(
                QgsFeatureRequest().setFlags(QgsFeatureRequest.Flag.EmbeddedSymbols)
            )
        ]
        self.assertTrue(all(symbols))
        self.assertCountEqual(
            [s.color().name() for s in symbols], ["#0040c0", "#ffb060", "#e03800"]
        )


if __name__ == "__main__":
    unittest.main()
