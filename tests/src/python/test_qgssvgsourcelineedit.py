# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsSvgSourceLineEdit

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '19/07/2018'
__copyright__ = 'Copyright 2018, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.gui import QgsSvgSourceLineEdit

from qgis.PyQt.QtTest import QSignalSpy
from qgis.testing import start_app, unittest

start_app()


class TestQgsSvgSourceLineEdit(unittest.TestCase):

    def testGettersSetters(self):
        """ test widget getters/setters """
        w = QgsSvgSourceLineEdit()
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


if __name__ == '__main__':
    unittest.main()
