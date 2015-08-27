# -*- coding: utf-8 -*-
"""QGIS Unit tests for edit widgets.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Tobias Reber'
__date__ = '20/05/2015'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis
import os

from qgis.core import QgsFeature, QgsGeometry, QgsPoint, QgsVectorLayer, NULL

from qgis.gui import QgsEditorWidgetRegistry

from PyQt4 import QtCore, QtGui

from utilities import (unitTestDataPath,
                       getQgisTestApp,
                       TestCase,
                       unittest
                       )
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()


class TestQgsRangeWidget(TestCase):

    @classmethod
    def setUpClass(cls):
        QgsEditorWidgetRegistry.initEditors()

    def setUp(self):
        """
        create a layer with one feature
        """
        self.layer = QgsVectorLayer("Point?crs=EPSG:21781&field=fldtxt:string&field=fldint:integer",
                                    "addfeat", "memory")
        pr = self.layer.dataProvider()
        f = QgsFeature()
        f.setAttributes(["Hello World", 123])
        f.setGeometry(QgsGeometry.fromPoint(QgsPoint(600000, 200000)))

    def __createRangeWidget(self, allownull=False):
        """
        create a range widget
        """
        reg = QgsEditorWidgetRegistry.instance()
        configWdg = reg.createConfigWidget('Range', self.layer, 1, None)
        config = configWdg.config()

        # if null shall be allowed
        if allownull == True:
            config["AllowNull"] = allownull

        rangewidget = reg.create('Range', self.layer, 1, config, None, None)
        return rangewidget

    def test_range_widget_numbers(self):
        """
        are the numbers being returned correctly
        """
        rangewidget = self.__createRangeWidget()

        rangewidget.setValue(1)
        assert rangewidget.value() == 1

        rangewidget.setValue(0)
        assert rangewidget.value() == 0

    def test_range_widget_no_null(self):
        """
        are None and NULL being returned as expected
        """
        rangewidget = self.__createRangeWidget()

        rangewidget.setValue(NULL)
        assert rangewidget.value() == 0

        rangewidget.setValue(None)
        assert rangewidget.value() == 0

    def test_range_widget_null_allowed(self):
        """
        are None and NULL being returned as expected
        """
        rangewidget = self.__createRangeWidget(True)

        rangewidget.setValue(NULL)
        assert rangewidget.value() == NULL

        rangewidget.setValue(None)
        assert rangewidget.value() == NULL


if __name__ == '__main__':
    unittest.main()
