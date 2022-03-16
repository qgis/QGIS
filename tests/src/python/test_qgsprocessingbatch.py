# -*- coding: utf-8 -*-
"""QGIS Unit tests for batch Processing

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '09/11/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.core import (
    QgsProcessingBatchFeedback,
    QgsProcessingFeedback
)

from qgis.PyQt.QtXml import QDomDocument

from qgis.testing import start_app, unittest

start_app()


class TestQgsProcessingBatch(unittest.TestCase):

    def testFeedback(self):
        """
        Test QgsProcessingBatchFeedback
        """
        parent_feedback = QgsProcessingFeedback()
        feedback = QgsProcessingBatchFeedback(5, parent_feedback)

        # test error collection
        self.assertFalse(feedback.popErrors())
        feedback.reportError('error 1')
        feedback.reportError('error 2')
        self.assertEqual(feedback.popErrors(), ['error 1', 'error 2'])
        self.assertFalse(feedback.popErrors())


if __name__ == '__main__':
    unittest.main()
