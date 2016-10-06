# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMultiEditToolButton.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '16/03/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis # switch sip api

from qgis.gui import QgsMultiEditToolButton

from qgis.testing import (start_app,
                          unittest
                          )

start_app()


class TestQgsMultiEditToolButton(unittest.TestCase):

    def test_state_logic(self):
        """
        Test that the logic involving button states is correct
        """
        w = QgsMultiEditToolButton()
        self.assertEqual(w.state(), QgsMultiEditToolButton.Default)

        # set is changed should update state to changed
        w.setIsChanged(True)
        self.assertEqual(w.state(), QgsMultiEditToolButton.Changed)
        w.setIsChanged(False)
        self.assertEqual(w.state(), QgsMultiEditToolButton.Default)
        #resetting changes should fall back to default state
        w.setIsChanged(True)
        w.resetChanges()
        self.assertEqual(w.state(), QgsMultiEditToolButton.Default)
        #setting changes committed should result in default state
        w.setIsChanged(True)
        w.changesCommitted()
        self.assertEqual(w.state(), QgsMultiEditToolButton.Default)

        #Test with mixed values
        w.setIsMixed(True)
        self.assertEqual(w.state(), QgsMultiEditToolButton.MixedValues)
        #changed state takes priority over mixed state
        w.setIsChanged(True)
        self.assertEqual(w.state(), QgsMultiEditToolButton.Changed)
        w.setIsChanged(False)
        #should reset to mixed state
        self.assertEqual(w.state(), QgsMultiEditToolButton.MixedValues)
        #resetting changes should fall back to mixed state
        w.setIsChanged(True)
        w.resetChanges()
        self.assertEqual(w.state(), QgsMultiEditToolButton.MixedValues)
        #setting changes committed should result in default state
        w.setIsChanged(True)
        w.changesCommitted()
        self.assertEqual(w.state(), QgsMultiEditToolButton.Default)

if __name__ == '__main__':
    unittest.main()
