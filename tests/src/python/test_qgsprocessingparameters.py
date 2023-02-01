"""QGIS Unit tests for Processing algorithm runner(s).

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'David Marteau'
__date__ = '2020-09'
__copyright__ = 'Copyright 2020, The QGIS Project'

import re
from qgis.PyQt.QtCore import QCoreApplication
from qgis.testing import start_app, unittest
from qgis.core import QgsProcessingAlgRunnerTask

from processing.core.Processing import Processing
from processing.core.ProcessingConfig import ProcessingConfig
from qgis.testing import start_app, unittest
from qgis.core import (
    QgsApplication,
    QgsSettings,
    QgsProcessingContext,
    QgsProcessingAlgRunnerTask,
    QgsProcessingAlgorithm,
    QgsProject,
    QgsProcessingFeedback,
    QgsProcessingParameterGeometry,
    QgsWkbTypes,
)

start_app()


class TestQgsProcessingParameters(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(
            "QGIS_TestPyQgsProcessingParameters.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsProcessingParameters")
        QgsSettings().clear()
        Processing.initialize()
        cls.registry = QgsApplication.instance().processingRegistry()

    def test_qgsprocessinggometry(self):  # spellok
        """ Test QgsProcessingParameterGeometry initialization """
        geomtypes = [QgsWkbTypes.PointGeometry, QgsWkbTypes.PolygonGeometry]
        param = QgsProcessingParameterGeometry(name='test', geometryTypes=geomtypes)

        types = param.geometryTypes()

        self.assertEqual(param.geometryTypes(), geomtypes)


if __name__ == '__main__':
    unittest.main()
