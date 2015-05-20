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


    def setUp(self):
        """
        create a layer with one feature
        """
        QgsEditorWidgetRegistry.initEditors()
        self.layer = QgsVectorLayer("Point?crs=EPSG:21781&field=fldtxt:string&field=fldint:integer",
                               "addfeat", "memory")
        pr = self.layer.dataProvider()
        f = QgsFeature()
        f.setAttributes(["Hello World", 123])
        f.setGeometry(QgsGeometry.fromPoint(QgsPoint(600000,200000)))

        self.reg = QgsEditorWidgetRegistry.instance()
        self.configWdg = self.reg.createConfigWidget('Range', self.layer, 1, None)
        self.config = self.configWdg.config()
        self.rangewidget = self.reg.create('Range', self.layer, 1, self.config, None, None )
       

    def test_range_widget_numbers(self):
        """
        are the numbers being returned correctly
        """
        self.rangewidget.setValue(1)
        assert self.rangewidget.value() == 1

        self.rangewidget.setValue(0)
        assert self.rangewidget.value() == 0
    

    def test_range_widget_null(self):
        """
        Is None being returned as expected
        """
        self.rangewidget.setValue(NULL)
        assert self.rangewidget.value() == 0

        self.rangewidget.setValue(None)
        assert self.rangewidget.value() == 0

        # allow NULL
        self.config["AllowNull"] = True
        self.rangewidget = self.reg.create('Range', self.layer, 1, self.config, None, None )

        self.rangewidget.setValue(NULL)
        assert self.rangewidget.value() == NULL

        self.rangewidget.setValue(None)
        assert self.rangewidget.value() == NULL



if __name__ == '__main__':
    unittest.main()

