"""
Base tests for auth manager storage API

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os
from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtNetwork import QSslCertificate
from qgis.core import QgsSettings, QgsAuthConfigurationStorage, QgsAuthMethodConfig, QgsAuthConfigSslServer
from qgis.testing import start_app, QgisTestCase
from utilities import unitTestDataPath

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

        cls.certdata_path = os.path.join(unitTestDataPath('auth_system'), 'certs_keys_2048')


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

        # Create another configuration
        config2 = QgsAuthMethodConfig()
        config2.setId('test2')
        config2.setName('test name 2')
        config2.setMethod('basic')
        config2.setConfig('username', 'test user 2')
        config2.setConfig('password', 'test pass 2')
        config2.setConfig('realm', 'test realm 2')
        payload2 = config2.configString()
        self.assertTrue(self.storage.storeMethodConfig(config2, payload2))

        # Read it back
        configback, payloadback = self.storage.loadMethodConfig('test', True)
        configback.loadConfigString(payloadback)
        self.assertEqual(configback.id(), 'test')
        self.assertEqual(configback.name(), 'test name')
        self.assertEqual(configback.method(), 'basic')
        self.assertEqual(configback.config('username'), 'test user')
        self.assertEqual(configback.config('password'), 'test pass')
        self.assertEqual(configback.config('realm'), 'test realm')
        self.assertEqual(payloadback, payload)

        configs = self.storage.authMethodConfigs(['xxxx'])
        self.assertEqual(len(configs), 0)

        configs = self.storage.authMethodConfigs(['basic'])
        self.assertEqual(len(configs), 2)

        configs = self.storage.authMethodConfigsWithPayload()
        configback = configs['test']
        payloadback = configback.config('encrypted_payload')
        configback.loadConfigString(payloadback)
        self.assertEqual(configback.id(), 'test')
        self.assertEqual(configback.name(), 'test name')
        self.assertEqual(configback.method(), 'basic')
        self.assertEqual(configback.config('username'), 'test user')
        self.assertEqual(configback.config('password'), 'test pass')
        self.assertEqual(configback.config('realm'), 'test realm')
        self.assertEqual(payloadback, payload)

        # Remove method config
        self.assertTrue(self.storage.removeMethodConfig('test2'))
        configs = self.storage.authMethodConfigs(['basic'])
        self.assertEqual(len(configs), 1)

        # Clear the storage
        self.assertTrue(self.storage.clearMethodConfigs())

        # Check it's empty
        configs = self.storage.authMethodConfigsWithPayload()
        self.assertEqual(len(configs), 0)

    def testAuthSettings(self):

        assert self.storage is not None

        self.assertTrue(self.storage.initialize(), self.storage.lastError())

        self.assertTrue(bool(self.storage.capabilities() & QgsAuthConfigurationStorage.Capability.ReadSetting))
        self.assertTrue(bool(self.storage.capabilities() & QgsAuthConfigurationStorage.Capability.DeleteSetting))
        self.assertTrue(bool(self.storage.capabilities() & QgsAuthConfigurationStorage.Capability.UpdateSetting))
        self.assertTrue(bool(self.storage.capabilities() & QgsAuthConfigurationStorage.Capability.CreateSetting))
        self.assertTrue(bool(self.storage.capabilities() & QgsAuthConfigurationStorage.Capability.ClearStorage))

        # Create a setting
        self.assertTrue(self.storage.storeAuthSetting('test', 'test value'))

        # Create another setting
        self.assertTrue(self.storage.storeAuthSetting('test2', 'test value 2'))

        self.assertTrue(self.storage.authSettingExists('test'))
        self.assertTrue(self.storage.authSettingExists('test2'))
        self.assertFalse(self.storage.authSettingExists('xxxx'))

        # Read it back
        value = self.storage.loadAuthSetting('test')
        self.assertEqual(value, 'test value')

        # Remove setting
        self.assertTrue(self.storage.removeAuthSetting('test'))
        self.assertTrue(self.storage.removeAuthSetting('test2'))

        # Check it's empty
        self.assertFalse(self.storage.authSettingExists('test'))
        self.assertFalse(self.storage.authSettingExists('test2'))

    def testCertIdentity(self):

        assert self.storage is not None

        self.assertTrue(self.storage.initialize(), self.storage.lastError())

        sslrootcert_path = os.path.join(self.certdata_path, 'qgis_ca.crt')
        sslcert = os.path.join(self.certdata_path, 'Gerardus.crt')
        sslkey_path = os.path.join(self.certdata_path, 'Gerardus.key')
        # In real life, key should be encrypted (the auth manager does that)
        with open(sslkey_path, 'r') as f:
            sslkey = f.read()

        cert = QSslCertificate.fromPath(sslcert)[0]

        self.assertTrue(bool(self.storage.capabilities() & QgsAuthConfigurationStorage.Capability.ReadCertificateIdentity))
        self.assertTrue(bool(self.storage.capabilities() & QgsAuthConfigurationStorage.Capability.DeleteCertificateIdentity))
        self.assertTrue(bool(self.storage.capabilities() & QgsAuthConfigurationStorage.Capability.UpdateCertificateIdentity))
        self.assertTrue(bool(self.storage.capabilities() & QgsAuthConfigurationStorage.Capability.CreateCertificateIdentity))
        self.assertTrue(bool(self.storage.capabilities() & QgsAuthConfigurationStorage.Capability.ClearStorage))

        # Create an identity
        self.assertTrue(self.storage.storeCertIdentity(cert, sslkey))

        ids = self.storage.certIdentityIds()
        cert_id = ids[0]
        self.assertTrue(self.storage.certIdentityExists(cert_id))

        # Read it back
        certback = self.storage.loadCertIdentity(cert_id)

        # Verify the certificate
        self.assertEqual(cert.toPem(), certback.toPem())

        # Remove identity
        self.assertTrue(self.storage.removeCertIdentity(cert_id))

        # Check it's empty
        self.assertFalse(self.storage.certIdentityExists(cert_id))

    def testSslCertificateCustomConfig(self):

        assert self.storage is not None

        self.assertTrue(self.storage.initialize(), self.storage.lastError())

        sslrootcert_path = os.path.join(self.certdata_path, 'qgis_ca.crt')
        sslcert = os.path.join(self.certdata_path, 'Gerardus.crt')
        sslkey_path = os.path.join(self.certdata_path, 'Gerardus.key')
        # In real life, key should be encrypted (the auth manager does that)
        with open(sslkey_path, 'r') as f:
            sslkey = f.read()

        cert = QSslCertificate.fromPath(sslcert)[0]

        self.assertTrue(bool(self.storage.capabilities() & QgsAuthConfigurationStorage.Capability.ReadSslCertificateCustomConfig))
        self.assertTrue(bool(self.storage.capabilities() & QgsAuthConfigurationStorage.Capability.DeleteSslCertificateCustomConfig))
        self.assertTrue(bool(self.storage.capabilities() & QgsAuthConfigurationStorage.Capability.UpdateSslCertificateCustomConfig))
        self.assertTrue(bool(self.storage.capabilities() & QgsAuthConfigurationStorage.Capability.CreateSslCertificateCustomConfig))
        self.assertTrue(bool(self.storage.capabilities() & QgsAuthConfigurationStorage.Capability.ClearStorage))

        # Create a custom config
        config = QgsAuthConfigSslServer()
        config.setSslCertificate(cert)
        config.setSslHostPort('localhost:5432')

        self.assertTrue(self.storage.storeSslCertCustomConfig(config))

        ids = self.storage.sslCertCustomConfigIds()
        cert_id = ids[0]
        self.assertTrue(self.storage.sslCertCustomConfigExists(cert_id, config.sslHostPort()))

        # Read it back
        configback = self.storage.loadSslCertCustomConfig(cert_id, config.sslHostPort())

        # Verify the config
        self.assertEqual(config.sslCertificate().toPem(), configback.sslCertificate().toPem())
        self.assertEqual(config.sslHostPort(), configback.sslHostPort())

        # Remove custom config
        self.assertTrue(self.storage.removeSslCertCustomConfig(cert_id, config.sslHostPort()))

        # Check it's empty
        self.assertFalse(self.storage.sslCertCustomConfigExists(cert_id, config.sslHostPort()))
