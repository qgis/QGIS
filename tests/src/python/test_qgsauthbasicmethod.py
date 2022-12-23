# -*- coding: utf-8 -*-
"""
Tests for Basic Auth

From build dir, run: ctest -R PyQgsAuthBasicMethod -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os
import tempfile
import base64

from qgis.core import QgsApplication, QgsAuthManager, QgsAuthMethodConfig, QgsNetworkAccessManager
from qgis.PyQt.QtCore import QFileInfo, QUrl
from qgis.testing import start_app, unittest
from qgis.PyQt.QtNetwork import QNetworkRequest

AUTHDBDIR = tempfile.mkdtemp()
os.environ['QGIS_AUTH_DB_DIR_PATH'] = AUTHDBDIR


__author__ = 'Alessandro Pasotti'
__date__ = '13/10/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

qgis_app = start_app()


class TestAuthManager(unittest.TestCase):

    @classmethod
    def setUpAuth(cls, username, password):
        """Run before all tests and set up authentication"""
        assert (cls.authm.setMasterPassword('masterpassword', True))
        # Client side
        auth_config = QgsAuthMethodConfig("Basic")
        auth_config.setConfig('username', username)
        auth_config.setConfig('password', password)
        auth_config.setName('test_basic_auth_config')
        assert (cls.authm.storeAuthenticationConfig(auth_config)[0])
        assert auth_config.isValid()
        return auth_config

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        cls.authm = QgsApplication.authManager()
        assert not cls.authm.isDisabled(), cls.authm.disabledMessage()

        cls.mpass = 'pass'  # master password

        db1 = QFileInfo(cls.authm.authenticationDatabasePath()
                        ).canonicalFilePath()
        db2 = QFileInfo(AUTHDBDIR + '/qgis-auth.db').canonicalFilePath()
        msg = 'Auth db temp path does not match db path of manager'
        assert db1 == db2, msg

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        pass

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    def _get_decoded_credentials(self, username, password):
        """Extracts and decode credentials from request Authorization header"""

        ac = self.setUpAuth(username, password)
        req = QNetworkRequest(QUrl('http://none'))
        self.authm.updateNetworkRequest(req, ac.id())
        auth = bytes(req.rawHeader(b'Authorization'))[6:]
        # Note that RFC7617 states clearly: User-ids containing colons cannot be encoded in user-pass strings
        u, p = base64.b64decode(auth).split(b':')
        return u.decode('utf8'), p.decode('utf8')

    def testHeaderEncoding(self):
        """Test credentials encoding"""

        for creds in (
            ('username', 'password'),
            ('username', r'pa%%word'),
            ('username', r'èé'),
            ('username', r'😁😂😍'),
        ):
            self.assertEqual(self._get_decoded_credentials(*creds), creds)


if __name__ == '__main__':
    unittest.main()
