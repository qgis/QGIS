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

__author__ = "Nyall Dawson"
__date__ = "July 2018"
__copyright__ = "(C) 2018, Nyall Dawson"

import unittest

from qgis.core import (
    QgsApplication,
    QgsProcessingModelAlgorithm,
    QgsProcessingProjectModelProvider,
    QgsProject,
)
from qgis.testing import QgisTestCase, start_app

from processing.modeler.ModelerDialog import ModelerDialog

start_app()


class ProjectProviderGuiTest(QgisTestCase):
    def testDialog(self):
        """
        Test saving model to project from dialog
        """
        p = QgsProject().instance()
        provider = QgsProcessingProjectModelProvider(p)
        QgsApplication.processingRegistry().addProvider(provider)

        # make an algorithm
        alg = QgsProcessingModelAlgorithm("test name", "test group")

        dialog = ModelerDialog(alg)
        dialog.saveInProject()

        self.assertEqual(len(provider.algorithms()), 1)
        self.assertEqual(provider.algorithms()[0].name(), "test name")
        self.assertEqual(provider.algorithms()[0].group(), "test group")


if __name__ == "__main__":
    unittest.main()
