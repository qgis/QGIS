"""
***************************************************************************
    test_qgsrasterpipe.py
    ---------------------
    Date                 : June 2021
    Copyright            : (C) 2021 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************

From build dir, run: ctest -R PyQgsRasterPipe -V

"""

__author__ = "Nyall Dawson"
__date__ = "June 2021"
__copyright__ = "(C) 2021, Nyall Dawson"

from qgis.core import (
    QgsExpressionContext,
    QgsProperty,
    QgsRasterPipe,
    QgsSingleBandPseudoColorRenderer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsRasterPipe(QgisTestCase):

    def test_data_defined_properties(self):
        pipe = QgsRasterPipe()

        pipe.dataDefinedProperties().setProperty(
            QgsRasterPipe.Property.RendererOpacity, QgsProperty.fromExpression("100/2")
        )
        self.assertEqual(
            pipe.dataDefinedProperties().property(
                QgsRasterPipe.Property.RendererOpacity
            ),
            QgsProperty.fromExpression("100/2"),
        )

        pipe.set(QgsSingleBandPseudoColorRenderer(None))
        self.assertTrue(pipe.renderer())
        self.assertEqual(pipe.renderer().opacity(), 1.0)

        # apply properties to pipe
        pipe.evaluateDataDefinedProperties(QgsExpressionContext())
        self.assertEqual(pipe.renderer().opacity(), 0.5)


if __name__ == "__main__":
    unittest.main()
