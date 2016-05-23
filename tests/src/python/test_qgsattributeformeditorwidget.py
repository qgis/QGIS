# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsAttributeFormEditorWidget.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2016-05'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.gui import (QgsSearchWidgetWrapper,
                      QgsAttributeFormEditorWidget,
                      QgsSearchWidgetToolButton,
                      QgsDefaultSearchWidgetWrapper,
                      QgsAttributeForm,
                      QgsEditorWidgetRegistry
                      )
from qgis.core import (QgsVectorLayer)
from qgis.PyQt.QtWidgets import QWidget, QDateTimeEdit
from qgis.PyQt.QtCore import QDateTime, QDate, QTime
from qgis.testing import start_app, unittest

start_app()
QgsEditorWidgetRegistry.instance().initEditors()


class PyQgsAttributeFormEditorWidget(unittest.TestCase):

    def testCurrentFilterExpression(self):
        """ Test creating an expression using the widget"""

        layer = QgsVectorLayer("Point?field=fldint:integer", "test", "memory")
        parent = QWidget()
        w = QgsDefaultSearchWidgetWrapper(layer, 0, parent)
        af = QgsAttributeFormEditorWidget(None, None)
        af.setSearchWidgetWrapper(w)

        # test that filter combines both current value in search widget wrapper and flags from search tool button
        w.lineEdit().setText('5.5')
        af.searchWidgetToolButton().setActiveFlags(QgsSearchWidgetWrapper.EqualTo)
        self.assertEquals(af.currentFilterExpression(), '"fldint"=5.5')
        af.searchWidgetToolButton().setActiveFlags(QgsSearchWidgetWrapper.NotEqualTo)
        self.assertEquals(af.currentFilterExpression(), '"fldint"<>5.5')

    def testSetActive(self):
        """ Test setting the search as active - should set active flags to match search widget wrapper's defaults """

        layer = QgsVectorLayer("Point?field=fldtext:string&field=fldint:integer", "test", "memory")
        parent = QWidget()
        w = QgsDefaultSearchWidgetWrapper(layer, 0, parent)
        af = QgsAttributeFormEditorWidget(None, None)
        af.setSearchWidgetWrapper(w)

        sb = af.searchWidgetToolButton()
        # start with inactive
        sb.setActiveFlags(QgsSearchWidgetWrapper.FilterFlags())
        # set to inactive
        sb.setActive()
        # check that correct default flag was taken from search widget wrapper
        self.assertTrue(sb.activeFlags() & QgsSearchWidgetWrapper.Contains)

        # try again with numeric field - default should be "EqualTo"
        w = QgsDefaultSearchWidgetWrapper(layer, 1, parent)
        af.setSearchWidgetWrapper(w)
        # start with inactive
        sb.setActiveFlags(QgsSearchWidgetWrapper.FilterFlags())
        # set to inactive
        sb.setActive()
        # check that correct default flag was taken from search widget wrapper
        self.assertTrue(sb.activeFlags() & QgsSearchWidgetWrapper.EqualTo)

    def testBetweenFilter(self):
        """ Test creating a between type filter """
        layer = QgsVectorLayer("Point?field=fldtext:string&field=fldint:integer", "test", "memory")
        form = QgsAttributeForm(layer)
        af = QgsAttributeFormEditorWidget(None, form)
        af.createSearchWidgetWrappers("DateTime", 0, {})

        d1 = af.findChildren(QDateTimeEdit)[0]
        d2 = af.findChildren(QDateTimeEdit)[1]
        d1.setDateTime(QDateTime(QDate(2013, 5, 6), QTime()))
        d2.setDateTime(QDateTime(QDate(2013, 5, 16), QTime()))

        af.searchWidgetToolButton().setActiveFlags(QgsSearchWidgetWrapper.Between)
        self.assertEquals(af.currentFilterExpression(), '"fldtext">=\'2013-05-06\' AND "fldtext"<=\'2013-05-16\'')
        af.searchWidgetToolButton().setActiveFlags(QgsSearchWidgetWrapper.IsNotBetween)
        self.assertEquals(af.currentFilterExpression(), '"fldtext"<\'2013-05-06\' OR "fldtext">\'2013-05-16\'')


if __name__ == '__main__':
    unittest.main()
