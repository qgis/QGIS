# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsEncodingSelectionDialog

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '21/11/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA

from qgis.gui import QgsEncodingSelectionDialog

from qgis.testing import start_app, unittest

start_app()


class TestQgsEncodingSelectionDialog(unittest.TestCase):

    def testGettersSetters(self):
        """ test dialog getters/setters """
        dlg = qgis.gui.QgsEncodingSelectionDialog(encoding='UTF-16')
        self.assertEqual(dlg.encoding(), 'UTF-16')
        dlg.setEncoding('UTF-8')
        self.assertEqual(dlg.encoding(), 'UTF-8')
        # custom encoding option
        dlg.setEncoding('trisolarian')
        self.assertEqual(dlg.encoding(), 'trisolarian')


if __name__ == '__main__':
    unittest.main()
