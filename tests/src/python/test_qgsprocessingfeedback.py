"""
***************************************************************************
    test_qgsprocessingfeedback.py
    ---------------------
    Date                 : January 2026
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

import os
import unittest
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import QgsProcessingFeedback, QgsProcessingMultiStepFeedback
from qgis.testing import unittest


class TestQgsProcessingFeedback(unittest.TestCase):

    def test_multi_step_feedback_default(self):
        """
        Test basic equal-weight step progression
        """
        f = QgsProcessingFeedback()
        multi_feedback = QgsProcessingMultiStepFeedback(2, f)

        progress_spy = QSignalSpy(f.progressChanged)

        # first step: 0% to 50%
        multi_feedback.setCurrentStep(0)
        self.assertEqual(f.progress(), 0.0)

        multi_feedback.setProgress(50)
        # halfway through first step of 2 => 25%
        self.assertEqual(f.progress(), 25.0)
        self.assertEqual(progress_spy[-1][0], 25.0)

        multi_feedback.setProgress(100)
        self.assertEqual(f.progress(), 50.0)

        # second step: 50% to 100%
        multi_feedback.setCurrentStep(1)
        self.assertEqual(f.progress(), 50.0)

        multi_feedback.setProgress(50)
        # halfway through second step of two => 75%
        self.assertEqual(f.progress(), 75.0)

        multi_feedback.setProgress(100)
        self.assertEqual(f.progress(), 100.0)

    def test_multi_step_feedback_with_weights(self):
        """
        Test custom weights for steps
        """
        f = QgsProcessingFeedback()
        multi_feedback = QgsProcessingMultiStepFeedback(2, f)

        # relative weights: step 1 = 20%, step 2 = 80%
        # check that weights are normalized internally
        multi_feedback.setStepWeights([1, 4])

        progress_spy = QSignalSpy(f.progressChanged)

        # step 1: 0% to 20%
        multi_feedback.setCurrentStep(0)
        self.assertEqual(f.progress(), 0.0)

        multi_feedback.setProgress(50)
        # 50% of 20% => 10% overall
        self.assertEqual(f.progress(), 10.0)

        multi_feedback.setProgress(100)
        self.assertEqual(f.progress(), 20.0)

        # step 2: 20% to 100%
        multi_feedback.setCurrentStep(1)
        self.assertEqual(f.progress(), 20.0)

        multi_feedback.setProgress(50)
        # first step = 20% + (50% of 80% for second step) = 20 + 40 = 60%
        self.assertEqual(f.progress(), 60.0)

        multi_feedback.setProgress(100)
        self.assertEqual(f.progress(), 100.0)

    def test_multistep_invalid_weights(self):
        """
        Test robustness against invalid weight lists
        """
        f = QgsProcessingFeedback()
        multi_feedback = QgsProcessingMultiStepFeedback(2, f)

        # incorrect number of weights should be ignored, revert to equal weights
        multi_feedback.setStepWeights([0.5, 0.2, 0.3])

        multi_feedback.setCurrentStep(0)
        multi_feedback.setProgress(100)
        self.assertEqual(f.progress(), 50.0)

        # zero total weight (should revert to equal weights)
        f = QgsProcessingFeedback()
        multi_feedback = QgsProcessingMultiStepFeedback(2, f)
        multi_feedback.setStepWeights([0.0, 0.0])

        multi_feedback.setCurrentStep(0)
        multi_feedback.setProgress(100)
        self.assertEqual(f.progress(), 50.0)

    def test_multistep_method_proxying(self):
        """
        Test that feedback methods are correctly proxied from multistep feedback
        """
        f = QgsProcessingFeedback(True)
        multi_feedback = QgsProcessingMultiStepFeedback(1, f)

        multi_feedback.pushInfo("Test Info")
        self.assertIn("Test Info", f.textLog())

        multi_feedback.pushWarning("Test Warning")
        self.assertIn("Test Warning", f.textLog())

        multi_feedback.pushDebugInfo("Test Debug")
        self.assertIn("Test Debug", f.textLog())

        multi_feedback.pushCommandInfo("Test Command")
        self.assertIn("Test Command", f.textLog())

        multi_feedback.pushConsoleInfo("Test Console")
        self.assertIn("Test Console", f.textLog())

    def test_multi_step_cancel_propagation(self):
        """
        Test that cancellation propagates
        """
        f = QgsProcessingFeedback()
        multi_feedback = QgsProcessingMultiStepFeedback(2, f)

        spy = QSignalSpy(multi_feedback.canceled)
        f.cancel()
        self.assertTrue(multi_feedback.isCanceled())
        self.assertEqual(len(spy), 1)


if __name__ == "__main__":
    unittest.main()
