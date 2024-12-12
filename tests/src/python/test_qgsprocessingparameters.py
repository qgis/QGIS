"""QGIS Unit tests for Processing algorithm runner(s).

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "David Marteau"
__date__ = "2020-09"
__copyright__ = "Copyright 2020, The QGIS Project"

from processing.core.Processing import Processing
from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (
    QgsApplication,
    QgsProcessingParameterGeometry,
    QgsSettings,
    QgsWkbTypes,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsProcessingParameters(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestPyQgsProcessingParameters.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsProcessingParameters")
        QgsSettings().clear()
        Processing.initialize()
        cls.registry = QgsApplication.instance().processingRegistry()

    def test_qgsprocessinggometry(self):  # spellok
        """Test QgsProcessingParameterGeometry initialization"""
        geomtypes = [
            QgsWkbTypes.GeometryType.PointGeometry,
            QgsWkbTypes.GeometryType.PolygonGeometry,
        ]
        param = QgsProcessingParameterGeometry(name="test", geometryTypes=geomtypes)

        types = param.geometryTypes()

        self.assertEqual(param.geometryTypes(), geomtypes)


if __name__ == "__main__":
    unittest.main()
