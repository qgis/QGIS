"""QGIS Unit tests for QgsServerApiContext class.

From build dir, run: ctest -R PyQgsServerApiContext -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""

__author__ = "Alessandro Pasotti"
__date__ = "11/07/2019"
__copyright__ = "Copyright 2019, The QGIS Project"

import os

# Deterministic XML
os.environ["QT_HASH_SEED"] = "1"

from qgis.server import (
    QgsBufferServerRequest,
    QgsBufferServerResponse,
    QgsServerApiContext,
)
from qgis.testing import unittest
from test_qgsserver import QgsServerTestBase


class QgsServerApiContextsTest(QgsServerTestBase):
    """QGIS Server API context tests"""

    def testMatchedPath(self):
        """Test path extraction"""

        response = QgsBufferServerResponse()
        request = QgsBufferServerRequest("http://www.qgis.org/services/wfs3")
        context = QgsServerApiContext("/wfs3", request, response, None, None)
        self.assertEqual(context.matchedPath(), "/services/wfs3")

        request = QgsBufferServerRequest(
            "http://www.qgis.org/services/wfs3/collections.hml"
        )
        context = QgsServerApiContext("/wfs3", request, response, None, None)
        self.assertEqual(context.matchedPath(), "/services/wfs3")

        request = QgsBufferServerRequest(
            "http://www.qgis.org/services/wfs3/collections.hml"
        )
        context = QgsServerApiContext("/wfs4", request, response, None, None)
        self.assertEqual(context.matchedPath(), "")


if __name__ == "__main__":
    unittest.main()
