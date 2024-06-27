"""
Base tests for auth manager storage API

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import QgsSettings, QgsAuthConfigurationStorage, QgsAuthMethodConfig
from qgis.testing import start_app, QgisTestCase

__author__ = 'Alessandro Pasotti'
__date__ = '2024-06-24'
__copyright__ = 'Copyright 2024, The QGIS Project'


class AuthManagerStorageBaseTestCase(QgisTestCase):

    # This must be populated by the derived class and be an instance of QgsAuthConfigurationStorage
    storage = None

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("%s.com" % __name__)
        QCoreApplication.setApplicationName(__name__)
        QgsSettings().clear()
        start_app()


class TestAuthManagerStorageBase():

    def testInitialized(self):

        assert self.storage is not None
        self.assertTrue(self.storage.initialize())
        for table in ['auth_authorities', 'auth_identities', 'auth_servers', 'auth_settings', 'auth_trust', 'auth_configs']:
            self.assertTrue(self.storage.tableExists(table))

    def testAuthConfigs(self):

        assert self.storage is not None

        self.assertTrue(self.storage.initialize(), self.storage.lastError())

        self.assertTrue(bool(self.storage.capabilities() & QgsAuthConfigurationStorage.Capability.ReadConfiguration))
        self.assertTrue(bool(self.storage.capabilities() & QgsAuthConfigurationStorage.Capability.DeleteConfiguration))
        self.assertTrue(bool(self.storage.capabilities() & QgsAuthConfigurationStorage.Capability.UpdateConfiguration))
        self.assertTrue(bool(self.storage.capabilities() & QgsAuthConfigurationStorage.Capability.CreateConfiguration))
        self.assertTrue(bool(self.storage.capabilities() & QgsAuthConfigurationStorage.Capability.ClearStorage))

        # Create a configuration
        config = QgsAuthMethodConfig()
        config.setId('test')
        config.setName('test name')
        config.setMethod('basic')
        config.setConfig('username', 'test user')
        config.setConfig('password', 'test pass')
        config.setConfig('realm', 'test realm')

        payload = config.configString()
        self.assertTrue(self.storage.storeMethodConfig(config, payload))

        # Read it back
        config2, payload2 = self.storage.loadMethodConfig('test', True)
        config2.loadConfigString(payload2)
        self.assertEqual(config2.id(), 'test')
        self.assertEqual(config2.name(), 'test name')
        self.assertEqual(config2.method(), 'basic')
        self.assertEqual(config2.config('username'), 'test user')
        self.assertEqual(config2.config('password'), 'test pass')
        self.assertEqual(config2.config('realm'), 'test realm')
        self.assertEqual(payload2, payload)

        configs = self.storage.authMethodConfigs(['xxxx'])
        self.assertEqual(len(configs), 0)

        configs = self.storage.authMethodConfigs(['basic'])
        self.assertEqual(len(configs), 1)

        configs = self.storage.authMethodConfigsWithPayload()
        config3 = configs['test']
        payload3 = config3.config('encrypted_payload')
        config3.loadConfigString(payload3)
        self.assertEqual(config3.id(), 'test')
        self.assertEqual(config3.name(), 'test name')
        self.assertEqual(config3.method(), 'basic')
        self.assertEqual(config3.config('username'), 'test user')
        self.assertEqual(config3.config('password'), 'test pass')
        self.assertEqual(config3.config('realm'), 'test realm')
        self.assertEqual(payload3, payload)

        # Clear the storage
        self.assertTrue(self.storage.clearMethodConfigs())

        # Check it's empty
        configs = self.storage.authMethodConfigsWithPayload()
        self.assertEqual(len(configs), 0)
