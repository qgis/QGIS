"""
Tests for auth manager storage API: use a custom storage

From build dir, run from test directory:
LC_ALL=en_US.UTF-8 ctest -R PyQgsAuthManagerStorageCustom -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import unittest

from test_authmanager_storage_base import (
    AuthManagerStorageBaseTestCase,
    TestAuthManagerStorageBase,
)
from qgsauthconfigurationcustomstorage import QgsAuthConfigurationCustomStorage

__author__ = "Alessandro Pasotti"
__date__ = "2024-06-24"
__copyright__ = "Copyright 2024, The QGIS Project"


class TestAuthStorageCustom(AuthManagerStorageBaseTestCase, TestAuthManagerStorageBase):

    @classmethod
    def setUpClass(cls):
        """Run before each tests"""

        super().setUpClass()

        config = {"is_encrypted": "false"}
        cls.storage = QgsAuthConfigurationCustomStorage(config)
        assert not cls.storage.isEncrypted()
        assert cls.storage.type() == "custom"


if __name__ == "__main__":
    unittest.main()
