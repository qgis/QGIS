"""
***************************************************************************
    ParametersTest
    ---------------------
    Date                 : August 2017
    Copyright            : (C) 2017 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Nyall Dawson"
__date__ = "August 2017"
__copyright__ = "(C) 2017, Nyall Dawson"

import os
import unittest

from qgis.analysis import QgsNativeAlgorithms
from qgis.core import (
    QgsApplication,
)
from qgis.testing import QgisTestCase, start_app

from processing.gui.algorithm_widget import AlgorithmWidget

start_app()
QgsApplication.processingRegistry().addProvider(QgsNativeAlgorithms())

testDataPath = os.path.join(os.path.dirname(__file__), "testdata")


class AlgorithmDialogTest(QgisTestCase):
    def testCreation(self):
        alg = QgsApplication.processingRegistry().createAlgorithmById(
            "native:centroids"
        )
        a = AlgorithmWidget(alg)
        self.assertEqual(a.mainWidget().algorithm(), alg)


if __name__ == "__main__":
    unittest.main()
