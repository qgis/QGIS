# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsImageSourceLineEdit

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '5/12/2018'
__copyright__ = 'Copyright 2018, The QGIS Project'

import qgis  # NOQA
import os
from qgis.gui import QgsImageSourceLineEdit

from qgis.PyQt.QtTest import QSignalSpy
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath


start_app()


class TestQgsImageSourceLineEdit(unittest.TestCase):

    def testGettersSetters(self):
        """ test widget getters/setters """
        w = QgsImageSourceLineEdit()
        spy = QSignalSpy(w.sourceChanged)

        w.setSource('source')
        self.assertEqual(w.source(), 'source')
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[0][0], 'source')

        # no signal for same value
        w.setSource('source')
        self.assertEqual(w.source(), 'source')
        self.assertEqual(len(spy), 1)

        w.setSource('another')
        self.assertEqual(w.source(), 'another')
        self.assertEqual(len(spy), 2)
        self.assertEqual(spy[1][0], 'another')

    def testEmbedding(self):
        """Test embedding large SVGs """
        w = QgsImageSourceLineEdit()
        spy = QSignalSpy(w.sourceChanged)

        w.setSource('source')
        self.assertEqual(w.source(), 'source')
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[0][0], 'source')

        b64 = 'base64:' + ''.join(['x'] * 1000000)
        w.setSource(b64)
        self.assertEqual(w.source(), b64)
        self.assertEqual(len(spy), 2)
        self.assertEqual(spy[1][0], b64)

        w.setSource(os.path.join(unitTestDataPath(), 'landsat.tif'))
        self.assertEqual(w.source(), os.path.join(unitTestDataPath(), 'landsat.tif'))
        self.assertEqual(len(spy), 3)
        self.assertEqual(spy[2][0], os.path.join(unitTestDataPath(), 'landsat.tif'))

        w.setSource(b64)
        self.assertEqual(w.source(), b64)
        self.assertEqual(len(spy), 4)
        self.assertEqual(spy[3][0], b64)

        w.setSource('')
        self.assertEqual(w.source(), '')
        self.assertEqual(len(spy), 5)
        self.assertEqual(spy[4][0], '')


if __name__ == '__main__':
    unittest.main()
