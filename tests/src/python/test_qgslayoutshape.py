# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutItemShape.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2017 by Nyall Dawson'
__date__ = '23/10/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA

from qgis.testing import start_app, unittest
from qgis.core import QgsLayoutItemShape

from test_qgslayoutitem import LayoutItemTestCase

start_app()


class TestQgsLayoutShape(unittest.TestCase, LayoutItemTestCase):

    @classmethod
    def setUpClass(cls):
        cls.item_class = QgsLayoutItemShape


if __name__ == '__main__':
    unittest.main()
