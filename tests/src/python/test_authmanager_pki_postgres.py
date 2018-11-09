# -*- coding: utf-8 -*-
"""
Tests for auth manager PKI access to postgres.

This is an integration test for QGIS Desktop Auth Manager postgres provider that
checks if QGIS can use a stored auth manager auth configuration to access
a PKI protected postgres.

Configuration from the environment:

    * QGIS_POSTGRES_SERVER_PORT (default: 55432)
    * QGIS_POSTGRES_EXECUTABLE_PATH (default: /usr/lib/postgresql/9.4/bin)


From build dir, run: ctest -R PyQgsAuthManagerPKIPostgresTest -V

or, if your PostgreSQL path differs from the default:

QGIS_POSTGRES_EXECUTABLE_PATH=/usr/lib/postgresql/<your_version_goes_here>/bin \
    ctest -R PyQgsAuthManagerPKIPostgresTest -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
import os
import time
import signal
import stat
import subprocess
import tempfile
import glob

from shutil import rmtree

from utilities import unitTestDataPath
from qgis.core import (
    QgsApplication,
    QgsAuthManager,
    QgsAuthMethodConfig,
    QgsVectorLayer,
    QgsDataSourceUri,
    QgsWkbTypes,
)

from qgis.PyQt.QtNetwork import QSslCertificate
from qgis.PyQt.QtCore import QFile

from qgis.testing import (
    start_app,
    unittest,
)


__author__ = 'Alessandro Pasotti'
__date__ = '25/10/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

QGIS_POSTGRES_SERVER_PORT = os.environ.get('QGIS_POSTGRES_SERVER_PORT', '55432')
QGIS_POSTGRES_EXECUTABLE_PATH = os.environ.get('QGIS_POSTGRES_EXECUTABLE_PATH', '/usr/lib/postgresql/9.4/bin')

assert os.path.exists(QGIS_POSTGRES_EXECUTABLE_PATH)

QGIS_AUTH_DB_DIR_PATH = tempfile.mkdtemp()

# Postgres test path
QGIS_PG_TEST_PATH = tempfile.mkdtemp()

os.environ['QGIS_AUTH_DB_DIR_PATH'] = QGIS_AUTH_DB_DIR_PATH

qgis_app = start_app()

QGIS_POSTGRES_CONF_TEMPLATE = """
hba_file = '%(tempfolder)s/pg_hba.conf'
listen_addresses = '*'
port = %(port)s
max_connections = 100
unix_socket_directories = '%(tempfolder)s'
ssl = true
ssl_ciphers = 'DEFAULT:!LOW:!EXP:!MD5:@STRENGTH'	# allowed SSL ciphers
ssl_cert_file = '%(server_cert)s'
ssl_key_file = '%(server_key)s'
ssl_ca_file = '%(sslrootcert_path)s'
password_encryption = on
"""

QGIS_POSTGRES_HBA_TEMPLATE = """
hostssl    all           all             0.0.0.0/0              cert clientcert=1
hostssl    all           all             ::1/0                  cert clientcert=1
host       all           all             127.0.0.1/32           trust
host       all           all             ::1/32                 trust
"""


class TestAuthManager(unittest.TestCase):

    @classmethod
    def setUpAuth(cls):
        """Run before all tests and set up authentication"""
        authm = QgsApplication.authManager()
        assert (authm.setMasterPassword('masterpassword', True))
        cls.pg_conf = os.path.join(cls.tempfolder, 'postgresql.conf')
        cls.pg_hba = os.path.join(cls.tempfolder, 'pg_hba.conf')
        # Client side
        cls.sslrootcert_path = os.path.join(cls.certsdata_path, 'chains_subissuer-issuer-root_issuer2-root2.pem')
        cls.sslcert = os.path.join(cls.certsdata_path, 'gerardus_cert.pem')
        cls.sslkey = os.path.join(cls.certsdata_path, 'gerardus_key.pem')
        assert os.path.isfile(cls.sslcert)
        assert os.path.isfile(cls.sslkey)
        assert os.path.isfile(cls.sslrootcert_path)
        os.chmod(cls.sslcert, stat.S_IRUSR)
        os.chmod(cls.sslkey, stat.S_IRUSR)
        os.chmod(cls.sslrootcert_path, stat.S_IRUSR)
        cls.auth_config = QgsAuthMethodConfig("PKI-Paths")
        cls.auth_config.setConfig('certpath', cls.sslcert)
        cls.auth_config.setConfig('keypath', cls.sslkey)
        cls.auth_config.setName('test_pki_auth_config')
        cls.username = 'Gerardus'
        cls.sslrootcert = QSslCertificate.fromPath(cls.sslrootcert_path)
        assert cls.sslrootcert is not None
        authm.storeCertAuthorities(cls.sslrootcert)
        authm.rebuildCaCertsCache()
        authm.rebuildTrustedCaCertsCache()
        authm.rebuildCertTrustCache()
        assert (authm.storeAuthenticationConfig(cls.auth_config)[0])
        assert cls.auth_config.isValid()

        # Server side
        cls.server_cert = os.path.join(cls.certsdata_path, 'localhost_ssl_cert.pem')
        cls.server_key = os.path.join(cls.certsdata_path, 'localhost_ssl_key.pem')
        cls.server_rootcert = cls.sslrootcert_path
        os.chmod(cls.server_cert, stat.S_IRUSR)
        os.chmod(cls.server_key, stat.S_IRUSR)
        os.chmod(cls.server_rootcert, stat.S_IRUSR)

        # Place conf in the data folder
        with open(cls.pg_conf, 'w+') as f:
            f.write(QGIS_POSTGRES_CONF_TEMPLATE % {
                'port': cls.port,
                'tempfolder': cls.tempfolder,
                'server_cert': cls.server_cert,
                'server_key': cls.server_key,
                'sslrootcert_path': cls.sslrootcert_path,
            })

        with open(cls.pg_hba, 'w+') as f:
            f.write(QGIS_POSTGRES_HBA_TEMPLATE)

    @classmethod
    def setUpClass(cls):
        """Run before all tests:
        Creates an auth configuration"""
        cls.port = QGIS_POSTGRES_SERVER_PORT
        cls.dbname = 'test_pki'
        cls.tempfolder = QGIS_PG_TEST_PATH
        cls.certsdata_path = os.path.join(unitTestDataPath('auth_system'), 'certs_keys')
        cls.hostname = 'localhost'
        cls.data_path = os.path.join(cls.tempfolder, 'data')
        os.mkdir(cls.data_path)

        cls.setUpAuth()
        subprocess.check_call([os.path.join(QGIS_POSTGRES_EXECUTABLE_PATH, 'initdb'), '-D', cls.data_path])

        cls.server = subprocess.Popen([os.path.join(QGIS_POSTGRES_EXECUTABLE_PATH, 'postgres'), '-D',
                                       cls.data_path, '-c',
                                       "config_file=%s" % cls.pg_conf],
                                      env=os.environ,
                                      stdout=subprocess.PIPE,
                                      stderr=subprocess.PIPE)
        # Wait max 10 secs for the server to start
        end = time.time() + 10
        while True:
            line = cls.server.stderr.readline()
            print(line)
            if line.find(b"database system is ready to accept") != -1:
                break
            if time.time() > end:
                raise Exception("Timeout connecting to PostgreSQL")
        # Create a DB
        subprocess.check_call([os.path.join(QGIS_POSTGRES_EXECUTABLE_PATH, 'createdb'), '-h', 'localhost', '-p', cls.port, 'test_pki'])
        # Inject test SQL from test path
        test_sql = os.path.join(unitTestDataPath('provider'), 'testdata_pg.sql')
        subprocess.check_call([os.path.join(QGIS_POSTGRES_EXECUTABLE_PATH, 'psql'), '-h', 'localhost', '-p', cls.port, '-f', test_sql, cls.dbname])
        # Create a role
        subprocess.check_call([os.path.join(QGIS_POSTGRES_EXECUTABLE_PATH, 'psql'), '-h', 'localhost', '-p', cls.port, '-c', 'CREATE ROLE "%s" WITH SUPERUSER LOGIN' % cls.username, cls.dbname])

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        cls.server.terminate()
        os.kill(cls.server.pid, signal.SIGABRT)
        del cls.server
        time.sleep(2)
        rmtree(QGIS_AUTH_DB_DIR_PATH)
        rmtree(cls.tempfolder)

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    @classmethod
    def _getPostGISLayer(cls, type_name, layer_name=None, authcfg=None):
        """
        PG layer factory
        """
        if layer_name is None:
            layer_name = 'pg_' + type_name
        uri = QgsDataSourceUri()
        uri.setWkbType(QgsWkbTypes.Point)
        uri.setConnection("localhost", cls.port, cls.dbname, "", "", QgsDataSourceUri.SslVerifyFull, authcfg)
        uri.setKeyColumn('pk')
        uri.setSrid('EPSG:4326')
        uri.setDataSource('qgis_test', 'someData', "geom", "", "pk")
        # Note: do not expand here!
        layer = QgsVectorLayer(uri.uri(False), layer_name, 'postgres')
        return layer

    def testValidAuthAccess(self):
        """
        Access the protected layer with valid credentials
        """
        pg_layer = self._getPostGISLayer('testlayer_èé', authcfg=self.auth_config.id())
        self.assertTrue(pg_layer.isValid())

    def testInvalidAuthAccess(self):
        """
        Access the protected layer with not valid credentials
        """
        pg_layer = self._getPostGISLayer('testlayer_èé')
        self.assertFalse(pg_layer.isValid())

    def testRemoveTemporaryCerts(self):
        """
        Check that no temporary cert remain after connection with
        postgres provider
        """
        def cleanTempPki():
            pkies = glob.glob(os.path.join(tempfile.gettempdir(), 'tmp*_{*}.pem'))
            for fn in pkies:
                f = QFile(fn)
                f.setPermissions(QFile.WriteOwner)
                f.remove()

        # remove any temppki in temprorary path to check that no
        # other pki remain after connection
        cleanTempPki()
        # connect using postgres provider
        pg_layer = self._getPostGISLayer('testlayer_èé', authcfg=self.auth_config.id())
        self.assertTrue(pg_layer.isValid())
        # do test no certs remained
        pkies = glob.glob(os.path.join(tempfile.gettempdir(), 'tmp*_{*}.pem'))
        self.assertEqual(len(pkies), 0)


if __name__ == '__main__':
    unittest.main()
