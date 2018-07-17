# -*- coding: utf-8 -*-

"""
***************************************************************************
    Project Provider tests
    ---------------------
    Date                 : July 2018
    Copyright            : (C) 2018 by Nyall Dawson
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

__author__ = 'Nyall Dawson'
__date__ = 'July 2018'
__copyright__ = '(C) 2018, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.testing import start_app, unittest
from qgis.PyQt.QtCore import QTemporaryFile
from qgis.core import (QgsApplication,
                       QgsProcessingModelAlgorithm,
                       QgsProject)
from processing.modeler.ProjectProvider import ProjectProvider
from processing.modeler.ModelerDialog import ModelerDialog

start_app()


class ProjectProviderTest(unittest.TestCase):

    def testSaveRestoreFromProject(self):
        p = QgsProject()
        provider = ProjectProvider(p)

        # add some algorithms
        alg = QgsProcessingModelAlgorithm('test name', 'test group')
        provider.add_model(alg)
        alg2 = QgsProcessingModelAlgorithm('test name2', 'test group2')
        provider.add_model(alg2)
        self.assertEqual(len(provider.algorithms()), 2)

        tmp_file = QTemporaryFile()
        tmp_file.open()  # fileName is no available until open
        temp_path = tmp_file.fileName()
        tmp_file.close()

        self.assertTrue(p.write(temp_path))

        # restore project
        p2 = QgsProject()
        provider2 = ProjectProvider(p2)
        self.assertTrue(p2.read(temp_path))

        self.assertEqual(len(provider2.model_definitions), 2)
        self.assertEqual(len(provider2.algorithms()), 2)
        self.assertEqual(provider2.algorithms()[0].name(), 'test name')
        self.assertEqual(provider2.algorithms()[0].group(), 'test group')
        self.assertEqual(provider2.algorithms()[1].name(), 'test name2')
        self.assertEqual(provider2.algorithms()[1].group(), 'test group2')

        # clear project should remove algorithms
        p2.clear()
        self.assertFalse(provider2.algorithms())

    def testDelete(self):
        """
        Test deleting a model from the project
        """
        p = QgsProject()
        provider = ProjectProvider(p)

        # add some models
        alg = QgsProcessingModelAlgorithm('test name', 'test group')
        provider.add_model(alg)
        alg2 = QgsProcessingModelAlgorithm('test name2', 'test group2')
        provider.add_model(alg2)
        self.assertEqual(len(provider.algorithms()), 2)

        # try to delete
        provider.remove_model(None)
        self.assertEqual(len(provider.algorithms()), 2)

        # not in provider!
        alg3 = QgsProcessingModelAlgorithm('test name3', 'test group')
        provider.remove_model(alg3)
        self.assertEqual(len(provider.algorithms()), 2)

        # delete model actually in project
        provider.remove_model(alg)
        self.assertEqual(len(provider.algorithms()), 1)
        self.assertEqual(provider.algorithms()[0].name(), 'test name2')

        provider.remove_model(alg2)
        self.assertEqual(len(provider.algorithms()), 0)

    def testDialog(self):
        """
        Test saving model to project from dialog
        """
        p = QgsProject().instance()
        provider = ProjectProvider()
        QgsApplication.processingRegistry().addProvider(provider)

        # make an algorithm
        alg = QgsProcessingModelAlgorithm('test name', 'test group')

        dialog = ModelerDialog(alg)
        dialog.saveInProject()

        self.assertEqual(len(provider.model_definitions), 1)
        self.assertEqual(len(provider.algorithms()), 1)
        self.assertEqual(provider.algorithms()[0].name(), 'test name')
        self.assertEqual(provider.algorithms()[0].group(), 'test group')


if __name__ == '__main__':
    unittest.main()
