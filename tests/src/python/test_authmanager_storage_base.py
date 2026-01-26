"""
Base tests for auth manager storage API

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os
import unittest
from osgeo import gdal
from qgis.PyQt.QtCore import QCoreApplication, QTemporaryDir
from qgis.PyQt.QtNetwork import QSslCertificate
from qgis.core import (
    Qgis,
    QgsApplication,
    QgsSettings,
    QgsAuthConfigurationStorage,
    QgsAuthConfigurationStorageDb,
    QgsAuthMethodConfig,
    QgsAuthConfigSslServer,
    QgsAuthCertUtils,
    QgsNotSupportedException,
)
from qgis.testing import start_app, QgisTestCase
from utilities import unitTestDataPath

__author__ = "Alessandro Pasotti"
__date__ = "2024-06-24"
__copyright__ = "Copyright 2024, The QGIS Project"


class AuthManagerStorageBaseTestCase(QgisTestCase):

    # This must be populated by the derived class and be an instance of QgsAuthConfigurationStorage
    storage = None
    # If not None, it will be used to test the storage integration with QgsAuthManager
    storage_uri = None

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()

        if cls.storage_uri is not None:
            os.environ["QGIS_AUTH_DB_URI"] = cls.storage_uri

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("%s.com" % __name__)
        QCoreApplication.setApplicationName(__name__)
        QgsSettings().clear()
        start_app()

        cls.certdata_path = os.path.join(
            unitTestDataPath("auth_system"), "certs_keys_2048"
        )


class TestAuthManagerStorageBase:

    def testInitialized(self):

        assert self.storage is not None
        self.assertTrue(self.storage.initialize())
        if issubclass(type(self.storage), QgsAuthConfigurationStorageDb):
            for table in [
                "auth_authorities",
                "auth_identities",
                "auth_servers",
                "auth_settings",
                "auth_trust",
                "auth_configs",
            ]:
                self.assertTrue(self.storage.tableExists(table))

    def testDefaultStorage(self):

        if self.storage_uri is None:
            raise unittest.SkipTest("No storage URI defined")

        auth_manager = QgsApplication.authManager()
        auth_manager.ensureInitialized()

        # Verify that the default storage is the one we set
        self.assertEqual(auth_manager.authenticationDatabaseUri(), self.storage_uri)

        # Verify that the registry has the storage
        registry = QgsApplication.authConfigurationStorageRegistry()
        storage = registry.readyStorages()[0]
        self.assertEqual(
            storage.settings()["database"], self.storage.settings()["database"]
        )
        self.assertEqual(
            storage.settings()["driver"], self.storage.settings()["driver"]
        )

    def _assert_readonly(self, storage):

        self.assertTrue(storage.isReadOnly())

        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.ReadConfiguration
            )
        )
        self.assertFalse(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.DeleteConfiguration
            )
        )
        self.assertFalse(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.UpdateConfiguration
            )
        )
        self.assertFalse(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.CreateConfiguration
            )
        )

        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.ReadSetting
            )
        )
        self.assertFalse(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.DeleteSetting
            )
        )
        self.assertFalse(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.UpdateSetting
            )
        )
        self.assertFalse(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.CreateSetting
            )
        )

        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.ReadCertificateIdentity
            )
        )
        self.assertFalse(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.DeleteCertificateIdentity
            )
        )
        self.assertFalse(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.UpdateCertificateIdentity
            )
        )
        self.assertFalse(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.CreateCertificateIdentity
            )
        )

        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.ReadSslCertificateCustomConfig
            )
        )
        self.assertFalse(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.DeleteSslCertificateCustomConfig
            )
        )
        self.assertFalse(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.UpdateSslCertificateCustomConfig
            )
        )
        self.assertFalse(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.CreateSslCertificateCustomConfig
            )
        )

        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.ReadCertificateTrustPolicy
            )
        )
        self.assertFalse(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.DeleteCertificateTrustPolicy
            )
        )
        self.assertFalse(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.UpdateCertificateTrustPolicy
            )
        )
        self.assertFalse(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.CreateCertificateTrustPolicy
            )
        )

        self.assertFalse(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.ClearStorage
            )
        )

        # Checks that calling a RO method raises an QgsNotSupportedException
        with self.assertRaises(QgsNotSupportedException):
            config = QgsAuthMethodConfig()
            storage.storeMethodConfig(config, "test")

    def _assert_readwrite(self, storage):

        self.assertFalse(storage.isReadOnly())

        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.ReadConfiguration
            )
        )
        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.DeleteConfiguration
            )
        )
        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.UpdateConfiguration
            )
        )
        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.CreateConfiguration
            )
        )

        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.ReadSetting
            )
        )
        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.DeleteSetting
            )
        )
        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.UpdateSetting
            )
        )
        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.CreateSetting
            )
        )

        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.ReadCertificateIdentity
            )
        )
        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.DeleteCertificateIdentity
            )
        )
        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.UpdateCertificateIdentity
            )
        )
        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.CreateCertificateIdentity
            )
        )

        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.ReadSslCertificateCustomConfig
            )
        )
        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.DeleteSslCertificateCustomConfig
            )
        )
        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.UpdateSslCertificateCustomConfig
            )
        )
        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.CreateSslCertificateCustomConfig
            )
        )

        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.ReadCertificateTrustPolicy
            )
        )
        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.DeleteCertificateTrustPolicy
            )
        )
        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.UpdateCertificateTrustPolicy
            )
        )
        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.CreateCertificateTrustPolicy
            )
        )

        self.assertTrue(
            bool(
                storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.ClearStorage
            )
        )

    def testAuthStoragePermissions(self):
        """Checks that the auth manager default DB storage permissions are set correctly"""

        auth_manager = QgsApplication.authManager()
        auth_manager.ensureInitialized()
        registry = QgsApplication.authConfigurationStorageRegistry()
        storage = registry.readyStorages()[0]

        if not auth_manager.isFilesystemBasedDatabase(self.storage_uri):
            self._assert_readonly(storage)
            storage.setReadOnly(False)
            self._assert_readwrite(storage)
        else:
            self._assert_readwrite(storage)
            storage.setReadOnly(True)
            self._assert_readonly(storage)

    def testAuthConfigs(self):

        assert self.storage is not None

        self.assertTrue(self.storage.initialize(), self.storage.lastError())

        if not bool(
            self.storage.capabilities()
            & Qgis.AuthConfigurationStorageCapability.ReadConfiguration
        ):
            raise unittest.SkipTest("Storage does not support reading configurations")

        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.ReadConfiguration
            )
        )
        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.DeleteConfiguration
            )
        )
        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.UpdateConfiguration
            )
        )
        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.CreateConfiguration
            )
        )
        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.ClearStorage
            )
        )

        # Create a configuration
        config = QgsAuthMethodConfig()
        config.setId("test")
        config.setName("test name")
        config.setMethod("basic")
        config.setConfig("username", "test user")
        config.setConfig("password", "test pass")
        config.setConfig("realm", "test realm")
        payload = config.configString()
        self.assertTrue(self.storage.storeMethodConfig(config, payload))

        # Create another configuration
        config2 = QgsAuthMethodConfig()
        config2.setId("test2")
        config2.setName("test name 2")
        config2.setMethod("basic")
        config2.setConfig("username", "test user 2")
        config2.setConfig("password", "test pass 2")
        config2.setConfig("realm", "test realm 2")
        payload2 = config2.configString()
        self.assertTrue(self.storage.storeMethodConfig(config2, payload2))

        # Test exists
        self.assertTrue(self.storage.methodConfigExists("test"))
        self.assertFalse(self.storage.methodConfigExists("xxxx"))

        # Read it back
        configback, payloadback = self.storage.loadMethodConfig("test", True)
        configback.loadConfigString(payloadback)
        self.assertEqual(configback.id(), "test")
        self.assertEqual(configback.name(), "test name")
        self.assertEqual(configback.method(), "basic")
        self.assertEqual(configback.config("username"), "test user")
        self.assertEqual(configback.config("password"), "test pass")
        self.assertEqual(configback.config("realm"), "test realm")
        self.assertEqual(payloadback, payload)

        configback, payloadback = self.storage.loadMethodConfig("test", False)
        self.assertEqual(payloadback, "")
        self.assertEqual(configback.id(), "test")
        self.assertEqual(configback.name(), "test name")
        self.assertEqual(configback.method(), "basic")
        self.assertEqual(configback.config("username"), "")
        self.assertEqual(configback.config("password"), "")
        self.assertEqual(configback.config("realm"), "")

        configs = self.storage.authMethodConfigs(["xxxx"])
        self.assertEqual(len(configs), 0)

        configs = self.storage.authMethodConfigs(["basic"])
        self.assertEqual(len(configs), 2)

        configs = self.storage.authMethodConfigsWithPayload()
        configback = configs["test"]
        payloadback = configback.config("encrypted_payload")
        configback.loadConfigString(payloadback)
        self.assertEqual(configback.id(), "test")
        self.assertEqual(configback.name(), "test name")
        self.assertEqual(configback.method(), "basic")
        self.assertEqual(configback.config("username"), "test user")
        self.assertEqual(configback.config("password"), "test pass")
        self.assertEqual(configback.config("realm"), "test realm")
        self.assertEqual(payloadback, payload)

        # Remove method config
        self.assertTrue(self.storage.removeMethodConfig("test2"))
        configs = self.storage.authMethodConfigs(["basic"])
        self.assertEqual(len(configs), 1)

        # Clear the storage
        self.assertTrue(self.storage.clearMethodConfigs())

        # Check it's empty
        configs = self.storage.authMethodConfigsWithPayload()
        self.assertEqual(len(configs), 0)

    def testAuthSettings(self):

        assert self.storage is not None

        self.assertTrue(self.storage.initialize(), self.storage.lastError())

        if not bool(
            self.storage.capabilities()
            & Qgis.AuthConfigurationStorageCapability.ReadSetting
        ):
            raise unittest.SkipTest("Storage does not support reading settings")

        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.ReadSetting
            )
        )
        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.DeleteSetting
            )
        )
        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.UpdateSetting
            )
        )
        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.CreateSetting
            )
        )
        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.ClearStorage
            )
        )

        # Create a setting
        self.assertTrue(self.storage.storeAuthSetting("test", "test value"))

        # Create another setting
        self.assertTrue(self.storage.storeAuthSetting("test2", "test value 2"))

        self.assertTrue(self.storage.authSettingExists("test"))
        self.assertTrue(self.storage.authSettingExists("test2"))
        self.assertFalse(self.storage.authSettingExists("xxxx"))

        # Read it back
        value = self.storage.loadAuthSetting("test")
        self.assertEqual(value, "test value")

        # Remove setting
        self.assertTrue(self.storage.removeAuthSetting("test"))
        self.assertTrue(self.storage.removeAuthSetting("test2"))

        # Check it's empty
        self.assertFalse(self.storage.authSettingExists("test"))
        self.assertFalse(self.storage.authSettingExists("test2"))

    def testCertIdentity(self):

        assert self.storage is not None

        self.assertTrue(self.storage.initialize(), self.storage.lastError())

        if not bool(
            self.storage.capabilities()
            & Qgis.AuthConfigurationStorageCapability.ReadCertificateIdentity
        ):
            raise unittest.SkipTest(
                "Storage does not support reading certificate identities"
            )

        sslrootcert_path = os.path.join(self.certdata_path, "qgis_ca.crt")
        sslcert = os.path.join(self.certdata_path, "Gerardus.crt")
        sslkey_path = os.path.join(self.certdata_path, "Gerardus.key")
        # In real life, key should be encrypted (the auth manager does that)
        with open(sslkey_path) as f:
            sslkey = f.read()

        cert = QSslCertificate.fromPath(sslcert)[0]

        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.ReadCertificateIdentity
            )
        )
        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.DeleteCertificateIdentity
            )
        )
        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.UpdateCertificateIdentity
            )
        )
        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.CreateCertificateIdentity
            )
        )
        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.ClearStorage
            )
        )

        # Create an identity
        self.assertTrue(self.storage.storeCertIdentity(cert, sslkey))

        ids = self.storage.certIdentityIds()
        cert_id = ids[0]
        self.assertTrue(self.storage.certIdentityExists(cert_id))

        # Read it back
        certback = self.storage.loadCertIdentity(cert_id)

        # Verify the certificate
        self.assertEqual(cert.toPem(), certback.toPem())

        # Read bundle
        bundle = self.storage.loadCertIdentityBundle(cert_id)

        # Verify the certificate
        self.assertEqual(cert.toPem(), bundle[0].toPem())

        # verify the key
        self.assertEqual(sslkey, bundle[1])

        # Remove identity
        self.assertTrue(self.storage.removeCertIdentity(cert_id))

        # Check it's empty
        self.assertFalse(self.storage.certIdentityExists(cert_id))

    def testSslCertificateCustomConfig(self):

        assert self.storage is not None

        self.assertTrue(self.storage.initialize(), self.storage.lastError())

        if not bool(
            self.storage.capabilities()
            & Qgis.AuthConfigurationStorageCapability.ReadSslCertificateCustomConfig
        ):
            raise unittest.SkipTest(
                "Storage does not support reading ssl certificate custom configs"
            )

        sslrootcert_path = os.path.join(self.certdata_path, "qgis_ca.crt")
        sslcert = os.path.join(self.certdata_path, "Gerardus.crt")
        sslkey_path = os.path.join(self.certdata_path, "Gerardus.key")
        # In real life, key should be encrypted (the auth manager does that)
        with open(sslkey_path) as f:
            sslkey = f.read()

        cert = QSslCertificate.fromPath(sslcert)[0]

        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.ReadSslCertificateCustomConfig
            )
        )
        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.DeleteSslCertificateCustomConfig
            )
        )
        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.UpdateSslCertificateCustomConfig
            )
        )
        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.CreateSslCertificateCustomConfig
            )
        )
        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.ClearStorage
            )
        )

        # Create a custom config
        config = QgsAuthConfigSslServer()
        config.setSslCertificate(cert)
        config.setSslHostPort("localhost:5432")

        self.assertTrue(self.storage.storeSslCertCustomConfig(config))

        ids = self.storage.sslCertCustomConfigIds()
        cert_id = ids[0]
        self.assertTrue(
            self.storage.sslCertCustomConfigExists(cert_id, config.sslHostPort())
        )

        # Read it back
        configback = self.storage.loadSslCertCustomConfig(cert_id, config.sslHostPort())

        # Verify the config
        self.assertEqual(
            config.sslCertificate().toPem(), configback.sslCertificate().toPem()
        )
        self.assertEqual(config.sslHostPort(), configback.sslHostPort())

        # Remove custom config
        self.assertTrue(
            self.storage.removeSslCertCustomConfig(cert_id, config.sslHostPort())
        )

        # Check it's empty
        self.assertFalse(
            self.storage.sslCertCustomConfigExists(cert_id, config.sslHostPort())
        )

    def testTrust(self):

        assert self.storage is not None

        self.assertTrue(self.storage.initialize(), self.storage.lastError())

        if not bool(
            self.storage.capabilities()
            & Qgis.AuthConfigurationStorageCapability.ReadCertificateTrustPolicy
        ):
            raise unittest.SkipTest(
                "Storage does not support reading certificate trust policies"
            )

        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.ReadCertificateTrustPolicy
            )
        )
        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.DeleteCertificateTrustPolicy
            )
        )
        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.UpdateCertificateTrustPolicy
            )
        )
        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.CreateCertificateTrustPolicy
            )
        )
        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.ClearStorage
            )
        )

        sslrootcert_path = os.path.join(self.certdata_path, "qgis_ca.crt")
        sslcert = os.path.join(self.certdata_path, "Gerardus.crt")
        sslkey_path = os.path.join(self.certdata_path, "Gerardus.key")
        # In real life, key should be encrypted (the auth manager does that)
        with open(sslkey_path) as f:
            sslkey = f.read()

        cert = QSslCertificate.fromPath(sslcert)[0]
        rootcert = QSslCertificate.fromPath(sslrootcert_path)[0]

        policy = QgsAuthCertUtils.CertTrustPolicy.Trusted

        # Create a trust
        self.assertTrue(self.storage.storeCertTrustPolicy(cert, policy))

        self.assertTrue(self.storage.certTrustPolicyExists(cert))

        # Read it back
        trustback = self.storage.loadCertTrustPolicy(cert)

        # Verify the trust
        self.assertEqual(trustback, policy)

        # Remove trust
        self.assertTrue(self.storage.removeCertTrustPolicy(cert))

        # Check it's empty
        self.assertFalse(self.storage.certTrustPolicyExists(cert))

    def testAuthority(self):

        assert self.storage is not None

        self.assertTrue(self.storage.initialize(), self.storage.lastError())

        if not bool(
            self.storage.capabilities()
            & Qgis.AuthConfigurationStorageCapability.ReadCertificateAuthority
        ):
            raise unittest.SkipTest(
                "Storage does not support reading certificate authorities"
            )

        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.ReadCertificateAuthority
            )
        )
        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.DeleteCertificateAuthority
            )
        )
        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.UpdateCertificateAuthority
            )
        )
        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.CreateCertificateAuthority
            )
        )
        self.assertTrue(
            bool(
                self.storage.capabilities()
                & Qgis.AuthConfigurationStorageCapability.ClearStorage
            )
        )

        sslrootcert_path = os.path.join(self.certdata_path, "qgis_ca.crt")
        rootcert = QSslCertificate.fromPath(sslrootcert_path)[0]

        # Create an authority
        self.assertTrue(self.storage.storeCertAuthority(rootcert))

        ids = self.storage.certAuthorityIds()
        authority_id = ids[0]

        self.assertTrue(self.storage.certAuthorityExists(rootcert))

        # Read it back
        certback = self.storage.loadCertAuthority(authority_id)

        # Verify the certificate
        self.assertEqual(rootcert.toPem(), certback.toPem())

        # Remove authority
        self.assertTrue(self.storage.removeCertAuthority(rootcert))

        # Check it's empty
        self.assertFalse(self.storage.certAuthorityExists(rootcert))

    def testUpdateReadOnly(self):
        """Tests that updating a setting in a read-only storage fails"""

        if not bool(
            self.storage.capabilities()
            & Qgis.AuthConfigurationStorageCapability.UpdateSetting
        ):
            raise unittest.SkipTest(
                "Storage does not support reading certificate authorities"
            )

        auth_manager = QgsApplication.authManager()
        auth_manager.ensureInitialized()

        temp_dir = QTemporaryDir()
        temp_dir_path = temp_dir.path()

        # Create an empty sqlite database using GDAL
        db_path = os.path.join(temp_dir_path, "test.sqlite")
        ds = gdal.GetDriverByName("SQLite").Create(db_path, 0, 0, 0)
        del ds

        uri = f"QSQLITE://{db_path}"
        tmp_storage = QgsAuthConfigurationStorageDb(uri)

        self.assertTrue(tmp_storage.initialize(), tmp_storage.lastError())

        # Add the storage to the registry
        registry = QgsApplication.authConfigurationStorageRegistry()
        self.assertTrue(registry.addStorage(tmp_storage))

        # Check we have two ready storages
        self.assertEqual(len(registry.readyStorages()), 2)

        # Create a setting
        self.assertTrue(self.storage.storeAuthSetting("test", "test value"))

        # Set the original storage as read-only
        self.storage.setReadOnly(True)

        # Try to update the setting using auth manager
        self.assertFalse(auth_manager.storeAuthSetting("test", "test value 2"))

        # Remove the temp storage
        self.assertTrue(registry.removeStorage(tmp_storage.id()))
