"""QGIS Unit tests for QgsVectorFileWriter with a PostGres database.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Julien Cabieces"
__date__ = "22/03/2021"
__copyright__ = "Copyright 2021, The QGIS Project"

import os

from qgis.PyQt.QtCore import QDir, QVariant
from qgis.core import (
    QgsCoordinateReferenceSystem,
    QgsVectorFileWriter,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()
start_app()


class TestQgsVectorFileWriterPG(QgisTestCase):

    def testWriteWithBoolField(self):

        # init connection string
        dbconn = "service=qgis_test"
        if "QGIS_PGTEST_DB" in os.environ:
            dbconn = os.environ["QGIS_PGTEST_DB"]

        # create a vector layer
        vl = QgsVectorLayer(
            f'{dbconn} table="qgis_test"."boolean_table" sql=', "testbool", "postgres"
        )
        self.assertTrue(vl.isValid())

        # check that 1 of its fields is a bool
        fields = vl.fields()
        self.assertEqual(fields.at(fields.indexFromName("fld1")).type(), QVariant.Bool)

        # write a gpkg package with a bool field
        crs = QgsCoordinateReferenceSystem("EPSG:4326")
        filename = os.path.join(str(QDir.tempPath()), "with_bool_field")
        rc, errmsg = QgsVectorFileWriter.writeAsVectorFormat(
            vl, filename, "utf-8", crs, "GPKG"
        )

        self.assertEqual(rc, QgsVectorFileWriter.WriterError.NoError)

        # open the resulting geopackage
        vl = QgsVectorLayer(filename + ".gpkg", "", "ogr")
        self.assertTrue(vl.isValid())
        fields = vl.fields()

        # test type of converted field
        idx = fields.indexFromName("fld1")
        self.assertEqual(fields.at(idx).type(), QVariant.Bool)

        # test values
        self.assertEqual(vl.getFeature(1).attributes()[idx], 1)
        self.assertEqual(vl.getFeature(2).attributes()[idx], 0)

        del vl
        os.unlink(filename + ".gpkg")


if __name__ == "__main__":
    unittest.main()
