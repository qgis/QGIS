"""QGIS Unit tests for QgsLayoutFrame.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2017 by Nyall Dawson"
__date__ = "23/10/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

from qgis.core import QgsLayoutFrame, QgsLayoutItemHtml
import unittest
from qgis.testing import start_app, QgisTestCase

from test_qgslayoutitem import LayoutItemTestCase

start_app()


class TestQgsLayoutFrame(QgisTestCase, LayoutItemTestCase):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.mf = None

    @classmethod
    def createItem(cls, layout):
        cls.mf = QgsLayoutItemHtml(layout)
        return QgsLayoutFrame(layout, cls.mf)


if __name__ == "__main__":
    unittest.main()
