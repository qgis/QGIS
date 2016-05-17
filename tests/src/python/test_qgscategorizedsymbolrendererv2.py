# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsCategorizedSymbolRendererV2

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2/12/2015'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.testing import unittest, start_app
from qgis.core import (QgsCategorizedSymbolRendererV2,
                       QgsRendererCategoryV2,
                       QgsMarkerSymbolV2,
                       QgsField,
                       QgsFields
                       )
from qgis.PyQt.QtCore import QVariant

start_app()


def createMarkerSymbol():
    symbol = QgsMarkerSymbolV2.createSimple({
        "color": "100,150,50",
        "name": "square",
        "size": "3.0"
    })
    return symbol


class TestQgsCategorizedSymbolRendererV2(unittest.TestCase):

    def testFilter(self):
        """Test filter creation"""
        renderer = QgsCategorizedSymbolRendererV2()
        renderer.setClassAttribute('field')

        renderer.addCategory(QgsRendererCategoryV2('a', createMarkerSymbol(), 'a'))
        renderer.addCategory(QgsRendererCategoryV2('b', createMarkerSymbol(), 'b'))
        renderer.addCategory(QgsRendererCategoryV2('c', createMarkerSymbol(), 'c'))
        # add default category
        renderer.addCategory(QgsRendererCategoryV2('', createMarkerSymbol(), 'default'))

        fields = QgsFields()
        fields.append(QgsField('field', QVariant.String))
        fields.append(QgsField('num', QVariant.Double))

        self.assertEqual(renderer.filter(fields), '')
        # remove categories, leaving default
        assert renderer.updateCategoryRenderState(0, False)
        self.assertEqual(renderer.filter(fields), "(\"field\") NOT IN ('a') OR (\"field\") IS NULL")
        assert renderer.updateCategoryRenderState(1, False)
        self.assertEqual(renderer.filter(fields), "(\"field\") NOT IN ('a','b') OR (\"field\") IS NULL")
        assert renderer.updateCategoryRenderState(2, False)
        self.assertEqual(renderer.filter(fields), "(\"field\") NOT IN ('a','b','c') OR (\"field\") IS NULL")
        # remove default category
        assert renderer.updateCategoryRenderState(3, False)
        self.assertEqual(renderer.filter(fields), "FALSE")
        # add back other categories, leaving default disabled
        assert renderer.updateCategoryRenderState(0, True)
        self.assertEqual(renderer.filter(fields), "(\"field\") IN ('a')")
        assert renderer.updateCategoryRenderState(1, True)
        self.assertEqual(renderer.filter(fields), "(\"field\") IN ('a','b')")
        assert renderer.updateCategoryRenderState(2, True)
        self.assertEqual(renderer.filter(fields), "(\"field\") IN ('a','b','c')")

        renderer.deleteAllCategories()
        # just default category
        renderer.addCategory(QgsRendererCategoryV2('', createMarkerSymbol(), 'default'))
        self.assertEqual(renderer.filter(fields), '')
        assert renderer.updateCategoryRenderState(0, False)
        self.assertEqual(renderer.filter(fields), 'FALSE')

        renderer.deleteAllCategories()
        # no default category
        renderer.addCategory(QgsRendererCategoryV2('a', createMarkerSymbol(), 'a'))
        renderer.addCategory(QgsRendererCategoryV2('b', createMarkerSymbol(), 'b'))
        renderer.addCategory(QgsRendererCategoryV2('c', createMarkerSymbol(), 'c'))
        self.assertEqual(renderer.filter(fields), "(\"field\") IN ('a','b','c')")
        assert renderer.updateCategoryRenderState(0, False)
        self.assertEqual(renderer.filter(fields), "(\"field\") IN ('b','c')")
        assert renderer.updateCategoryRenderState(2, False)
        self.assertEqual(renderer.filter(fields), "(\"field\") IN ('b')")
        assert renderer.updateCategoryRenderState(1, False)
        self.assertEqual(renderer.filter(fields), "FALSE")

        renderer.deleteAllCategories()
        renderer.setClassAttribute('num')
        # numeric categories
        renderer.addCategory(QgsRendererCategoryV2(1, createMarkerSymbol(), 'a'))
        renderer.addCategory(QgsRendererCategoryV2(2, createMarkerSymbol(), 'b'))
        renderer.addCategory(QgsRendererCategoryV2(3, createMarkerSymbol(), 'c'))
        self.assertEqual(renderer.filter(fields), '(\"num\") IN (1,2,3)')
        assert renderer.updateCategoryRenderState(0, False)
        self.assertEqual(renderer.filter(fields), "(\"num\") IN (2,3)")
        assert renderer.updateCategoryRenderState(2, False)
        self.assertEqual(renderer.filter(fields), "(\"num\") IN (2)")
        assert renderer.updateCategoryRenderState(1, False)
        self.assertEqual(renderer.filter(fields), "FALSE")

    def testFilterExpression(self):
        """Test filter creation with expression"""
        renderer = QgsCategorizedSymbolRendererV2()
        renderer.setClassAttribute('field + field2')

        renderer.addCategory(QgsRendererCategoryV2('a', createMarkerSymbol(), 'a'))
        renderer.addCategory(QgsRendererCategoryV2('b', createMarkerSymbol(), 'b'))
        renderer.addCategory(QgsRendererCategoryV2('c', createMarkerSymbol(), 'c'))
        # add default category
        renderer.addCategory(QgsRendererCategoryV2('', createMarkerSymbol(), 'default'))

        fields = QgsFields()
        fields.append(QgsField('field', QVariant.String))

        self.assertEqual(renderer.filter(fields), '')
        # remove categories, leaving default
        assert renderer.updateCategoryRenderState(0, False)
        self.assertEqual(renderer.filter(fields), "(field + field2) NOT IN ('a') OR (field + field2) IS NULL")
        assert renderer.updateCategoryRenderState(1, False)
        self.assertEqual(renderer.filter(fields), "(field + field2) NOT IN ('a','b') OR (field + field2) IS NULL")
        assert renderer.updateCategoryRenderState(2, False)
        self.assertEqual(renderer.filter(fields), "(field + field2) NOT IN ('a','b','c') OR (field + field2) IS NULL")
        # remove default category
        assert renderer.updateCategoryRenderState(3, False)
        self.assertEqual(renderer.filter(fields), "FALSE")
        # add back other categories, leaving default disabled
        assert renderer.updateCategoryRenderState(0, True)
        self.assertEqual(renderer.filter(fields), "(field + field2) IN ('a')")
        assert renderer.updateCategoryRenderState(1, True)
        self.assertEqual(renderer.filter(fields), "(field + field2) IN ('a','b')")
        assert renderer.updateCategoryRenderState(2, True)
        self.assertEqual(renderer.filter(fields), "(field + field2) IN ('a','b','c')")

        renderer.deleteAllCategories()
        # just default category
        renderer.addCategory(QgsRendererCategoryV2('', createMarkerSymbol(), 'default'))
        self.assertEqual(renderer.filter(fields), '')
        assert renderer.updateCategoryRenderState(0, False)
        self.assertEqual(renderer.filter(fields), 'FALSE')

        renderer.deleteAllCategories()
        # no default category
        renderer.addCategory(QgsRendererCategoryV2('a', createMarkerSymbol(), 'a'))
        renderer.addCategory(QgsRendererCategoryV2('b', createMarkerSymbol(), 'b'))
        renderer.addCategory(QgsRendererCategoryV2('c', createMarkerSymbol(), 'c'))
        self.assertEqual(renderer.filter(fields), "(field + field2) IN ('a','b','c')")
        assert renderer.updateCategoryRenderState(0, False)
        self.assertEqual(renderer.filter(fields), "(field + field2) IN ('b','c')")
        assert renderer.updateCategoryRenderState(2, False)
        self.assertEqual(renderer.filter(fields), "(field + field2) IN ('b')")
        assert renderer.updateCategoryRenderState(1, False)
        self.assertEqual(renderer.filter(fields), "FALSE")

        renderer.deleteAllCategories()
        # numeric categories
        renderer.addCategory(QgsRendererCategoryV2(1, createMarkerSymbol(), 'a'))
        renderer.addCategory(QgsRendererCategoryV2(2, createMarkerSymbol(), 'b'))
        renderer.addCategory(QgsRendererCategoryV2(3, createMarkerSymbol(), 'c'))
        self.assertEqual(renderer.filter(fields), '(field + field2) IN (1,2,3)')
        assert renderer.updateCategoryRenderState(0, False)
        self.assertEqual(renderer.filter(fields), "(field + field2) IN (2,3)")
        assert renderer.updateCategoryRenderState(2, False)
        self.assertEqual(renderer.filter(fields), "(field + field2) IN (2)")
        assert renderer.updateCategoryRenderState(1, False)
        self.assertEqual(renderer.filter(fields), "FALSE")

if __name__ == "__main__":
    unittest.main()
