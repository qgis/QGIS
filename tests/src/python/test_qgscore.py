# -*- coding: utf-8 -*-
"""QGIS Unit tests for core functions

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Loïc Bartoletti'
__date__ = '28.6.2019'
__copyright__ = 'Copyright 2019, The QGIS Project'

import qgis  # NOQA
import os

from qgis.testing import unittest, start_app
from qgis.core import qgsRound, qgsDoubleNear
from qgis.PyQt import sip

start_app()


class TestCoreAdditions(unittest.TestCase):

    def testQgsRound(self):
        qgsDoubleNear(qgsRound(1234.567, 2), 1234.57, 0.01)
        qgsDoubleNear(qgsRound(-1234.567, 2), -1234.57, 0.01)
        qgsDoubleNear(qgsRound(98765432198, 8), 98765432198, 1.0)
        qgsDoubleNear(qgsRound(98765432198, 9), 98765432198, 1.0)
        qgsDoubleNear(qgsRound(98765432198, 10), 98765432198, 1.0)
        qgsDoubleNear(qgsRound(98765432198, 11), 98765432198, 1.0)
        qgsDoubleNear(qgsRound(98765432198, 12), 98765432198, 1.0)
        qgsDoubleNear(qgsRound(98765432198, 13), 98765432198, 1.0)
        qgsDoubleNear(qgsRound(98765432198, 14), 98765432198, 1.0)
        qgsDoubleNear(qgsRound(98765432198765, 14), 98765432198765, 1.0)
        qgsDoubleNear(qgsRound(98765432198765432, 20), 98765432198765432, 1.0)
        qgsDoubleNear(qgsRound(9.8765432198765, 2), 9.88, 0.001)
        qgsDoubleNear(qgsRound(9.8765432198765, 3), 9.877, 0.0001)
        qgsDoubleNear(qgsRound(9.8765432198765, 4), 9.8765, 0.00001)
        qgsDoubleNear(qgsRound(9.8765432198765, 5), 9.87654, 0.000001)
        qgsDoubleNear(qgsRound(9.8765432198765, 6), 9.876543, 0.0000001)
        qgsDoubleNear(qgsRound(9.8765432198765, 7), 9.8765432, 0.00000001)
        qgsDoubleNear(qgsRound(-9.8765432198765, 7), -9.8765432, 0.0000001)
        qgsDoubleNear(qgsRound(9876543.2198765, 5), 9876543.219880, 0.000001)
        qgsDoubleNear(qgsRound(-9876543.2198765, 5), -9876543.219880, 0.000001)
        qgsDoubleNear(qgsRound(9.87654321987654321, 13), 9.87654321987654, 0.0000000000001)
        qgsDoubleNear(qgsRound(9.87654321987654321, 14), 9.876543219876543, 0.00000000000001)
        qgsDoubleNear(qgsRound(9998.87654321987654321, 14), 9998.876543219876543, 0.00000000000001)
        qgsDoubleNear(qgsRound(9999999.87654321987654321, 14),
                      9999999.876543219876543, 0.00000000000001)


if __name__ == "__main__":
    unittest.main()
