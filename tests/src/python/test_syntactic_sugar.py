# -*- coding: utf-8 -*-
"""QGIS Unit tests for some syntactic sugar in python

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Matthias Kuhn'
__date__ = '12.8.2015'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.testing import unittest, start_app
from qgis.core import (edit,
                       QgsFeature,
                       QgsVectorLayer,
                       QgsEditError
                       )

start_app()


class TestSyntacticSugar(unittest.TestCase):

    def testEdit(self):
        """Test `with edit(layer):` code"""

        ml = QgsVectorLayer("Point?crs=epsg:4236&field=id:integer&field=value:double",
                            "test_data", "memory")
        # Data as list of x, y, id, value
        self.assertTrue(ml.isValid())
        fields = ml.fields()

        # Check insert
        with edit(ml):
            feat = QgsFeature(fields)
            feat['id'] = 1
            feat['value'] = 0.9
            self.assertTrue(ml.addFeature(feat))

        self.assertEqual(next(ml.dataProvider().getFeatures())['value'], 0.9)

        # Check update
        with edit(ml):
            f = next(ml.getFeatures())
            f['value'] = 9.9
            self.assertTrue(ml.updateFeature(f))

        self.assertEqual(next(ml.dataProvider().getFeatures())['value'], 9.9)

        # Check for rollBack after exceptions
        with self.assertRaises(NameError):
            with edit(ml):
                f = next(ml.getFeatures())
                f['value'] = 3.8
                crashycrash()  # NOQA

        self.assertEqual(next(ml.dataProvider().getFeatures())['value'], 9.9)
        self.assertEqual(next(ml.getFeatures())['value'], 9.9)

        # Check for `as`
        with edit(ml) as l:
            f = next(l.getFeatures())
            f['value'] = 10
            self.assertTrue(l.updateFeature(f))

        self.assertEqual(next(ml.dataProvider().getFeatures())['value'], 10)

        # Check that we get a QgsEditError exception when the commit fails
        with self.assertRaises(QgsEditError):
            with edit(ml) as l:
                l.rollBack()

if __name__ == "__main__":
    unittest.main()
