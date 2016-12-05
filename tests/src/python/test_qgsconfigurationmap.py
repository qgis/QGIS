# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsConfigurationMap.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Matthias Kuhn'
__date__ = '18/11/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis # switch sip api

from qgis.core import QgsConfigurationMap

from qgis.PyQt.QtXml import (
    QDomDocument,
    QDomElement
)

from qgis.testing import (
    start_app,
    unittest
)

start_app()


class TestQgsConfigurationMap(unittest.TestCase):

    def create_doc(self):
        doc = QDomDocument("properties")
        element = doc.createElement("test")
        doc.appendChild(element)
        return (doc, element)

    def test_integer(self):
        """
        Test that maps are correctly loaded and written
        """
        doc, element = self.create_doc()

        my_properties = {'a': 1, 'b': 2, 'c': 3, 'd': -1}
        prop = QgsConfigurationMap(my_properties)
        prop.toXml(element)

        self.assertEquals(prop.get(), my_properties)

        prop2 = QgsConfigurationMap()

        prop2.fromXml(element)
        self.assertEquals(my_properties, prop2.get())

    def test_string(self):
        """
        Test that maps are correctly loaded and written
        """
        doc, element = self.create_doc()

        my_properties = {'a': 'a', 'b': 'b', 'c': 'something_else', 'empty': ''}
        prop = QgsConfigurationMap(my_properties)
        prop.toXml(element)

        self.assertEquals(prop.get(), my_properties)

        prop2 = QgsConfigurationMap()

        prop2.fromXml(element)
        self.assertEquals(my_properties, prop2.get())

    def test_double(self):
        """
        Test that maps are correctly loaded and written
        """
        doc, element = self.create_doc()

        my_properties = {'a': 0.27, 'b': 1.0, 'c': 5}
        prop = QgsConfigurationMap(my_properties)
        prop.toXml(element)

        self.assertEquals(prop.get(), my_properties)

        prop2 = QgsConfigurationMap()

        prop2.fromXml(element)
        self.assertEquals(my_properties, prop2.get())

    def test_boolean(self):
        """
        Test that maps are correctly loaded and written
        """
        doc, element = self.create_doc()

        my_properties = {'a': True, 'b': False}
        prop = QgsConfigurationMap(my_properties)
        prop.toXml(element)

        print(doc.toString())

        self.assertEquals(prop.get(), my_properties)

        prop2 = QgsConfigurationMap()

        prop2.fromXml(element)
        self.assertEquals(my_properties, prop2.get())

    def test_complex(self):
        """
        Test that maps are correctly loaded and written
        """
        doc, element = self.create_doc()

        my_properties = {'boolean': True, 'integer': False, 'map': {'a': 1}}
        prop = QgsConfigurationMap(my_properties)
        prop.toXml(element)

        print(doc.toString())

        self.assertEquals(prop.get(), my_properties)

        prop2 = QgsConfigurationMap()

        prop2.fromXml(element)
        self.assertEquals(my_properties, prop2.get())


if __name__ == '__main__':
    unittest.main()
