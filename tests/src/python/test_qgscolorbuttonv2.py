# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsColorButtonV2.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '25/05/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.gui import QgsColorButtonV2
from qgis.testing import start_app, unittest
from qgis.PyQt.QtGui import QColor
start_app()


class TestQgsColorButtonV2(unittest.TestCase):

    def testClearingColors(self):
        """
        Test setting colors to transparent
        """

        # start with a valid color
        button = QgsColorButtonV2()
        button.setAllowAlpha(True)
        button.setColor(QColor(255, 100, 200, 255))
        self.assertEqual(button.color(), QColor(255, 100, 200, 255))

        # now set to no color
        button.setToNoColor()
        # ensure that only the alpha channel has changed - not the other color components
        self.assertEqual(button.color(), QColor(255, 100, 200, 0))


if __name__ == '__main__':
    unittest.main()
