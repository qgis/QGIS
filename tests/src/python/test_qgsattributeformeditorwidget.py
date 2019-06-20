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

import qgis  # NOQA

from qgis.gui import (QgsSearchWidgetWrapper,
                      QgsAttributeFormEditorWidget,
                      QgsDefaultSearchWidgetWrapper,
                      QgsAttributeForm,
                      QgsSearchWidgetToolButton,
                      QgsGui
                      )
from qgis.core import (QgsVectorLayer)
from qgis.PyQt.QtWidgets import QWidget, QDateTimeEdit
from qgis.PyQt.QtCore import QDateTime, QDate, QTime
from qgis.testing import start_app, unittest

start_app()
QgsGui.editorWidgetRegistry().initEditors()


class PyQgsAttributeFormEditorWidget(unittest.TestCase):

    def testCurrentFilterExpression(self):
        """ Test creating an expression using the widget"""

        layer = QgsVectorLayer("Point?field=fldint:integer", "test", "memory")
        parent = QWidget()
        w = QgsDefaultSearchWidgetWrapper(layer, 0, parent)
        setup = QgsGui.editorWidgetRegistry().findBest(layer, "fldint")
        wrapper = QgsGui.editorWidgetRegistry().create(layer, 0, None, parent)
        af = QgsAttributeFormEditorWidget(wrapper, setup.type(), None)
        af.setSearchWidgetWrapper(w)

        # test that filter combines both current value in search widget wrapper and flags from search tool button
        w.lineEdit().setText('5.5')
        sb = af.findChild(QWidget, "SearchWidgetToolButton")
        sb.setActiveFlags(QgsSearchWidgetWrapper.EqualTo)
        self.assertEqual(af.currentFilterExpression(), '"fldint"=5.5')
        sb.setActiveFlags(QgsSearchWidgetWrapper.NotEqualTo)
        self.assertEqual(af.currentFilterExpression(), '"fldint"<>5.5')

    def testSetActive(self):
        """ Test setting the search as active - should set active flags to match search widget wrapper's defaults """

        layer = QgsVectorLayer("Point?field=fldtext:string&field=fldint:integer", "test", "memory")
        parent = QWidget()
        w = QgsDefaultSearchWidgetWrapper(layer, 0, parent)
        setup = QgsGui.editorWidgetRegistry().findBest(layer, "fldint")
        wrapper = QgsGui.editorWidgetRegistry().create(layer, 0, None, parent)
        af = QgsAttributeFormEditorWidget(wrapper, setup.type(), None)
        af.setSearchWidgetWrapper(w)

        sb = af.findChild(QWidget, "SearchWidgetToolButton")
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
        wrapper = QgsGui.editorWidgetRegistry().create(layer, 0, None, form)
        af = QgsAttributeFormEditorWidget(wrapper, 'DateTime', None)
        af.createSearchWidgetWrappers()

        d1 = af.findChildren(QDateTimeEdit)[0]
        d2 = af.findChildren(QDateTimeEdit)[1]
        d1.setDateTime(QDateTime(QDate(2013, 5, 6), QTime()))
        d2.setDateTime(QDateTime(QDate(2013, 5, 16), QTime()))

        sb = af.findChild(QWidget, "SearchWidgetToolButton")
        sb.setActiveFlags(QgsSearchWidgetWrapper.Between)
        self.assertEqual(af.currentFilterExpression(), '"fldtext">=\'2013-05-06\' AND "fldtext"<=\'2013-05-16\'')
        sb.setActiveFlags(QgsSearchWidgetWrapper.IsNotBetween)
        self.assertEqual(af.currentFilterExpression(), '"fldtext"<\'2013-05-06\' OR "fldtext">\'2013-05-16\'')


if __name__ == '__main__':
    unittest.main()
