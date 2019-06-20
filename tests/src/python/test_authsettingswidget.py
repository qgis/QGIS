# -*- coding: utf-8 -*-
"""
Tests for authentication widget

From build dir, run from test directory:
LC_ALL=en_US.UTF-8 ctest -R PyQgsAuthSettingsWidget -V

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

from qgis.core import QgsAuthMethodConfig, QgsNetworkAccessManager, QgsSettings, QgsApplication
from qgis.gui import QgsAuthSettingsWidget
from qgis.testing import start_app, unittest

from utilities import unitTestDataPath

__author__ = 'Alessandro Pasotti'
__date__ = '27/09/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'


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
        authm = QgsApplication.authManager()
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

    def testWidgetNoArgs(self):
        """
        Test the widget with no args
        """
        w = QgsAuthSettingsWidget()
        self.assertEqual(w.username(), '')
        self.assertEqual(w.password(), '')
        self.assertEqual(w.configId(), '')
        self.assertTrue(w.configurationTabIsSelected())
        self.assertFalse(w.btnConvertToEncryptedIsEnabled())

    def testWidgetConfigId(self):
        """
        Test the widget with configId
        """
        w = QgsAuthSettingsWidget(None, self.auth_config.id())
        self.assertEqual(w.username(), '')
        self.assertEqual(w.password(), '')
        self.assertEqual(w.configId(), self.auth_config.id())
        self.assertTrue(w.configurationTabIsSelected())
        self.assertFalse(w.btnConvertToEncryptedIsEnabled())

    def testWidgetUsername(self):
        """
        Test the widget with username only
        """
        w = QgsAuthSettingsWidget(None, None, 'username')
        self.assertEqual(w.username(), 'username')
        self.assertEqual(w.password(), '')
        self.assertEqual(w.configId(), '')
        self.assertFalse(w.configurationTabIsSelected())

    def testWidgetPassword(self):
        """
        Test the widget with password only
        """
        w = QgsAuthSettingsWidget(None, None, None, 'password')
        self.assertEqual(w.username(), '')
        self.assertEqual(w.password(), 'password')
        self.assertEqual(w.configId(), '')
        self.assertFalse(w.configurationTabIsSelected())

    def testWidgetUsernameAndPassword(self):
        """
        Test the widget with username and password
        """
        w = QgsAuthSettingsWidget(None, None, 'username', 'password')
        self.assertEqual(w.username(), 'username')
        self.assertEqual(w.password(), 'password')
        self.assertEqual(w.configId(), '')
        self.assertFalse(w.configurationTabIsSelected())
        self.assertTrue(w.btnConvertToEncryptedIsEnabled())

    def testConvertToEncrypted(self):
        """
        Test the widget to encrypted conversion
        """
        w = QgsAuthSettingsWidget(None, None, 'username', 'password')
        self.assertEqual(w.username(), 'username')
        self.assertEqual(w.password(), 'password')
        self.assertEqual(w.configId(), '')
        self.assertFalse(w.configurationTabIsSelected())
        self.assertTrue(w.btnConvertToEncryptedIsEnabled())
        self.assertTrue(w.convertToEncrypted())
        self.assertNotEqual(w.configId(), '')
        self.assertEqual(w.username(), '')
        self.assertEqual(w.password(), '')
        self.assertTrue(w.configurationTabIsSelected())
        self.assertFalse(w.btnConvertToEncryptedIsEnabled())

    def test_setters(self):
        """
        Test setters
        """
        w = QgsAuthSettingsWidget()
        w.setUsername('username')
        self.assertFalse(w.configurationTabIsSelected())
        self.assertEqual(w.username(), 'username')

        w = QgsAuthSettingsWidget()
        w.setPassword('password')
        self.assertEqual(w.password(), 'password')
        self.assertFalse(w.configurationTabIsSelected())

        w = QgsAuthSettingsWidget()
        w.setConfigId(self.auth_config.id())
        self.assertEqual(w.configId(), self.auth_config.id())
        self.assertTrue(w.configurationTabIsSelected())

        w = QgsAuthSettingsWidget()
        w.setUsername('username')
        w.setPassword('password')
        w.setConfigId(self.auth_config.id())
        self.assertEqual(w.configId(), self.auth_config.id())
        self.assertTrue(w.configurationTabIsSelected())

        w = QgsAuthSettingsWidget()
        w.setDataprovider('db2')
        self.assertEqual(w.dataprovider(), 'db2')

    def test_storeCheckBoxes(self):
        """
        Test store cb setters and getters
        """
        w = QgsAuthSettingsWidget()
        self.assertFalse(w.storePasswordIsChecked())
        self.assertFalse(w.storeUsernameIsChecked())

        w = QgsAuthSettingsWidget()
        w.setStorePasswordChecked(True)
        self.assertTrue(w.storePasswordIsChecked())
        self.assertFalse(w.storeUsernameIsChecked())

        w = QgsAuthSettingsWidget()
        w.setStoreUsernameChecked(True)
        self.assertFalse(w.storePasswordIsChecked())
        self.assertTrue(w.storeUsernameIsChecked())

        w = QgsAuthSettingsWidget()
        w.setStoreUsernameChecked(True)
        w.setStorePasswordChecked(True)
        self.assertTrue(w.storePasswordIsChecked())
        self.assertTrue(w.storeUsernameIsChecked())


if __name__ == '__main__':
    unittest.main()
