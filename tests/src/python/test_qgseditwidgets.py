# -*- coding: utf-8 -*-
"""QGIS Unit tests for edit widgets.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Matthias Kuhn'
__date__ = '20/05/2015'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis
import os

from qgis.core import QgsFeature, QgsGeometry, QgsPoint, QgsVectorLayer, NULL

from qgis.gui import QgsEditorWidgetRegistry

from PyQt4 import QtCore

from utilities import (unitTestDataPath,
                       getQgisTestApp,
                       TestCase,
                       unittest
                       )
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()


class TestQgsTextEditWidget(TestCase):

    @classmethod
    def setUpClass(cls):
        QgsEditorWidgetRegistry.initEditors()

    def createLayerWithOnePoint(self):
        self.layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                                    "addfeat", "memory")
        pr = self.layer.dataProvider()
        f = QgsFeature()
        f.setAttributes(["test", 123])
        f.setGeometry(QgsGeometry.fromPoint(QgsPoint(100, 200)))
        assert pr.addFeatures([f])
        assert self.layer.pendingFeatureCount() == 1
        return self.layer

    def doAttributeTest(self, idx, expected):
        reg = QgsEditorWidgetRegistry.instance()
        configWdg = reg.createConfigWidget('TextEdit', self.layer, idx, None)
        config = configWdg.config()
        editwidget = reg.create('TextEdit', self.layer, idx, config, None, None)

        editwidget.setValue('value')
        assert editwidget.value() == expected[0]

        editwidget.setValue(123)
        assert editwidget.value() == expected[1]

        editwidget.setValue(None)
        assert editwidget.value() == expected[2]

        editwidget.setValue(NULL)
        assert editwidget.value() == expected[3]

    def test_SetValue(self):
        self.createLayerWithOnePoint()

        self.doAttributeTest(0, ['value', '123', NULL, NULL])
        self.doAttributeTest(1, [NULL, 123, NULL, NULL])


if __name__ == '__main__':
    unittest.main()
