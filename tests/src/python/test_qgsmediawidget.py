"""QGIS Unit tests for QgsMediaWidget.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Mathieu Pellerin'
__date__ = '25/01/2023'
__copyright__ = 'Copyright 2023, The QGIS Project'

import qgis  # NOQA

from qgis.gui import QgsMediaWidget
from qgis.testing import start_app, unittest

start_app()


class TestQgsMediaWidget(unittest.TestCase):

    def testMediaPath(self):
        """
        Test media path
        """

        mw = QgsMediaWidget()
        self.assertEqual(mw.mediaPath(), '')
        mw.setMediaPath('/home/my.mp3')
        self.assertEqual(mw.mediaPath(), '/home/my.mp3')

    def testMode(self):
        """
        Test media widget's mode
        """

        mw = QgsMediaWidget()
        self.assertEqual(mw.mode(), QgsMediaWidget.Audio)
        mw.setMode(QgsMediaWidget.Video)
        self.assertEqual(mw.mode(), QgsMediaWidget.Video)

    def testLinkProjectColor(self):
        """
        Test video frame's minimum height
        """

        mw = QgsMediaWidget()
        mw.setVideoHeight(222)
        self.assertEqual(mw.videoHeight(), 222)


if __name__ == '__main__':
    unittest.main()
