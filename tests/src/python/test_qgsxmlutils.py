# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsXmlUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Matthias Kuhn'
__date__ = '18/11/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'

import qgis  # NOQA switch sip api
from qgis.core import (QgsXmlUtils,
                       QgsProperty,
                       QgsGeometry,
                       QgsCoordinateReferenceSystem)

from qgis.PyQt.QtXml import QDomDocument
from qgis.PyQt.QtGui import QColor

from qgis.testing import start_app, unittest

start_app()


class TestQgsXmlUtils(unittest.TestCase):

    def test_invalid(self):
        """
        Test that invalid attributes are correctly loaded and written
        """
        doc = QDomDocument("properties")

        elem = QgsXmlUtils.writeVariant(None, doc)

        prop2 = QgsXmlUtils.readVariant(elem)
        self.assertIsNone(prop2)

    def test_integer(self):
        """
        Test that maps are correctly loaded and written
        """
        doc = QDomDocument("properties")

        my_properties = {'a': 1, 'b': 2, 'c': 3, 'd': -1}
        elem = QgsXmlUtils.writeVariant(my_properties, doc)

        prop2 = QgsXmlUtils.readVariant(elem)
        self.assertEqual(my_properties, prop2)

    def test_long(self):
        """
        Test that maps are correctly loaded and written
        """
        doc = QDomDocument("properties")

        # not sure if this actually does map to a long?
        my_properties = {'a': 9223372036854775808}
        elem = QgsXmlUtils.writeVariant(my_properties, doc)

        prop2 = QgsXmlUtils.readVariant(elem)
        self.assertEqual(my_properties, prop2)

    def test_string(self):
        """
        Test that maps are correctly loaded and written
        """
        doc = QDomDocument("properties")

        my_properties = {'a': 'a', 'b': 'b', 'c': 'something_else', 'empty': ''}
        elem = QgsXmlUtils.writeVariant(my_properties, doc)

        prop2 = QgsXmlUtils.readVariant(elem)

        self.assertEqual(my_properties, prop2)

    def test_double(self):
        """
        Test that maps are correctly loaded and written
        """
        doc = QDomDocument("properties")

        my_properties = {'a': 0.27, 'b': 1.0, 'c': 5}
        elem = QgsXmlUtils.writeVariant(my_properties, doc)

        prop2 = QgsXmlUtils.readVariant(elem)

        self.assertEqual(my_properties, prop2)

    def test_boolean(self):
        """
        Test that maps are correctly loaded and written
        """
        doc = QDomDocument("properties")

        my_properties = {'a': True, 'b': False}
        elem = QgsXmlUtils.writeVariant(my_properties, doc)

        prop2 = QgsXmlUtils.readVariant(elem)

        self.assertEqual(my_properties, prop2)

    def test_list(self):
        """
        Test that lists are correctly loaded and written
        """
        doc = QDomDocument("properties")
        my_properties = [1, 4, 'a', 'test', 7.9]
        elem = QgsXmlUtils.writeVariant(my_properties, doc)

        prop2 = QgsXmlUtils.readVariant(elem)

        self.assertEqual(my_properties, prop2)

    def test_complex(self):
        """
        Test that maps are correctly loaded and written
        """
        doc = QDomDocument("properties")

        my_properties = {'boolean': True, 'integer': False, 'map': {'a': 1}}
        elem = QgsXmlUtils.writeVariant(my_properties, doc)

        prop2 = QgsXmlUtils.readVariant(elem)

        self.assertEqual(my_properties, prop2)

    def test_property(self):
        """
        Test that QgsProperty values are correctly loaded and written
        """
        doc = QDomDocument("properties")

        prop = QgsProperty.fromValue(1001)
        elem = QgsXmlUtils.writeVariant(prop, doc)

        prop2 = QgsXmlUtils.readVariant(elem)

        self.assertEqual(prop, prop2)

        prop = QgsProperty.fromExpression('1+2=5')
        elem = QgsXmlUtils.writeVariant(prop, doc)

        prop2 = QgsXmlUtils.readVariant(elem)

        self.assertEqual(prop, prop2)

        prop = QgsProperty.fromField('oid')
        elem = QgsXmlUtils.writeVariant(prop, doc)

        prop2 = QgsXmlUtils.readVariant(elem)

        self.assertEqual(prop, prop2)

    def test_crs(self):
        """
        Test that QgsCoordinateReferenceSystem values are correctly loaded and written
        """
        doc = QDomDocument("properties")

        crs = QgsCoordinateReferenceSystem('epsg:3111')
        elem = QgsXmlUtils.writeVariant(crs, doc)

        crs2 = QgsXmlUtils.readVariant(elem)
        self.assertTrue(crs2.isValid())
        self.assertEqual(crs2.authid(), 'EPSG:3111')

        crs = QgsCoordinateReferenceSystem()
        elem = QgsXmlUtils.writeVariant(crs, doc)

        crs2 = QgsXmlUtils.readVariant(elem)
        self.assertFalse(crs2.isValid())

    def test_geom(self):
        """
        Test that QgsGeometry values are correctly loaded and written
        """
        doc = QDomDocument("properties")

        g = QgsGeometry.fromWkt('Point(3 4)')
        elem = QgsXmlUtils.writeVariant(g, doc)

        g2 = QgsXmlUtils.readVariant(elem)
        self.assertEqual(g2.asWkt(), 'Point (3 4)')

    def test_color(self):
        """
        Test that QColor values are correctly loaded and written
        """
        doc = QDomDocument("properties")

        elem = QgsXmlUtils.writeVariant(QColor(100, 200, 210), doc)
        c = QgsXmlUtils.readVariant(elem)
        self.assertEqual(c, QColor(100, 200, 210))
        elem = QgsXmlUtils.writeVariant(QColor(100, 200, 210, 50), doc)
        c = QgsXmlUtils.readVariant(elem)
        self.assertEqual(c, QColor(100, 200, 210, 50))
        elem = QgsXmlUtils.writeVariant(QColor(), doc)
        c = QgsXmlUtils.readVariant(elem)
        self.assertFalse(c.isValid())


if __name__ == '__main__':
    unittest.main()
