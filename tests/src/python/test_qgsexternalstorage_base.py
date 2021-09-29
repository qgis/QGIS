# -*- coding: utf-8 -*-
"""QGIS Base Unit tests for QgsExternalStorage API

External storage backend must implement a test based on TestPyQgsExternalStorageBase

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = 'Julien Cabieces'
__date__ = '31/03/2021'
__copyright__ = 'Copyright 2021, The QGIS Project'

from shutil import rmtree
import os
import tempfile
import time

from utilities import unitTestDataPath, waitServer

from qgis.PyQt.QtCore import QCoreApplication, QEventLoop, QUrl, QTimer
from qgis.PyQt.QtTest import QSignalSpy

from qgis.core import (
    Qgis,
    QgsApplication,
    QgsAuthMethodConfig,
    QgsExternalStorageFetchedContent)

from qgis.testing import (
    start_app,
    unittest,
)


class TestPyQgsExternalStorageBase():

    storageType = None
    url = None
    badUrl = None

    @classmethod
    def setUpClass(cls):
        """Run before all tests:"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(cls.__name__)
        QCoreApplication.setApplicationName(cls.__name__)
        start_app()

        cls.authm = QgsApplication.authManager()
        assert (cls.authm.setMasterPassword('masterpassword', True))
        assert not cls.authm.isDisabled(), cls.authm.disabledMessage()

        cls.auth_config = QgsAuthMethodConfig("Basic")
        cls.auth_config.setConfig('username', "qgis")
        cls.auth_config.setConfig('password', "myPasswd!")
        cls.auth_config.setName('test_basic_auth_config')
        assert(cls.authm.storeAuthenticationConfig(cls.auth_config)[0])
        assert cls.auth_config.isValid()

        cls.registry = QgsApplication.instance().externalStorageRegistry()
        assert cls.registry

        cls.storage = cls.registry.externalStorageFromType(cls.storageType)
        assert cls.storage

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        cls.registry.unregisterExternalStorage(cls.storage)
        assert cls.storageType not in cls.registry.externalStorages()

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    def getNewFile(self, content):
        """Return a newly created temporary file with content"""
        f = tempfile.NamedTemporaryFile(suffix='.txt')
        f.write(content)
        f.flush()
        return f

    def checkContent(self, file_path, content):
        """Check that file content matches given content"""
        f = open(file_path, 'r')
        self.assertTrue(f.read(), b"New content")
        f.close()

    def testStorageList(self):
        """
        Check that storage list in in correct order
        """
        self.assertEqual([storage.type() for storage in self.registry.externalStorages()],
                         ["SimpleCopy", "WebDAV"])

    def testStoreFetchFileLater(self):
        """
        Test file storing and fetching (Later mode)
        """

        f = self.getNewFile(b"New content")

        # store
        url = self.url + "/" + os.path.basename(f.name)
        storedContent = self.storage.store(f.name, url, self.auth_config.id())
        self.assertTrue(storedContent)
        self.assertEqual(storedContent.status(), Qgis.ContentStatus.NotStarted)

        spyErrorOccurred = QSignalSpy(storedContent.errorOccurred)
        spyProgressChanged = QSignalSpy(storedContent.progressChanged)

        loop = QEventLoop()
        storedContent.stored.connect(loop.quit)
        storedContent.errorOccurred.connect(loop.quit)
        QTimer.singleShot(1, lambda: storedContent.store())
        loop.exec()

        self.assertEqual(len(spyErrorOccurred), 0)
        self.assertEqual(storedContent.url(), url)
        self.assertFalse(storedContent.errorString())
        self.assertEqual(storedContent.status(), Qgis.ContentStatus.Finished)
        self.assertTrue(len(spyProgressChanged) > 0)
        self.assertEqual(spyProgressChanged[-1][0], 100)

        # fetch
        fetchedContent = self.storage.fetch(self.url + "/" + os.path.basename(f.name), self.auth_config.id())
        self.assertTrue(fetchedContent)
        self.assertEqual(fetchedContent.status(), Qgis.ContentStatus.NotStarted)

        spyErrorOccurred = QSignalSpy(fetchedContent.errorOccurred)

        loop = QEventLoop()
        fetchedContent.fetched.connect(loop.quit)
        fetchedContent.errorOccurred.connect(loop.quit)
        QTimer.singleShot(1, lambda: fetchedContent.fetch())
        loop.exec()

        self.assertEqual(len(spyErrorOccurred), 0)
        self.assertEqual(fetchedContent.status(), Qgis.ContentStatus.Finished)
        self.assertFalse(fetchedContent.errorString())
        self.assertTrue(fetchedContent.filePath())
        self.checkContent(fetchedContent.filePath(), b"New content")
        self.assertEqual(os.path.splitext(fetchedContent.filePath())[1], '.txt')

        # fetch again, should be cached
        fetchedContent = self.storage.fetch(self.url + "/" + os.path.basename(f.name), self.auth_config.id())
        self.assertTrue(fetchedContent)
        self.assertEqual(fetchedContent.status(), Qgis.ContentStatus.NotStarted)

        spyErrorOccurred = QSignalSpy(fetchedContent.errorOccurred)

        loop = QEventLoop()
        fetchedContent.fetched.connect(loop.quit)
        fetchedContent.errorOccurred.connect(loop.quit)
        QTimer.singleShot(1, lambda: fetchedContent.fetch())
        loop.exec()

        self.assertEqual(len(spyErrorOccurred), 0)
        self.assertEqual(fetchedContent.status(), Qgis.ContentStatus.Finished)
        self.assertTrue(not fetchedContent.errorString())
        self.assertTrue(fetchedContent.filePath())
        self.checkContent(fetchedContent.filePath(), b"New content")
        self.assertEqual(os.path.splitext(fetchedContent.filePath())[1], '.txt')

        # fetch bad url
        fetchedContent = self.storage.fetch(self.url + "/error", self.auth_config.id())
        self.assertTrue(fetchedContent)
        self.assertEqual(fetchedContent.status(), Qgis.ContentStatus.NotStarted)

        spyErrorOccurred = QSignalSpy(fetchedContent.errorOccurred)

        loop = QEventLoop()
        fetchedContent.fetched.connect(loop.quit)
        fetchedContent.errorOccurred.connect(loop.quit)
        QTimer.singleShot(1, lambda: fetchedContent.fetch())
        loop.exec()

        self.assertEqual(len(spyErrorOccurred), 1)
        self.assertEqual(fetchedContent.status(), Qgis.ContentStatus.Failed)
        self.assertTrue(fetchedContent.errorString())
        self.assertFalse(fetchedContent.filePath())

    def testStoreFetchFileImmediately(self):
        """
        Test file storing and fetching (Immediately mode)
        """

        f = self.getNewFile(b"New content")

        # store
        url = self.url + "/" + os.path.basename(f.name)
        storedContent = self.storage.store(f.name, url, self.auth_config.id(), Qgis.ActionStart.Immediate)
        self.assertTrue(storedContent)
        self.assertEqual(storedContent.status(), Qgis.ContentStatus.Running)

        spyErrorOccurred = QSignalSpy(storedContent.errorOccurred)
        spyProgressChanged = QSignalSpy(storedContent.progressChanged)

        loop = QEventLoop()
        storedContent.stored.connect(loop.quit)
        storedContent.errorOccurred.connect(loop.quit)
        loop.exec()

        self.assertEqual(len(spyErrorOccurred), 0)
        self.assertEqual(storedContent.url(), url)
        self.assertFalse(storedContent.errorString())
        self.assertEqual(storedContent.status(), Qgis.ContentStatus.Finished)
        self.assertTrue(len(spyProgressChanged) > 0)
        self.assertEqual(spyProgressChanged[-1][0], 100)

        # fetch
        fetchedContent = self.storage.fetch(self.url + "/" + os.path.basename(f.name), self.auth_config.id(), Qgis.ActionStart.Immediate)
        self.assertTrue(fetchedContent)

        # Some external storage (SimpleCopy) doesn't actually need to retrieve the resource
        self.assertTrue(fetchedContent.status() == Qgis.ContentStatus.Finished or
                        fetchedContent.status() == Qgis.ContentStatus.Running)

        if (fetchedContent.status() == Qgis.ContentStatus.Running):

            spyErrorOccurred = QSignalSpy(fetchedContent.errorOccurred)

            loop = QEventLoop()
            fetchedContent.fetched.connect(loop.quit)
            fetchedContent.errorOccurred.connect(loop.quit)
            loop.exec()

            self.assertEqual(len(spyErrorOccurred), 0)

        self.assertEqual(fetchedContent.status(), Qgis.ContentStatus.Finished)
        self.assertFalse(fetchedContent.errorString())
        self.assertTrue(fetchedContent.filePath())
        self.checkContent(fetchedContent.filePath(), b"New content")
        self.assertEqual(os.path.splitext(fetchedContent.filePath())[1], '.txt')

        # fetch again, should be cached
        fetchedContent = self.storage.fetch(self.url + "/" + os.path.basename(f.name), self.auth_config.id(), Qgis.ActionStart.Immediate)
        self.assertTrue(fetchedContent)
        self.assertEqual(fetchedContent.status(), Qgis.ContentStatus.Finished)

        self.assertTrue(not fetchedContent.errorString())
        self.assertTrue(fetchedContent.filePath())
        self.checkContent(fetchedContent.filePath(), b"New content")
        self.assertEqual(os.path.splitext(fetchedContent.filePath())[1], '.txt')

        # fetch bad url
        fetchedContent = self.storage.fetch(self.url + "/error", self.auth_config.id(), Qgis.ActionStart.Immediate)
        self.assertTrue(fetchedContent)

        # Some external storage (SimpleCopy) doesn't actually need to retrieve the resource
        self.assertTrue(fetchedContent.status() == Qgis.ContentStatus.Failed or
                        fetchedContent.status() == Qgis.ContentStatus.Running)

        if (fetchedContent.status() == Qgis.ContentStatus.Running):
            spyErrorOccurred = QSignalSpy(fetchedContent.errorOccurred)

            loop = QEventLoop()
            fetchedContent.errorOccurred.connect(loop.quit)
            fetchedContent.fetched.connect(loop.quit)
            loop.exec()

            self.assertEqual(len(spyErrorOccurred), 1)

        self.assertEqual(fetchedContent.status(), Qgis.ContentStatus.Failed)
        self.assertTrue(fetchedContent.errorString())
        self.assertFalse(fetchedContent.filePath())

    def testStoreBadUrl(self):
        """
        Test file storing with a bad url
        """
        f = self.getNewFile(b"New content")

        storedContent = self.storage.store(f.name, self.badUrl + os.path.basename(f.name), self.auth_config.id())
        self.assertTrue(storedContent)
        self.assertEqual(storedContent.status(), Qgis.ContentStatus.NotStarted)

        spyStored = QSignalSpy(storedContent.stored)
        spyCanceled = QSignalSpy(storedContent.canceled)

        loop = QEventLoop()
        storedContent.errorOccurred.connect(loop.quit)
        QTimer.singleShot(1, lambda: storedContent.store())
        loop.exec()

        self.assertEqual(storedContent.status(), Qgis.ContentStatus.Failed)
        self.assertTrue(storedContent.errorString())
        self.assertFalse(storedContent.url())

        QCoreApplication.processEvents()

        self.assertEqual(len(spyStored), 0)
        self.assertEqual(len(spyCanceled), 0)

    def testStoreMissingAuth(self):
        """
        Test file storing with missing authentication
        """

        f = self.getNewFile(b"New content")
        storedContent = self.storage.store(f.name, self.url + "/" + os.path.basename(f.name))
        self.assertTrue(storedContent)
        self.assertEqual(storedContent.status(), Qgis.ContentStatus.NotStarted)

        spyStored = QSignalSpy(storedContent.stored)
        spyCanceled = QSignalSpy(storedContent.canceled)

        loop = QEventLoop()
        storedContent.errorOccurred.connect(loop.quit)
        QTimer.singleShot(1, lambda: storedContent.store())
        loop.exec()

        self.assertEqual(storedContent.status(), Qgis.ContentStatus.Failed)
        self.assertTrue(storedContent.errorString())
        self.assertFalse(storedContent.url())

        QCoreApplication.processEvents()

        self.assertEqual(len(spyStored), 0)
        self.assertEqual(len(spyCanceled), 0)

    def testStoreWithoutFileName(self):
        """
        Test file storing and fetching
        """

        f = self.getNewFile(b"New content")

        # store
        storedContent = self.storage.store(f.name, self.url + "/", self.auth_config.id())
        self.assertTrue(storedContent)
        self.assertEqual(storedContent.status(), Qgis.ContentStatus.NotStarted)

        spyErrorOccurred = QSignalSpy(storedContent.errorOccurred)
        spyProgressChanged = QSignalSpy(storedContent.progressChanged)

        loop = QEventLoop()
        storedContent.stored.connect(loop.quit)
        storedContent.errorOccurred.connect(loop.quit)
        QTimer.singleShot(1, lambda: storedContent.store())
        loop.exec()

        self.assertEqual(len(spyErrorOccurred), 0)
        self.assertFalse(storedContent.errorString())
        self.assertEqual(storedContent.url(), self.url + "/" + os.path.basename(f.name))
        self.assertEqual(storedContent.status(), Qgis.ContentStatus.Finished)
        self.assertTrue(len(spyProgressChanged) > 0)
        self.assertEqual(spyProgressChanged[-1][0], 100)

        # fetch
        fetchedContent = self.storage.fetch(self.url + "/" + os.path.basename(f.name), self.auth_config.id())
        self.assertTrue(fetchedContent)
        self.assertEqual(fetchedContent.status(), Qgis.ContentStatus.NotStarted)

        spyErrorOccurred = QSignalSpy(fetchedContent.errorOccurred)

        loop = QEventLoop()
        fetchedContent.fetched.connect(loop.quit)
        fetchedContent.errorOccurred.connect(loop.quit)
        QTimer.singleShot(1, lambda: fetchedContent.fetch())
        loop.exec()

        self.assertEqual(len(spyErrorOccurred), 0)
        self.assertEqual(fetchedContent.status(), Qgis.ContentStatus.Finished)
        self.assertFalse(fetchedContent.errorString())
        self.assertTrue(fetchedContent.filePath())
        self.checkContent(fetchedContent.filePath(), b"New content")
        self.assertEqual(os.path.splitext(fetchedContent.filePath())[1], '.txt')
