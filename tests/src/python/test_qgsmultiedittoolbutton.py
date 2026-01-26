"""QGIS Unit tests for QgsMultiEditToolButton.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "16/03/2016"
__copyright__ = "Copyright 2016, The QGIS Project"


from qgis.gui import QgsMultiEditToolButton
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsMultiEditToolButton(QgisTestCase):

    def test_state_logic(self):
        """
        Test that the logic involving button states is correct
        """
        w = QgsMultiEditToolButton()
        self.assertEqual(w.state(), QgsMultiEditToolButton.State.Default)

        # set is changed should update state to changed
        w.setIsChanged(True)
        self.assertEqual(w.state(), QgsMultiEditToolButton.State.Changed)
        w.setIsChanged(False)
        self.assertEqual(w.state(), QgsMultiEditToolButton.State.Default)
        # resetting changes should fall back to default state
        w.setIsChanged(True)
        w.resetChanges()
        self.assertEqual(w.state(), QgsMultiEditToolButton.State.Default)
        # setting changes committed should result in default state
        w.setIsChanged(True)
        w.changesCommitted()
        self.assertEqual(w.state(), QgsMultiEditToolButton.State.Default)

        # Test with mixed values
        w.setIsMixed(True)
        self.assertEqual(w.state(), QgsMultiEditToolButton.State.MixedValues)
        # changed state takes priority over mixed state
        w.setIsChanged(True)
        self.assertEqual(w.state(), QgsMultiEditToolButton.State.Changed)
        w.setIsChanged(False)
        # should reset to mixed state
        self.assertEqual(w.state(), QgsMultiEditToolButton.State.MixedValues)
        # resetting changes should fall back to mixed state
        w.setIsChanged(True)
        w.resetChanges()
        self.assertEqual(w.state(), QgsMultiEditToolButton.State.MixedValues)
        # setting changes committed should result in default state
        w.setIsChanged(True)
        w.changesCommitted()
        self.assertEqual(w.state(), QgsMultiEditToolButton.State.Default)


if __name__ == "__main__":
    unittest.main()
