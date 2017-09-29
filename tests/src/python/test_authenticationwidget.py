# -*- coding: utf-8 -*-
"""
Tests for authentication widget

From build dir, run from test directory:
LC_ALL=en_US.UTF-8 ctest -R PyQgsAuthenticationWidget -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
import os
import re
import string
import sys
from shutil import rmtree
import tempfile
import random

from qgis.core import QgsAuthManager, QgsAuthMethodConfig, QgsNetworkAccessManager, QgsSettings
from qgis.gui import QgsAuthenticationWidget
from qgis.testing import start_app, unittest

from utilities import unitTestDataPath

__author__ = 'Alessandro Pasotti'
__date__ = '27/09/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


QGIS_AUTH_DB_DIR_PATH = tempfile.mkdtemp()

os.environ['QGIS_AUTH_DB_DIR_PATH'] = QGIS_AUTH_DB_DIR_PATH

qgis_app = start_app()


class TestAuthenticationWidget(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests:
        Creates an auth configuration"""
        # Enable auth
        # os.environ['QGIS_AUTH_PASSWORD_FILE'] = QGIS_AUTH_PASSWORD_FILE
        authm = QgsAuthManager.instance()
        assert (authm.setMasterPassword('masterpassword', True))
        cls.auth_config = QgsAuthMethodConfig('Basic')
        cls.auth_config.setName('test_auth_config')
        cls.username = ''.join(random.choice(string.ascii_uppercase + string.digits) for _ in range(6))
        cls.password = cls.username[::-1]  # reversed
        cls.auth_config.setConfig('username', cls.username)
        cls.auth_config.setConfig('password', cls.password)
        assert (authm.storeAuthenticationConfig(cls.auth_config)[0])

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        rmtree(QGIS_AUTH_DB_DIR_PATH)

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    def testWidget(self):
        """
        Test the widget
        """
        w = QgsAuthenticationWidget()
        w.show()
        from IPython import embed
        embed()


if __name__ == '__main__':
    unittest.main()
