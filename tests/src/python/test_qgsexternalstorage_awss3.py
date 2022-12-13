# -*- coding: utf-8 -*-
"""QGIS Unit tests for AWS S3 external storage

External storage backend must implement a test based on TestPyQgsExternalStorageBase

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Jacky Volpes"
__date__ = "20/12/2022"
__copyright__ = "Copyright 2022, The QGIS Project"

from shutil import rmtree
import os
import tempfile
import time

from utilities import unitTestDataPath, waitServer
from test_qgsexternalstorage_base import TestPyQgsExternalStorageBase

from qgis.PyQt.QtCore import QCoreApplication, QEventLoop, QUrl

from qgis.core import (
    QgsApplication,
    QgsAuthMethodConfig,
    QgsExternalStorageFetchedContent,
)

from qgis.testing import (
    start_app,
    unittest,
)


class TestPyQgsExternalStorageAwsS3(TestPyQgsExternalStorageBase, unittest.TestCase):

    storageType = "AWSS3"
    badUrl = "http://nothinghere/"

    @classmethod
    def setUpClass(cls):
        """Run before all tests:"""

        super().setUpClass()

        bucket_name = "test_bucket"

        cls.auth_config = QgsAuthMethodConfig("AWSS3")
        cls.auth_config.setConfig("username", "minioadmin")
        cls.auth_config.setConfig("password", "adminio€")
        cls.auth_config.setConfig("region", "us-east-1")
        cls.auth_config.setName("test_awss3_auth_config")
        assert cls.authm.storeAuthenticationConfig(cls.auth_config)[0]
        assert cls.auth_config.isValid()

        cls.url = "http://{}:{}/{}".format(
            os.environ.get("QGIS_MINIO_HOST", "localhost"),
            os.environ.get("QGIS_MINIO_PORT", "80"),
            bucket_name,
        )


if __name__ == "__main__":
    unittest.main()
