"""
***************************************************************************
    Project Provider tests
    ---------------------
    Date                 : August 2026
    Copyright            : (C) 2026 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************8
"""

import unittest

from qgis.core import (
    QgsProcessingModelAlgorithm,
    QgsProcessingProjectModelProvider,
    QgsProject,
)
from qgis.PyQt.QtCore import QTemporaryFile
from qgis.testing import QgisTestCase, start_app

start_app()


class TestQgsProjectModelProvider(QgisTestCase):
    def testSaveRestoreFromProject(self):
        p = QgsProject()
        provider = QgsProcessingProjectModelProvider(p)

        # add some algorithms
        alg = QgsProcessingModelAlgorithm("test name", "test group")
        provider.addModel(alg)
        alg2 = QgsProcessingModelAlgorithm("test name2", "test group2")
        provider.addModel(alg2)
        self.assertEqual(len(provider.algorithms()), 2)

        tmp_file = QTemporaryFile()
        tmp_file.open()  # fileName is no available until open
        temp_path = tmp_file.fileName()
        tmp_file.close()

        self.assertTrue(p.write(temp_path))

        # restore project
        p2 = QgsProject()
        provider2 = QgsProcessingProjectModelProvider(p2)
        self.assertTrue(p2.read(temp_path))

        self.assertEqual(len(provider2.algorithms()), 2)
        self.assertEqual(provider2.algorithms()[0].name(), "test name")
        self.assertEqual(provider2.algorithms()[0].group(), "test group")
        self.assertEqual(provider2.algorithms()[1].name(), "test name2")
        self.assertEqual(provider2.algorithms()[1].group(), "test group2")

        # clear project should remove algorithms
        p2.clear()
        self.assertFalse(provider2.algorithms())

    def testDelete(self):
        """
        Test deleting a model from the project
        """
        p = QgsProject()
        provider = QgsProcessingProjectModelProvider(p)

        # add some models
        alg = QgsProcessingModelAlgorithm("test name", "test group")
        provider.addModel(alg)
        alg2 = QgsProcessingModelAlgorithm("test name2", "test group2")
        provider.addModel(alg2)
        self.assertEqual(len(provider.algorithms()), 2)

        # try to delete
        provider.removeModel(None)
        self.assertEqual(len(provider.algorithms()), 2)

        # not in provider!
        alg3 = QgsProcessingModelAlgorithm("test name3", "test group")
        provider.removeModel(alg3)
        self.assertEqual(len(provider.algorithms()), 2)

        # delete model actually in project
        provider.removeModel(alg)
        self.assertEqual(len(provider.algorithms()), 1)
        self.assertEqual(provider.algorithms()[0].name(), "test name2")

        # overwrite model
        alg2b = QgsProcessingModelAlgorithm("test name2", "test group2")
        alg2b.setHelpContent({"test": "test"})
        provider.addModel(alg2b)
        self.assertEqual(len(provider.algorithms()), 1)
        self.assertEqual(provider.algorithms()[0].helpContent(), {"test": "test"})

        provider.removeModel(alg2)
        self.assertEqual(len(provider.algorithms()), 0)


if __name__ == "__main__":
    unittest.main()
