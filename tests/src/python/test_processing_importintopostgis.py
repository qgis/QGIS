# -*- coding: utf-8 -*-
"""QGIS Unit tests for Processing Export to Postgis algorithm.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Alessandro Pasotti'
__date__ = '2018-09'
__copyright__ = 'Copyright 2019, The QGIS Project'

import re

from processing.core.Processing import Processing
from processing.gui.AlgorithmExecutor import execute
from qgis.analysis import QgsNativeAlgorithms
from qgis.core import (QgsApplication, QgsVectorLayer,
                       QgsGeometry, QgsProcessingContext,
                       QgsProcessingFeedback, QgsSettings,
                       )
from qgis.PyQt.QtCore import QCoreApplication
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

start_app()


class ConsoleFeedBack(QgsProcessingFeedback):

    _errors = []

    def reportError(self, error, fatalError=False):
        print(error)
        self._errors.append(error)


class TestExportToPostGis(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(
            "QGIS_TestPyQgsExportToPostgis.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsExportToPostgis")
        QgsSettings().clear()
        Processing.initialize()
        QgsApplication.processingRegistry().addProvider(QgsNativeAlgorithms())
        cls.registry = QgsApplication.instance().processingRegistry()

        # Create DB connection in the settings
        settings = QgsSettings()
        settings.beginGroup('/PostgreSQL/connections/qgis_test')
        settings.setValue('service', 'qgis_test')
        settings.setValue('database', 'qgis_test')

    def test_import(self):
        """Test algorithm with CamelCaseSchema"""

        alg = self.registry.createAlgorithmById("qgis:importintopostgis")
        self.assertIsNotNone(alg)

        table_name = 'out_TestPyQgsExportToPostgis'

        parameters = {
            'CREATEINDEX': True,
            'DATABASE': 'qgis_test',
            'DROP_STRING_LENGTH': False,
            'ENCODING': 'UTF-8',
            'FORCE_SINGLEPART': False,
            'GEOMETRY_COLUMN': 'geom',
            'INPUT': unitTestDataPath() + '/points.shp',
            'LOWERCASE_NAMES': True,
            'OVERWRITE': True,
            'PRIMARY_KEY': None,
            'SCHEMA': 'CamelCaseSchema',
            'TABLENAME': table_name
        }

        feedback = ConsoleFeedBack()
        context = QgsProcessingContext()
        # Note: the following returns true also in case of errors ...
        self.assertTrue(execute(alg, parameters, context, feedback))
        # ... so we check the log
        self.assertEqual(feedback._errors, [])

        # Check that data have been imported correctly
        exported = QgsVectorLayer(unitTestDataPath() + '/points.shp', 'exported')
        self.assertTrue(exported.isValid())
        imported = QgsVectorLayer("service='qgis_test' table=\"CamelCaseSchema\".\"%s\" (geom)" % table_name, 'imported', 'postgres')
        self.assertTrue(imported.isValid())
        imported_fields = [f.name() for f in imported.fields()]
        for f in exported.fields():
            self.assertTrue(f.name().lower() in imported_fields)

        # Check data
        imported_f = next(imported.getFeatures("class = 'Jet' AND heading = 85"))
        self.assertTrue(imported_f.isValid())
        exported_f = next(exported.getFeatures("class = 'Jet' AND heading = 85"))
        self.assertTrue(exported_f.isValid())
        self.assertEqual(exported_f.geometry().asWkt(), imported_f.geometry().asWkt())


if __name__ == '__main__':
    unittest.main()
