# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsSelectionContext.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import qgis  # NOQA

from qgis.core import (
    QgsSelectionContext,
)
from qgis.testing import unittest


class TestQgsSelectionContext(unittest.TestCase):

    def testBasic(self):
        context = QgsSelectionContext()
        context.setScale(1000)
        self.assertEqual(context.scale(), 1000)


if __name__ == '__main__':
    unittest.main()
