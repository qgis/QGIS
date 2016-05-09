# -*- coding: utf-8 -*-
"""QGIS Unit tests for bindings to core authentication system classes

From build dir: ctest -R PyQgsAuthenticationSystem -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Larry Shaffer'
__date__ = '2014/11/05'
__copyright__ = 'Copyright 2014, Boundless Spatial, Inc.'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
import tempfile

from qgis.core import QgsAuthManager, QgsAuthCertUtils, QgsPkiBundle, QgsAuthMethodConfig, QgsAuthMethod, QgsAuthConfigSslServer
from qgis.gui import QgsAuthEditorWidgets


from qgis.PyQt.QtCore import QFileInfo, qDebug
from qgis.PyQt.QtWidgets import QDialog, QVBoxLayout, QDialogButtonBox
from qgis.PyQt.QtTest import QTest
from qgis.PyQt.QtNetwork import QSsl, QSslError, QSslSocket

from qgis.testing import (
    start_app,
    unittest,
    unitTestDataPath,
)

AUTHDBDIR = tempfile.mkdtemp()
os.environ['QGIS_AUTH_DB_DIR_PATH'] = AUTHDBDIR
start_app()

TESTDATA = os.path.join(unitTestDataPath(), 'auth_system')
PKIDATA = os.path.join(TESTDATA, 'certs_keys')


class TestQgsAuthManager(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.authm = QgsAuthManager.instance()
        assert not cls.authm.isDisabled(), cls.authm.disabledMessage()

        cls.mpass = 'pass'  # master password

        db1 = QFileInfo(cls.authm.authenticationDbPath()).canonicalFilePath()
        db2 = QFileInfo(AUTHDBDIR + '/qgis-auth.db').canonicalFilePath()
        msg = 'Auth db temp path does not match db path of manager'
        assert db1 == db2, msg

    def setUp(self):
        testid = self.id().split('.')
        testheader = '\n#####_____ {0}.{1} _____#####\n'. \
            format(testid[1], testid[2])
        qDebug(testheader)

        if (not self.authm.masterPasswordIsSet()
                or not self.authm.masterPasswordHashInDb()):
            self.set_master_password()

    def widget_dialog(self, widget):
        dlg = QDialog()
        widget.setParent(dlg)
        layout = QVBoxLayout()
        layout.addWidget(widget)
        layout.setMargin(6)
        button_box = QDialogButtonBox(QDialogButtonBox.Close)
        button_box.rejected.connect(dlg.close)
        layout.addWidget(button_box)
        dlg.setLayout(layout)
        return dlg

    def show_editors_widget(self):
        editors = QgsAuthEditorWidgets()
        dlg = self.widget_dialog(editors)
        dlg.exec_()

    def set_master_password(self):
        msg = 'Failed to store and verify master password in auth db'
        assert self.authm.setMasterPassword(self.mpass, True), msg

    def test_010_master_password(self):
        msg = 'Master password is not set'
        self.assertTrue(self.authm.masterPasswordIsSet(), msg)

        msg = 'Master password hash is not in database'
        self.assertTrue(self.authm.masterPasswordHashInDb(), msg)

        msg = 'Master password not verified against hash in database'
        self.assertTrue(self.authm.verifyMasterPassword(), msg)

        msg = 'Master password comparison dissimilar'
        self.assertTrue(self.authm.masterPasswordSame(self.mpass), msg)

        msg = 'Master password not unset'
        self.authm.clearMasterPassword()
        self.assertFalse(self.authm.masterPasswordIsSet(), msg)

        msg = 'Master password not reset and validated'
        self.assertTrue(self.authm.setMasterPassword(self.mpass, True), msg)

        # NOTE: reset of master password is in auth db test unit

    def test_020_cert_utilities(self):
        pass

    def test_030_auth_settings(self):
        pass

    def test_040_authorities(self):

        def rebuild_caches():
            m = 'Authorities cache could not be rebuilt'
            self.assertTrue(self.authm.rebuildCaCertsCache(), m)

            m = 'Authorities trust policy cache could not be rebuilt'
            self.assertTrue(self.authm.rebuildTrustedCaCertsCache(), m)

        def trusted_ca_certs():
            tr_certs = self.authm.getTrustedCaCerts()
            m = 'Trusted authorities cache is empty'
            self.assertIsNotNone(tr_certs, m)
            return tr_certs

        msg = 'No system root CAs'
        self.assertIsNotNone(self.authm.getSystemRootCAs())

        # TODO: add more tests
        full_chain = 'chains_subissuer-issuer-root_issuer2-root2.pem'
        full_chain_path = os.path.join(PKIDATA, full_chain)

        # load CA file authorites for later comaprison
        # noinspection PyTypeChecker
        # ca_certs = QSslCertificate.fromPath(full_chain_path)
        ca_certs = QgsAuthCertUtils.certsFromFile(full_chain_path)
        msg = 'Authorities file could not be parsed'
        self.assertIsNotNone(ca_certs, msg)

        msg = 'Authorities file parsed count is incorrect'
        self.assertEqual(len(ca_certs), 5, msg)

        # first test CA file can be set and loaded
        msg = 'Authority file path setting could not be stored'
        self.assertTrue(
            self.authm.storeAuthSetting('cafile', full_chain_path), msg)

        msg = "Authority file 'allow invalids' setting could not be stored"
        self.assertTrue(
            self.authm.storeAuthSetting('cafileallowinvalid', False), msg)

        rebuild_caches()
        trusted_certs = trusted_ca_certs()

        not_cached = any([ca not in trusted_certs for ca in ca_certs])
        msg = 'Authorities not in trusted authorities cache'
        self.assertFalse(not_cached, msg)

        # test CA file can be unset
        msg = 'Authority file path setting could not be removed'
        self.assertTrue(self.authm.removeAuthSetting('cafile'), msg)

        msg = "Authority file 'allow invalids' setting could not be removed"
        self.assertTrue(
            self.authm.removeAuthSetting('cafileallowinvalid'), msg)

        rebuild_caches()
        trusted_certs = trusted_ca_certs()

        still_cached = any([ca in trusted_certs for ca in ca_certs])
        msg = 'Authorities still in trusted authorities cache'
        self.assertFalse(still_cached, msg)

        # test CAs can be stored in database
        msg = "Authority certs could not be stored in database"
        self.assertTrue(self.authm.storeCertAuthorities(ca_certs))

        rebuild_caches()
        trusted_certs = trusted_ca_certs()

        not_cached = any([ca not in trusted_certs for ca in ca_certs])
        msg = 'Stored authorities not in trusted authorities cache'
        self.assertFalse(not_cached, msg)

        # dlg = QgsAuthTrustedCAsDialog()
        # dlg.exec_()

    def test_050_trust_policy(self):
        pass

    # noinspection PyArgumentList
    def test_060_identities(self):
        client_cert_path = os.path.join(PKIDATA, 'fra_cert.pem')
        client_key_path = os.path.join(PKIDATA, 'fra_key_w-pass.pem')
        client_key_pass = 'password'
        client_p12_path = os.path.join(PKIDATA, 'gerardus_w-chain.p12')
        client_p12_pass = 'password'

        # store regular PEM cert/key and generate config
        # noinspection PyTypeChecker
        bundle1 = QgsPkiBundle.fromPemPaths(client_cert_path, client_key_path,
                                            client_key_pass)
        bundle1_cert = bundle1.clientCert()
        bundle1_key = bundle1.clientKey()
        bundle1_ca_chain = bundle1.caChain()
        bundle1_cert_sha = bundle1.certId()

        # with open(client_key_path, 'r') as f:
        #     key_data = f.read()
        #
        # client_cert = QgsAuthCertUtils.certsFromFile(client_cert_path)[0]
        msg = 'Identity PEM certificate is null'
        self.assertFalse(bundle1_cert.isNull(), msg)

        # cert_sha = QgsAuthCertUtils.shaHexForCert(client_cert)
        #
        # client_key = QSslKey(key_data, QSsl.Rsa, QSsl.Pem,
        #                      QSsl.PrivateKey, client_key_pass)
        msg = 'Identity PEM key is null'
        self.assertFalse(bundle1_key.isNull(), msg)

        msg = 'Identity PEM certificate chain is not empty'
        self.assertEqual(len(bundle1_ca_chain), 0, msg)

        msg = "Identity PEM could not be stored in database"
        self.assertTrue(
            self.authm.storeCertIdentity(bundle1_cert, bundle1_key), msg)

        msg = "Identity PEM not found in database"
        self.assertTrue(self.authm.existsCertIdentity(bundle1_cert_sha), msg)

        config1 = QgsAuthMethodConfig()
        config1.setName('IdentityCert - PEM')
        config1.setMethod('Identity-Cert')
        config1.setConfig('certid', bundle1_cert_sha)

        msg = 'Could not store PEM identity config'
        self.assertTrue(self.authm.storeAuthenticationConfig(config1), msg)

        configid1 = config1.id()
        msg = 'Could not retrieve PEM identity config id from store op'
        self.assertIsNotNone(configid1, msg)

        config2 = QgsAuthMethodConfig()
        msg = 'Could not load PEM identity config'
        self.assertTrue(
            self.authm.loadAuthenticationConfig(configid1, config2, True),
            msg)

        # store PKCS#12 bundled cert/key and generate config
        # bundle = QgsPkcsBundle(client_p12_path, client_p12_pass)
        # noinspection PyTypeChecker
        bundle = QgsPkiBundle.fromPkcs12Paths(client_p12_path, client_p12_pass)
        bundle_cert = bundle.clientCert()
        bundle_key = bundle.clientKey()
        bundle_ca_chain = bundle.caChain()
        bundle_cert_sha = QgsAuthCertUtils.shaHexForCert(bundle_cert)

        msg = 'Identity bundle certificate is null'
        self.assertFalse(bundle_cert.isNull(), msg)

        msg = 'Identity bundle key is null'
        self.assertFalse(bundle_key.isNull(), msg)

        msg = 'Identity bundle CA chain is not correct depth'
        self.assertEqual(len(bundle_ca_chain), 3, msg)

        msg = "Identity bundle could not be stored in database"
        self.assertTrue(
            self.authm.storeCertIdentity(bundle_cert, bundle_key), msg)

        msg = "Identity bundle not found in database"
        self.assertTrue(self.authm.existsCertIdentity(bundle_cert_sha), msg)

        bundle_config = QgsAuthMethodConfig()
        bundle_config.setName('IdentityCert - Bundle')
        bundle_config.setMethod('Identity-Cert')
        bundle_config.setConfig('certid', bundle_cert_sha)

        msg = 'Could not store bundle identity config'
        self.assertTrue(
            self.authm.storeAuthenticationConfig(bundle_config), msg)

        bundle_configid = bundle_config.id()
        msg = 'Could not retrieve bundle identity config id from store op'
        self.assertIsNotNone(bundle_configid, msg)

        bundle_config2 = QgsAuthMethodConfig()
        msg = 'Could not load bundle identity config'
        self.assertTrue(
            self.authm.loadAuthenticationConfig(bundle_configid,
                                                bundle_config2,
                                                True),
            msg)

        # TODO: add more tests
        # self.show_editors_widget()

        msg = 'Could not remove PEM identity config'
        self.assertTrue(self.authm.removeAuthenticationConfig(configid1), msg)

        msg = 'Could not remove bundle identity config'
        self.assertTrue(
            self.authm.removeAuthenticationConfig(bundle_configid), msg)

    def test_070_servers(self):
        # return
        ssl_cert_path = os.path.join(PKIDATA, 'localhost_ssl_cert.pem')

        ssl_cert = QgsAuthCertUtils.certsFromFile(ssl_cert_path)[0]
        msg = 'SSL server certificate is null'
        self.assertFalse(ssl_cert.isNull(), msg)

        cert_sha = QgsAuthCertUtils.shaHexForCert(ssl_cert)

        hostport = 'localhost:8443'
        config = QgsAuthConfigSslServer()
        config.setSslCertificate(ssl_cert)
        config.setSslHostPort(hostport)
        config.setSslIgnoredErrorEnums([QSslError.SelfSignedCertificate])
        config.setSslPeerVerifyMode(QSslSocket.VerifyNone)
        config.setSslPeerVerifyDepth(3)
        config.setSslProtocol(QSsl.TlsV1)

        msg = 'SSL config is null'
        self.assertFalse(config.isNull(), msg)

        msg = 'Could not store SSL config'
        self.assertTrue(self.authm.storeSslCertCustomConfig(config), msg)

        msg = 'Could not verify storage of SSL config'
        self.assertTrue(
            self.authm.existsSslCertCustomConfig(cert_sha, hostport), msg)

        msg = 'Could not verify SSL config in all configs'
        self.assertIsNotNone(self.authm.getSslCertCustomConfigs(), msg)

        msg = 'Could not retrieve SSL config'
        config2 = self.authm.getSslCertCustomConfig(cert_sha, hostport)
        """:type: QgsAuthConfigSslServer"""
        self.assertFalse(config2.isNull(), msg)

        msg = 'Certificate of retrieved SSL config does not match'
        self.assertEqual(config.sslCertificate(), config2.sslCertificate(), msg)

        msg = 'HostPort of retrieved SSL config does not match'
        self.assertEqual(config.sslHostPort(), config2.sslHostPort(), msg)

        msg = 'IgnoredErrorEnums of retrieved SSL config does not match'
        enums = config2.sslIgnoredErrorEnums()
        self.assertTrue(QSslError.SelfSignedCertificate in enums, msg)

        msg = 'PeerVerifyMode of retrieved SSL config does not match'
        self.assertEqual(config.sslPeerVerifyMode(),
                         config2.sslPeerVerifyMode(), msg)

        msg = 'PeerVerifyDepth of retrieved SSL config does not match'
        self.assertEqual(config.sslPeerVerifyDepth(),
                         config2.sslPeerVerifyDepth(), msg)

        msg = 'Protocol of retrieved SSL config does not match'
        self.assertEqual(config.sslProtocol(), config2.sslProtocol(), msg)

        # dlg = QgsAuthSslConfigDialog(None, ssl_cert, hostport)
        # dlg.exec_()

        msg = 'Could not remove SSL config'
        self.assertTrue(
            self.authm.removeSslCertCustomConfig(cert_sha, hostport), msg)

        msg = 'Could not verify removal of SSL config'
        self.assertFalse(
            self.authm.existsSslCertCustomConfig(cert_sha, hostport), msg)

    def test_080_auth_configid(self):
        msg = 'Could not generate a config id'
        self.assertIsNotNone(self.authm.uniqueConfigId(), msg)

        uids = []
        for _ in xrange(50):
            # time.sleep(0.01)  # or else the salt is not random enough
            uids.append(self.authm.uniqueConfigId())
        msg = 'Generated 50 config ids are not unique:\n{0}\n{1}'.format(
            uids,
            list(set(uids))
        )
        self.assertEqual(len(uids), len(list(set(uids))), msg)

    def config_list(self):
        return ['Basic', 'PKI-Paths', 'PKI-PKCS#12']

    def config_obj(self, kind, base=True):
        config = QgsAuthMethodConfig()
        config.setName(kind)
        config.setMethod(kind)
        config.setUri('http://example.com')
        if base:
            return config

        if kind == 'Basic':
            config.setConfig('username', 'username')
            config.setConfig('password', 'password')
            config.setConfig('realm', 'Realm')
        elif kind == 'PKI-Paths':
            config.setConfig('certpath',
                             os.path.join(PKIDATA, 'gerardus_cert.pem'))
            config.setConfig('keypath',
                             os.path.join(PKIDATA, 'gerardus_key_w-pass.pem'))
            config.setConfig('keypass', 'password')
        elif kind == 'PKI-PKCS#12':
            config.setConfig('bundlepath',
                             os.path.join(PKIDATA, 'gerardus.p12'))
            config.setConfig('bundlepass', 'password')

        return config

    def config_values_valid(self, kind, config):
        """:type config: QgsAuthMethodConfig"""
        if (config.name() != kind or
                config.method() != kind or
                config.uri() != 'http://example.com'):
            return False
        if kind == 'Basic':
            return (
                config.config('username') == 'username' and
                config.config('password') == 'password' and
                config.config('realm') == 'Realm'
            )
        elif kind == 'PKI-Paths':
            return (
                config.config('certpath') ==
                os.path.join(PKIDATA, 'gerardus_cert.pem') and
                config.config('keypath') ==
                os.path.join(PKIDATA, 'gerardus_key_w-pass.pem') and
                config.config('keypass') == 'password'
            )
        elif kind == 'PKI-PKCS#12':
            return (
                config.config('bundlepath') ==
                os.path.join(PKIDATA, 'gerardus.p12') and
                config.config('bundlepass') == 'password'
            )

    def test_090_auth_configs(self):
        # these list items need to match the QgsAuthType provider type strings
        for kind in self.config_list():
            config = self.config_obj(kind, base=False)
            msg = 'Could not validate {0} config'.format(kind)
            self.assertTrue(config.isValid(), msg)

            msg = 'Could not store {0} config'.format(kind)
            self.assertTrue(self.authm.storeAuthenticationConfig(config), msg)

            configid = config.id()
            msg = 'Could not retrieve {0} config id from store op'.format(kind)
            self.assertIsNotNone(configid, msg)

            msg = 'Config id {0} not in db'.format(configid)
            self.assertFalse(self.authm.configIdUnique(configid), msg)

            msg = 'Could not retrieve {0} config id from db'.format(kind)
            self.assertTrue(configid in self.authm.configIds(), msg)

            msg = 'Could not retrieve method key for {0} config'.format(kind)
            self.assertTrue(
                self.authm.configAuthMethodKey(configid) == kind, msg)

            msg = 'Could not retrieve method ptr for {0} config'.format(kind)
            self.assertTrue(
                isinstance(self.authm.configAuthMethod(configid),
                           QgsAuthMethod), msg)

            config2 = self.config_obj(kind, base=True)
            msg = 'Could not load {0} config'.format(kind)
            self.assertTrue(
                self.authm.loadAuthenticationConfig(configid, config2, True),
                msg)

            msg = 'Could not validate loaded {0} config values'.format(kind)
            self.assertTrue(self.config_values_valid(kind, config2), msg)

            # values haven't been changed, but the db update still takes place
            msg = 'Could not update {0} config values'.format(kind)
            self.assertTrue(self.authm.updateAuthenticationConfig(config2), msg)

            config3 = self.config_obj(kind, base=True)
            msg = 'Could not load updated {0} config'.format(kind)
            self.assertTrue(
                self.authm.loadAuthenticationConfig(configid, config3, True),
                msg)

            msg = 'Could not validate updated {0} config values'.format(kind)
            self.assertTrue(self.config_values_valid(kind, config3), msg)

            msg = 'Could not remove {0} config (by id) from db'.format(kind)
            self.assertTrue(
                self.authm.removeAuthenticationConfig(configid), msg)

            msg = 'Did not remove {0} config id from db'.format(kind)
            self.assertFalse(configid in self.authm.configIds(), msg)

    def test_100_auth_db(self):

        for kind in self.config_list():
            config = self.config_obj(kind, base=False)
            msg = 'Could not store {0} config'.format(kind)
            self.assertTrue(self.authm.storeAuthenticationConfig(config), msg)

        msg = 'Could not store a sample of all configs in auth db'
        self.assertTrue(
            (len(self.authm.configIds()) == len(self.config_list())), msg)

        msg = 'Could not retrieve available configs from auth db'
        self.assertTrue(len(self.authm.availableAuthMethodConfigs()) > 0, msg)

        backup = None
        resetpass, backup = self.authm.resetMasterPassword(
            'newpass', self.mpass, True, backup)
        msg = 'Could not reset master password and/or re-encrypt configs'
        self.assertTrue(resetpass, msg)

        # qDebug('Backup db path: {0}'.format(backup))
        msg = 'Could not retrieve backup path for reset master password op'
        self.assertIsNotNone(backup)
        self.assertTrue(backup != self.authm.authenticationDbPath(), msg)

        msg = 'Could not verify reset master password'
        self.assertTrue(self.authm.setMasterPassword('newpass', True), msg)

        msg = 'Could not remove all configs from auth db'
        self.assertTrue(self.authm.removeAllAuthenticationConfigs(), msg)

        msg = 'Configs were not removed from auth db'
        self.assertTrue(len(self.authm.configIds()) == 0, msg)

        msg = 'Auth db does not exist'
        self.assertTrue(os.path.exists(self.authm.authenticationDbPath()), msg)

        QTest.qSleep(1000)  # necessary for new backup to have different name

        msg = 'Could not erase auth db'
        backup = None
        reserase, backup = \
            self.authm.eraseAuthenticationDatabase(True, backup)
        self.assertTrue(reserase, msg)

        # qDebug('Erase db backup db path: {0}'.format(backup))
        msg = 'Could not retrieve backup path for erase db op'
        self.assertIsNotNone(backup)
        self.assertTrue(backup != self.authm.authenticationDbPath(), msg)

        msg = 'Master password not erased from auth db'
        self.assertTrue(not self.authm.masterPasswordIsSet()
                        and not self.authm.masterPasswordHashInDb(), msg)

        self.set_master_password()


if __name__ == '__main__':
    unittest.main()
