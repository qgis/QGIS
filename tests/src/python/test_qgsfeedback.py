"""QGIS Unit tests for QgsFeedback.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "12/02/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import QgsFeedback
from qgis.testing import unittest


class TestQgsFeedback(unittest.TestCase):

    def testCancel(self):
        f = QgsFeedback()
        self.assertFalse(f.isCanceled())

        cancel_spy = QSignalSpy(f.canceled)

        f.cancel()
        self.assertTrue(f.isCanceled())
        self.assertEqual(len(cancel_spy), 1)

    def testProgress(self):
        f = QgsFeedback()
        self.assertEqual(f.progress(), 0.0)

        progress_spy = QSignalSpy(f.progressChanged)

        f.setProgress(25)
        self.assertEqual(f.progress(), 25.0)
        self.assertEqual(len(progress_spy), 1)
        self.assertEqual(progress_spy[0][0], 25.0)

    def testProcessedCount(self):
        f = QgsFeedback()
        self.assertEqual(f.processedCount(), 0)

        processed_spy = QSignalSpy(f.processedCountChanged)

        f.setProcessedCount(25)
        self.assertEqual(f.processedCount(), 25)
        self.assertEqual(len(processed_spy), 1)
        self.assertEqual(processed_spy[0][0], 25)

    def testScaledFeedback(self):
        f = QgsFeedback()
        minScaled = 10
        maxScaled = 90
        scaledFeedback = QgsFeedback.createScaledFeedback(f, minScaled, maxScaled)

        scaledFeedback.setProgress(-1)

        scaledFeedback.setProgress(0)
        self.assertEqual(f.progress(), minScaled)

        scaledFeedback.setProgress(50)
        self.assertEqual(f.progress(), minScaled + (maxScaled - minScaled) * 50.0 / 100)

        scaledFeedback.setProgress(100)
        self.assertEqual(f.progress(), maxScaled)

        scaledFeedback.cancel()
        self.assertTrue(f.isCanceled())


if __name__ == "__main__":
    unittest.main()
