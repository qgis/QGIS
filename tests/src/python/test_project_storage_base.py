# -*- coding: utf-8 -*-
"""QGIS Unit tests base for the database project storage.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
from builtins import next

__author__ = 'Julien Cabieces'
__date__ = '2022-04-19'
__copyright__ = 'Copyright 2022, The QGIS Project'

import qgis  # NOQA

import os
import time

from qgis.core import (
    QgsApplication,
    QgsDataSourceUri,
    QgsVectorLayer,
    QgsProject,
)
from PyQt5.QtCore import QDateTime, QUrl, QUrlQuery
from qgis.PyQt.QtSql import QSqlDatabase, QSqlQuery
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath


class TestPyQgsProjectStorageBase:

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        pass

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        pass

    def dropProjectsTable(self):
        """Drop existing project storage table"""
        pass

    def encode_uri(self, ds_uri, schema_name, project_name=None):
        return None

    def testSaveLoadProject(self):
        schema_uri = self.encode_uri(self.ds_uri, self.schema)
        project_uri = self.encode_uri(self.ds_uri, self.schema, 'abc')

        self.dropProjectsTable()  # make sure we have a clean start

        prj = QgsProject()
        uri = self.vl.source()

        vl1 = QgsVectorLayer(uri, 'test', self.provider)
        self.assertEqual(vl1.isValid(), True)
        prj.addMapLayer(vl1)

        prj_storage = QgsApplication.projectStorageRegistry().projectStorageFromType(self.project_storage_type)
        self.assertTrue(prj_storage)

        lst0 = prj_storage.listProjects(schema_uri)
        self.assertEqual(lst0, [])

        # try to save project in the database

        prj.setFileName(project_uri)
        res = prj.write()
        self.assertTrue(res)

        lst1 = prj_storage.listProjects(schema_uri)
        self.assertEqual(lst1, ["abc"])

        # now try to load the project back

        prj2 = QgsProject()
        prj2.setFileName(project_uri)
        res = prj2.read()
        self.assertTrue(res)

        self.assertEqual(len(prj2.mapLayers()), 1)

        self.assertEqual(prj2.baseName(), "abc")
        self.assertEqual(prj2.absoluteFilePath(), "")  # path not supported for project storages
        self.assertTrue(abs(prj2.lastModified().secsTo(QDateTime.currentDateTime())) < 10)
        lastModified = prj2.lastModified()

        # try to see project's metadata

        res, metadata = prj_storage.readProjectStorageMetadata(project_uri)
        self.assertTrue(res)
        self.assertEqual(metadata.name, "abc")
        time_project = metadata.lastModified
        time_now = QDateTime.currentDateTime()
        time_diff = time_now.secsTo(time_project)
        self.assertTrue(abs(time_diff) < 10)

        # try to update the project
        vl1.setName("testNew")
        prj.write()

        prj3 = QgsProject()
        prj3.setFileName(project_uri)
        res = prj3.read()
        self.assertTrue(res)

        prj4 = QgsProject()
        prj4.setFileName(project_uri)
        res = prj4.read()
        self.assertTrue(res)

        self.assertEqual(len(prj4.mapLayers()), 1)
        self.assertEqual(list(prj4.mapLayers().values())[0].name(), "testNew")

        self.assertEqual(prj4.baseName(), "abc")
        self.assertEqual(prj4.absoluteFilePath(), "")  # path not supported for project storages
        self.assertTrue(prj4.lastModified() > lastModified)

        # try to remove the project

        res = prj_storage.removeProject(project_uri)
        self.assertTrue(res)

        lst2 = prj_storage.listProjects(schema_uri)
        self.assertEqual(lst2, [])

        self.dropProjectsTable()  # make sure we have a clean finish... "leave no trace"
