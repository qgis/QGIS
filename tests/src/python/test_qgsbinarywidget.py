# -*- coding: utf-8 -*-
"""QGIS Unit tests for binary editor widgets.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '11/11/2018'
__copyright__ = 'Copyright 2018, The QGIS Project'

import qgis  # NOQA

from qgis.PyQt.QtCore import QByteArray
from qgis.core import QgsFeature, QgsGeometry, QgsPointXY, QgsVectorLayer, NULL
from qgis.gui import QgsGui
from qgis.testing import start_app, unittest

start_app()


class TestQgsBinaryWidget(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        QgsGui.editorWidgetRegistry().initEditors()

    def setUp(self):
        """
        create a layer with one feature
        """
        self.layer = QgsVectorLayer("Point?crs=EPSG:21781&field=fldint:integer&field=fldbin:binary",
                                    "addfeat", "memory")
        self.assertTrue(self.layer.isValid())
        f = QgsFeature()
        bin_1 = b'xxx'
        bin_val1 = QByteArray(bin_1)
        f.setAttributes([123, bin_val1])
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(600000, 200000)))

    def __createBinaryWidget(self):
        """
        create a binary widget
        """
        reg = QgsGui.editorWidgetRegistry()
        configWdg = reg.createConfigWidget('Binary', self.layer, 1, None)
        config = configWdg.config()
        binary_widget = reg.create('Binary', self.layer, 1, config, None, None)
        return binary_widget

    def testValue(self):
        widget = self.__createBinaryWidget()
        self.assertTrue(widget.widget())

        self.assertFalse(widget.value())

        bin_2 = b'yyy'
        bin_val2 = QByteArray(bin_2)

        widget.setValues(bin_val2, [])
        self.assertEqual(widget.value(), bin_val2)

        widget.setValues(NULL, [])
        self.assertEqual(widget.value(), QByteArray())


if __name__ == '__main__':
    unittest.main()
