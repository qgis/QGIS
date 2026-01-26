"""QGIS Unit tests for QgsAction.

From build dir, run: ctest -R PyQgsAction -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Alessandro Pasotti"
__date__ = "24/11/2021"
__copyright__ = "Copyright 2021, The QGIS Project"

import os
from functools import partial

from qgis.PyQt.QtCore import QTemporaryDir
from qgis.core import (
    QgsAction,
    QgsApplication,
    QgsExpressionContext,
    QgsNetworkAccessManager,
    QgsNetworkRequestParameters,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsAction(QgisTestCase):

    def setUp(self):
        self.body = None

    def _req_logger(self, params):
        self.body = bytes(params.content())

    def test_post_urlencoded_action(self):
        """Test form www urlencoded"""

        def _req_logger(self, params):
            self.body = bytes(params.content())

        QgsNetworkAccessManager.instance().requestAboutToBeCreated[
            QgsNetworkRequestParameters
        ].connect(partial(_req_logger, self))

        temp_dir = QTemporaryDir()
        temp_path = temp_dir.path()
        temp_file = os.path.join(temp_path, "urlencoded.txt")

        action = QgsAction(
            QgsAction.ActionType.SubmitUrlEncoded,
            "url_encoded",
            "http://fake_qgis_http_endpoint"
            + temp_file
            + r"?[% url_encode(map('a&+b', 'a and plus b', 'a=b', 'a equals b')) %]",
        )
        ctx = QgsExpressionContext()
        action.run(ctx)

        while not self.body:
            QgsApplication.instance().processEvents()

        self.assertEqual(
            self.body, rb"a%26%2Bb=a%20and%20plus%20b&a%3Db=a%20equals%20b"
        )


if __name__ == "__main__":
    unittest.main()
