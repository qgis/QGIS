# -*- coding: utf-8 -*-

"""
***************************************************************************
    test_qgsrulebasedrenderer.py
    ---------------------
    Date                 : September 2015
    Copyright            : (C) 2015 by Matthias Kuhn
    Email                : matthias at opengis dot ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Matthias Kuhn'
__date__ = 'September 2015'
__copyright__ = '(C) 2015, Matthiasd Kuhn'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

import os

from qgis.PyQt.QtCore import QSize

from qgis.core import (QgsVectorLayer,
                       QgsMapLayerRegistry,
                       QgsRectangle,
                       QgsMultiRenderChecker,
                       QgsRuleBasedRenderer,
                       QgsFillSymbol,
                       QgsMarkerSymbol,
                       QgsRendererCategory,
                       QgsCategorizedSymbolRenderer,
                       QgsGraduatedSymbolRenderer,
                       QgsRendererRange
                       )
from qgis.testing import start_app, unittest
from qgis.testing.mocked import get_iface
from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsRulebasedRenderer(unittest.TestCase):

    def setUp(self):
        self.iface = get_iface()
        myShpFile = os.path.join(TEST_DATA_DIR, 'rectangles.shp')
        layer = QgsVectorLayer(myShpFile, 'Points', 'ogr')
        QgsMapLayerRegistry.instance().addMapLayer(layer)

        # Create rulebased style
        sym1 = QgsFillSymbol.createSimple({'color': '#fdbf6f'})
        sym2 = QgsFillSymbol.createSimple({'color': '#71bd6c'})
        sym3 = QgsFillSymbol.createSimple({'color': '#1f78b4'})

        self.r1 = QgsRuleBasedRenderer.Rule(sym1, 0, 0, '"id" = 1')
        self.r2 = QgsRuleBasedRenderer.Rule(sym2, 0, 0, '"id" = 2')
        self.r3 = QgsRuleBasedRenderer.Rule(sym3, 0, 0, 'ELSE')

        self.rootrule = QgsRuleBasedRenderer.Rule(None)
        self.rootrule.appendChild(self.r1)
        self.rootrule.appendChild(self.r2)
        self.rootrule.appendChild(self.r3)

        self.renderer = QgsRuleBasedRenderer(self.rootrule)
        layer.setRenderer(self.renderer)
        self.mapsettings = self.iface.mapCanvas().mapSettings()
        self.mapsettings.setOutputSize(QSize(400, 400))
        self.mapsettings.setOutputDpi(96)
        self.mapsettings.setExtent(QgsRectangle(-163, 22, -70, 52))

        rendered_layers = [layer.id()]
        self.mapsettings.setLayers(rendered_layers)

    def tearDown(self):
        QgsMapLayerRegistry.instance().removeAllMapLayers()

    def testElse(self):
        # Setup rendering check
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlName('expected_rulebased_else')
        result = renderchecker.runTest('rulebased_else')

        assert result

    def testDisabledElse(self):
        # Disable a rule and assert that it's hidden not rendered with else
        self.r2.setActive(False)

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlName('expected_rulebased_disabled_else')
        result = renderchecker.runTest('rulebased_disabled_else')

        assert result

    def testRefineWithCategories(self):
        # Test refining rule with categories (refs #10815)

        # First, try with a field based category (id)
        cats = []
        cats.append(QgsRendererCategory(1, QgsMarkerSymbol(), "id 1"))
        cats.append(QgsRendererCategory(2, QgsMarkerSymbol(), "id 2"))
        c = QgsCategorizedSymbolRenderer("id", cats)

        QgsRuleBasedRenderer.refineRuleCategories(self.r2, c)
        assert self.r2.children()[0].filterExpression() == '"id" = 1'
        assert self.r2.children()[1].filterExpression() == '"id" = 2'

        # Next try with an expression based category
        cats = []
        cats.append(QgsRendererCategory(1, QgsMarkerSymbol(), "result 1"))
        cats.append(QgsRendererCategory(2, QgsMarkerSymbol(), "result 2"))
        c = QgsCategorizedSymbolRenderer("id + 1", cats)

        QgsRuleBasedRenderer.refineRuleCategories(self.r1, c)
        assert self.r1.children()[0].filterExpression() == 'id + 1 = 1'
        assert self.r1.children()[1].filterExpression() == 'id + 1 = 2'

        # Last try with an expression which is just a quoted field name
        cats = []
        cats.append(QgsRendererCategory(1, QgsMarkerSymbol(), "result 1"))
        cats.append(QgsRendererCategory(2, QgsMarkerSymbol(), "result 2"))
        c = QgsCategorizedSymbolRenderer('"id"', cats)

        QgsRuleBasedRenderer.refineRuleCategories(self.r3, c)
        assert self.r3.children()[0].filterExpression() == '"id" = 1'
        assert self.r3.children()[1].filterExpression() == '"id" = 2'

    def testRefineWithRanges(self):
        # Test refining rule with ranges (refs #10815)

        # First, try with a field based category (id)
        ranges = []
        ranges.append(QgsRendererRange(0, 1, QgsMarkerSymbol(), "0-1"))
        ranges.append(QgsRendererRange(1, 2, QgsMarkerSymbol(), "1-2"))
        g = QgsGraduatedSymbolRenderer("id", ranges)

        QgsRuleBasedRenderer.refineRuleRanges(self.r2, g)
        assert self.r2.children()[0].filterExpression() == '"id" >= 0.0000 AND "id" <= 1.0000'
        assert self.r2.children()[1].filterExpression() == '"id" > 1.0000 AND "id" <= 2.0000'

        # Next try with an expression based range
        ranges = []
        ranges.append(QgsRendererRange(0, 1, QgsMarkerSymbol(), "0-1"))
        ranges.append(QgsRendererRange(1, 2, QgsMarkerSymbol(), "1-2"))
        g = QgsGraduatedSymbolRenderer("id / 2", ranges)

        QgsRuleBasedRenderer.refineRuleRanges(self.r1, g)
        assert self.r1.children()[0].filterExpression() == '(id / 2) >= 0.0000 AND (id / 2) <= 1.0000'
        assert self.r1.children()[1].filterExpression() == '(id / 2) > 1.0000 AND (id / 2) <= 2.0000'

        # Last try with an expression which is just a quoted field name
        ranges = []
        ranges.append(QgsRendererRange(0, 1, QgsMarkerSymbol(), "0-1"))
        ranges.append(QgsRendererRange(1, 2, QgsMarkerSymbol(), "1-2"))
        g = QgsGraduatedSymbolRenderer('"id"', ranges)

        QgsRuleBasedRenderer.refineRuleRanges(self.r3, g)
        assert self.r3.children()[0].filterExpression() == '"id" >= 0.0000 AND "id" <= 1.0000'
        assert self.r3.children()[1].filterExpression() == '"id" > 1.0000 AND "id" <= 2.0000'

    def testConvertFromCategorisedRenderer(self):
        # Test converting categorised renderer to rule based

        # First, try with a field based category (id)
        cats = []
        cats.append(QgsRendererCategory(1, QgsMarkerSymbol(), "id 1"))
        cats.append(QgsRendererCategory(2, QgsMarkerSymbol(), "id 2"))
        cats.append(QgsRendererCategory('a\'b', QgsMarkerSymbol(), "id a'b"))
        cats.append(QgsRendererCategory('a\nb', QgsMarkerSymbol(), "id a\\nb"))
        cats.append(QgsRendererCategory('a\\b', QgsMarkerSymbol(), "id a\\\\b"))
        cats.append(QgsRendererCategory('a\tb', QgsMarkerSymbol(), "id a\\tb"))
        c = QgsCategorizedSymbolRenderer("id", cats)

        r = QgsRuleBasedRenderer.convertFromRenderer(c)
        self.assertEqual(r.rootRule().children()[0].filterExpression(), '"id" = 1')
        self.assertEqual(r.rootRule().children()[1].filterExpression(), '"id" = 2')
        self.assertEqual(r.rootRule().children()[2].filterExpression(), '"id" = \'a\'\'b\'')
        self.assertEqual(r.rootRule().children()[3].filterExpression(), '"id" = \'a\\nb\'')
        self.assertEqual(r.rootRule().children()[4].filterExpression(), '"id" = \'a\\\\b\'')
        self.assertEqual(r.rootRule().children()[5].filterExpression(), '"id" = \'a\\tb\'')

        # Next try with an expression based category
        cats = []
        cats.append(QgsRendererCategory(1, QgsMarkerSymbol(), "result 1"))
        cats.append(QgsRendererCategory(2, QgsMarkerSymbol(), "result 2"))
        c = QgsCategorizedSymbolRenderer("id + 1", cats)

        r = QgsRuleBasedRenderer.convertFromRenderer(c)
        self.assertEqual(r.rootRule().children()[0].filterExpression(), 'id + 1 = 1')
        self.assertEqual(r.rootRule().children()[1].filterExpression(), 'id + 1 = 2')

        # Last try with an expression which is just a quoted field name
        cats = []
        cats.append(QgsRendererCategory(1, QgsMarkerSymbol(), "result 1"))
        cats.append(QgsRendererCategory(2, QgsMarkerSymbol(), "result 2"))
        c = QgsCategorizedSymbolRenderer('"id"', cats)

        r = QgsRuleBasedRenderer.convertFromRenderer(c)
        self.assertEqual(r.rootRule().children()[0].filterExpression(), '"id" = 1')
        self.assertEqual(r.rootRule().children()[1].filterExpression(), '"id" = 2')

    def testConvertFromGraduatedRenderer(self):
        # Test converting graduated renderer to rule based

        # First, try with a field based category (id)
        ranges = []
        ranges.append(QgsRendererRange(0, 1, QgsMarkerSymbol(), "0-1"))
        ranges.append(QgsRendererRange(1, 2, QgsMarkerSymbol(), "1-2"))
        g = QgsGraduatedSymbolRenderer("id", ranges)

        r = QgsRuleBasedRenderer.convertFromRenderer(g)
        self.assertEqual(r.rootRule().children()[0].filterExpression(), '"id" >= 0.000000 AND "id" <= 1.000000')
        self.assertEqual(r.rootRule().children()[1].filterExpression(), '"id" > 1.000000 AND "id" <= 2.000000')

        # Next try with an expression based range
        ranges = []
        ranges.append(QgsRendererRange(0, 1, QgsMarkerSymbol(), "0-1"))
        ranges.append(QgsRendererRange(1, 2, QgsMarkerSymbol(), "1-2"))
        g = QgsGraduatedSymbolRenderer("id / 2", ranges)

        r = QgsRuleBasedRenderer.convertFromRenderer(g)
        self.assertEqual(r.rootRule().children()[0].filterExpression(), '(id / 2) >= 0.000000 AND (id / 2) <= 1.000000')
        self.assertEqual(r.rootRule().children()[1].filterExpression(), '(id / 2) > 1.000000 AND (id / 2) <= 2.000000')

        # Last try with an expression which is just a quoted field name
        ranges = []
        ranges.append(QgsRendererRange(0, 1, QgsMarkerSymbol(), "0-1"))
        ranges.append(QgsRendererRange(1, 2, QgsMarkerSymbol(), "1-2"))
        g = QgsGraduatedSymbolRenderer('"id"', ranges)

        r = QgsRuleBasedRenderer.convertFromRenderer(g)
        self.assertEqual(r.rootRule().children()[0].filterExpression(), '"id" >= 0.000000 AND "id" <= 1.000000')
        self.assertEqual(r.rootRule().children()[1].filterExpression(), '"id" > 1.000000 AND "id" <= 2.000000')

if __name__ == '__main__':
    unittest.main()
