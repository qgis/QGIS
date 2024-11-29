"""QGIS Unit tests for QgsServer WMS GetPrint.

From build dir, run: ctest -R PyQgsServerWMSGetMapDxf -V


.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = "Tudor Bărăscu"
__date__ = "27/01/2021"
__copyright__ = "Copyright 2020, The QGIS Project"

import os
import urllib.parse

from qgis.PyQt import QtCore
from qgis.core import QgsVectorLayer
from qgis.testing import unittest

from test_qgsserver import QgsServerTestBase

# Needed on Qt 5 so that the serialization of XML is consistent among all executions
os.environ["QT_HASH_SEED"] = "1"


class PyQgsServerWMSGetMapDxf(QgsServerTestBase):

    def test_dxf_export_works_with_reverse_axis_epsg(self):
        qs = "?" + "&".join(
            [
                "%s=%s" % i
                for i in list(
                    {
                        "MAP": urllib.parse.quote(
                            os.path.join(self.testdata_path, "test_dxf_export.qgs")
                        ),
                        "SERVICE": "WMS",
                        "VERSION": "1.3.0",
                        "REQUEST": "GetMap",
                        "BBOX": "399980,449980,400050,450100",
                        "CRS": "EPSG:3844",
                        "LAYERS": "test_dxf_export",
                        "STYLES": ",",
                        "FORMAT": "application/dxf",
                        "SCALE": "500",
                        "FILE_NAME": "test_dxf_export.dxf",
                    }.items()
                )
            ]
        )

        r, h = self._result(self._execute_request(qs))

        tempDir = QtCore.QTemporaryDir()
        dxf = os.path.join(tempDir.path(), "test_dxf_export.dxf")
        f = open(dxf, "wb")
        f.write(r)
        f.close()

        vl = QgsVectorLayer(dxf, "lyr", "ogr")
        myMessage = "Expected downloaded dxf contains: {} line\nGot: {}\n".format(
            1,
            vl.featureCount(),
        )

        self.assertEqual(vl.featureCount(), 1, myMessage)

        line_from_dxf = next(vl.getFeatures()).geometry().asWkt()
        line = "LineString (450000 400000, 450100 400000)"

        self.assertEqual(line_from_dxf, line)


if __name__ == "__main__":
    unittest.main()
