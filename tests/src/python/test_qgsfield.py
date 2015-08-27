# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsField.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '16/08/2015'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis
import os

from qgis.core import QgsField, QgsVectorLayer, NULL
from utilities import (unitTestDataPath,
                       getQgisTestApp,
                       TestCase,
                       unittest
                       )
from unittest import expectedFailure

QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()


class TestQgsFields(TestCase):

    def test_expections(self):
        ml = QgsVectorLayer("Point?crs=epsg:4236&field=id:integer&field=value:double",
                            "test_data", "memory")
        assert ml.isValid()
        fields = ml.fields()

        #check no error
        fields.remove(1)
        #check exceptions raised
        with self.assertRaises(KeyError):
            fields.remove(-1)
        with self.assertRaises(KeyError):
            fields.remove(111)

        fields = ml.fields()
        #check no error
        fields.at(1)
        #check exceptions raised
        with self.assertRaises(KeyError):
            fields.at(-1)
        with self.assertRaises(KeyError):
            fields.at(111)

        #check no error
        fields.field(1)
        #check exceptions raised
        with self.assertRaises(KeyError):
            fields.field(-1)
        with self.assertRaises(KeyError):
            fields.field(111)

        #check no error
        fields.field('value')
        #check exceptions raised
        with self.assertRaises(KeyError):
            fields.field('bad')

        #check no error
        fields.fieldOrigin(1)
        #check exceptions raised
        with self.assertRaises(KeyError):
            fields.fieldOrigin(-1)
        with self.assertRaises(KeyError):
            fields.fieldOrigin(111)

        #check no error
        fields.fieldOriginIndex(1)
        #check exceptions raised
        with self.assertRaises(KeyError):
            fields.fieldOriginIndex(-1)
        with self.assertRaises(KeyError):
            fields.fieldOriginIndex(111)

if __name__ == '__main__':
    unittest.main()
