"""QGIS Unit tests for edit widgets.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Tobias Reber"
__date__ = "20/05/2015"
__copyright__ = "Copyright 2015, The QGIS Project"


from qgis.core import NULL, QgsFeature, QgsGeometry, QgsPointXY, QgsVectorLayer
from qgis.gui import QgsGui
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsRangeWidget(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        QgsGui.editorWidgetRegistry().initEditors()

    def setUp(self):
        """
        create a layer with one feature
        """
        self.layer = QgsVectorLayer(
            "Point?crs=EPSG:21781&field=fldtxt:string&field=fldint:integer",
            "addfeat",
            "memory",
        )
        pr = self.layer.dataProvider()  # NOQA
        f = QgsFeature()
        f.setAttributes(["Hello World", 123])
        f.setGeometry(QgsGeometry.fromPointXY(QgsPointXY(600000, 200000)))

    def __createRangeWidget(self, allownull=False):
        """
        create a range widget
        """
        reg = QgsGui.editorWidgetRegistry()
        configWdg = reg.createConfigWidget("Range", self.layer, 1, None)
        config = configWdg.config()
        config["Min"] = 0

        # if null shall be allowed
        if allownull:
            config["AllowNull"] = allownull

        rangewidget = reg.create("Range", self.layer, 1, config, None, None)
        return rangewidget

    def test_range_widget_numbers(self):
        """
        are the numbers being returned correctly
        """
        rangewidget = self.__createRangeWidget()

        rangewidget.setValues(1, [])
        assert rangewidget.value() == 1

        rangewidget.setValues(0, [])
        assert rangewidget.value() == 0

    def test_range_widget_no_null(self):
        """
        are None and NULL being returned as expected
        """
        rangewidget = self.__createRangeWidget()

        rangewidget.setValues(NULL, [])
        assert rangewidget.value() == 0

        rangewidget.setValues(None, [])
        assert rangewidget.value() == 0

    def test_range_widget_null_allowed(self):
        """
        are None and NULL being returned as expected
        """
        rangewidget = self.__createRangeWidget(True)

        rangewidget.setValues(NULL, [])
        self.assertEqual(rangewidget.value(), NULL)

        rangewidget.setValues(None, [])
        self.assertEqual(rangewidget.value(), NULL)

        rangewidget.setValues(0, [])
        self.assertEqual(rangewidget.value(), 0)


if __name__ == "__main__":
    unittest.main()
