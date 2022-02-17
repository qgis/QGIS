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

From build dir, run: ctest -R PyQgsRulebasedRenderer -V

"""

__author__ = 'Matthias Kuhn'
__date__ = 'September 2015'
__copyright__ = '(C) 2015, Matthiasd Kuhn'

import qgis  # NOQA

import os

from qgis.PyQt.QtCore import Qt, QSize, QVariant
from qgis.PyQt.QtGui import QColor

from qgis.core import (QgsVectorLayer,
                       QgsMapSettings,
                       QgsProject,
                       QgsRectangle,
                       QgsMultiRenderChecker,
                       QgsRuleBasedRenderer,
                       QgsFillSymbol,
                       QgsMarkerSymbol,
                       QgsRendererCategory,
                       QgsCategorizedSymbolRenderer,
                       QgsGraduatedSymbolRenderer,
                       QgsRendererRange,
                       QgsRenderContext,
                       QgsSymbolLayer,
                       QgsSimpleMarkerSymbolLayer,
                       QgsProperty,
                       QgsFeature,
                       QgsField,
                       QgsGeometry,
                       QgsEmbeddedSymbolRenderer
                       )
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

# Convenience instances in case you may need them
# not used in this test
start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsRulebasedRenderer(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        # Super ugly hack to make sure python does not clean up our mapsetting objects
        # this might lead to occasional crashes on travis
        cls.mapsettings_archive = list()

    def setUp(self):
        myShpFile = os.path.join(TEST_DATA_DIR, 'rectangles.shp')
        layer = QgsVectorLayer(myShpFile, 'Rectangles', 'ogr')
        vfield = QgsField('fa_cy-fie+ld', QVariant.Int)
        layer.addExpressionField('"id"', vfield)
        QgsProject.instance().addMapLayer(layer)

        # Create rulebased style
        sym1 = QgsFillSymbol.createSimple({'color': '#fdbf6f', 'outline_color': 'black'})
        sym2 = QgsFillSymbol.createSimple({'color': '#71bd6c', 'outline_color': 'black'})
        sym3 = QgsFillSymbol.createSimple({'color': '#1f78b4', 'outline_color': 'black'})

        self.r1 = QgsRuleBasedRenderer.Rule(sym1, 0, 0, '"id" = 1')
        self.r2 = QgsRuleBasedRenderer.Rule(sym2, 0, 0, '"id" = 2')
        self.r3 = QgsRuleBasedRenderer.Rule(sym3, 0, 0, 'ELSE')

        rootrule = QgsRuleBasedRenderer.Rule(None)
        rootrule.appendChild(self.r1)
        rootrule.appendChild(self.r2)
        rootrule.appendChild(self.r3)

        layer.setRenderer(QgsRuleBasedRenderer(rootrule))
        self.mapsettings = QgsMapSettings()
        self.mapsettings.setOutputSize(QSize(400, 400))
        self.mapsettings.setOutputDpi(96)
        self.mapsettings.setExtent(QgsRectangle(-163, 22, -70, 52))

        rendered_layers = [layer]
        self.mapsettings.setLayers(rendered_layers)
        self.mapsettings_archive.append(self.mapsettings)

    def testElse(self):
        # Setup rendering check
        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlName('expected_rulebased_else')
        self.assertTrue(renderchecker.runTest('rulebased_else'))

    def testDisabledElse(self):
        # Disable a rule and assert that it's hidden not rendered with else
        self.r2.setActive(False)

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlName('expected_rulebased_disabled_else')
        self.assertTrue(renderchecker.runTest('rulebased_disabled_else'))

    def testWillRenderFeature(self):
        vl = self.mapsettings.layers()[0]
        ft = vl.getFeature(0)  # 'id' = 1
        renderer = vl.renderer()

        ctx = QgsRenderContext.fromMapSettings(self.mapsettings)
        ctx.expressionContext().setFeature(ft)

        renderer.rootRule().children()[0].setActive(False)
        renderer.rootRule().children()[1].setActive(True)
        renderer.rootRule().children()[2].setActive(True)

        renderer.startRender(ctx, vl.fields())  # build mActiveChlidren
        rendered = renderer.willRenderFeature(ft, ctx)
        renderer.stopRender(ctx)
        renderer.rootRule().children()[0].setActive(True)
        self.assertFalse(rendered)

        renderer.startRender(ctx, vl.fields())  # build mActiveChlidren
        rendered = renderer.willRenderFeature(ft, ctx)
        renderer.stopRender(ctx)
        self.assertTrue(rendered)

    def testGroupAndElseRules(self):
        vl = self.mapsettings.layers()[0]

        # Create rulebased style
        sym1 = QgsFillSymbol.createSimple({'color': '#fdbf6f', 'outline_color': 'black'})
        sym2 = QgsFillSymbol.createSimple({'color': '#71bd6c', 'outline_color': 'black'})
        sym3 = QgsFillSymbol.createSimple({'color': '#1f78b4', 'outline_color': 'black'})

        self.rx1 = QgsRuleBasedRenderer.Rule(None, 0, 0, '"id" < 3')
        self.rx2 = QgsRuleBasedRenderer.Rule(sym3, 0, 0, 'ELSE')

        self.subrx1 = QgsRuleBasedRenderer.Rule(sym1, 0, 0, '"id" = 1')
        self.subrx2 = QgsRuleBasedRenderer.Rule(sym2, 0, 0, '"id" = 2')
        self.rx1.appendChild(self.subrx1)
        self.rx1.appendChild(self.subrx2)

        rootrule = QgsRuleBasedRenderer.Rule(None)
        rootrule.appendChild(self.rx1)
        rootrule.appendChild(self.rx2)
        rootrule.children()[0].children()[0].setActive(False)
        rootrule.children()[0].children()[1].setActive(False)

        vl.setRenderer(QgsRuleBasedRenderer(rootrule))

        renderchecker = QgsMultiRenderChecker()
        renderchecker.setMapSettings(self.mapsettings)
        renderchecker.setControlName('expected_rulebased_group_else')
        self.assertTrue(renderchecker.runTest('rulebased_group_else'))

    def testWillRenderFeatureNestedElse(self):
        vl = self.mapsettings.layers()[0]
        ft = vl.getFeature(0)  # 'id' = 1

        ctx = QgsRenderContext.fromMapSettings(self.mapsettings)
        ctx.expressionContext().setFeature(ft)

        # Create rulebased style
        sym1 = QgsFillSymbol.createSimple({'color': '#fdbf6f', 'outline_color': 'black'})
        sym2 = QgsFillSymbol.createSimple({'color': '#71bd6c', 'outline_color': 'black'})
        sym3 = QgsFillSymbol.createSimple({'color': '#1f78b4', 'outline_color': 'black'})

        self.rx1 = QgsRuleBasedRenderer.Rule(sym1, 0, 0, '"id" = 1')
        self.rx2 = QgsRuleBasedRenderer.Rule(sym2, 0, 0, '"id" = 2')
        self.rx3 = QgsRuleBasedRenderer.Rule(sym3, 0, 0, 'ELSE')

        self.rx3.appendChild(self.rx1)

        rootrule = QgsRuleBasedRenderer.Rule(None)
        rootrule.appendChild(self.rx2)
        rootrule.appendChild(self.rx3)

        vl.setRenderer(QgsRuleBasedRenderer(rootrule))
        renderer = vl.renderer()

        # Render with else rule and all activated
        renderer.startRender(ctx, vl.fields())
        self.assertTrue(renderer.willRenderFeature(ft, ctx))
        renderer.stopRender(ctx)

        # Render with else rule where else is deactivated
        renderer.rootRule().children()[1].setActive(False)
        renderer.startRender(ctx, vl.fields())
        self.assertFalse(renderer.willRenderFeature(ft, ctx))
        renderer.stopRender(ctx)

    def testFeatureCount(self):
        vl = self.mapsettings.layers()[0]
        ft = vl.getFeature(2)  # 'id' = 3 => ELSE
        renderer = vl.renderer()

        ctx = QgsRenderContext.fromMapSettings(self.mapsettings)
        ctx.expressionContext().setFeature(ft)

        counter = vl.countSymbolFeatures()
        counter.waitForFinished()

        renderer.startRender(ctx, vl.fields())

        elseRule = None
        for rule in renderer.rootRule().children():
            if rule.filterExpression() == 'ELSE':
                elseRule = rule

        self.assertIsNotNone(elseRule)

        cnt = counter.featureCount(elseRule.ruleKey())
        self.assertEqual(cnt, 1)

    def testRefineWithCategories(self):
        # Test refining rule with categories (refs #10815)

        # First, try with a field based category (id)
        cats = []
        cats.append(QgsRendererCategory(1, QgsMarkerSymbol(), "id 1"))
        cats.append(QgsRendererCategory(2, QgsMarkerSymbol(), ''))
        cats.append(QgsRendererCategory(None, QgsMarkerSymbol(), ''))
        c = QgsCategorizedSymbolRenderer("id", cats)

        QgsRuleBasedRenderer.refineRuleCategories(self.r2, c)
        self.assertEqual(self.r2.children()[0].filterExpression(), '"id" = 1')
        self.assertEqual(self.r2.children()[1].filterExpression(), '"id" = 2')
        self.assertEqual(self.r2.children()[2].filterExpression(), '"id" IS NULL')
        self.assertEqual(self.r2.children()[0].label(), 'id 1')
        self.assertEqual(self.r2.children()[1].label(), '2')
        self.assertEqual(self.r2.children()[2].label(), '')

        # Next try with an expression based category
        cats = []
        cats.append(QgsRendererCategory(1, QgsMarkerSymbol(), "result 1"))
        cats.append(QgsRendererCategory(2, QgsMarkerSymbol(), "result 2"))
        c = QgsCategorizedSymbolRenderer("id + 1", cats)

        QgsRuleBasedRenderer.refineRuleCategories(self.r1, c)
        self.assertEqual(self.r1.children()[0].filterExpression(), 'id + 1 = 1')
        self.assertEqual(self.r1.children()[1].filterExpression(), 'id + 1 = 2')
        self.assertEqual(self.r1.children()[0].label(), 'result 1')
        self.assertEqual(self.r1.children()[1].label(), 'result 2')

        # Last try with an expression which is just a quoted field name
        cats = []
        cats.append(QgsRendererCategory(1, QgsMarkerSymbol(), "result 1"))
        cats.append(QgsRendererCategory(2, QgsMarkerSymbol(), "result 2"))
        c = QgsCategorizedSymbolRenderer('"id"', cats)

        QgsRuleBasedRenderer.refineRuleCategories(self.r3, c)
        self.assertEqual(self.r3.children()[0].filterExpression(), '"id" = 1')
        self.assertEqual(self.r3.children()[1].filterExpression(), '"id" = 2')
        self.assertEqual(self.r3.children()[0].label(), 'result 1')
        self.assertEqual(self.r3.children()[1].label(), 'result 2')

    def testRefineWithRanges(self):
        # Test refining rule with ranges (refs #10815)

        # First, try with a field based category (id)
        ranges = []
        ranges.append(QgsRendererRange(0, 1, QgsMarkerSymbol(), "0-1"))
        ranges.append(QgsRendererRange(1, 2, QgsMarkerSymbol(), "1-2"))
        g = QgsGraduatedSymbolRenderer("id", ranges)

        QgsRuleBasedRenderer.refineRuleRanges(self.r2, g)
        self.assertEqual(self.r2.children()[0].filterExpression(), '"id" >= 0.0000 AND "id" <= 1.0000')
        self.assertEqual(self.r2.children()[1].filterExpression(), '"id" > 1.0000 AND "id" <= 2.0000')

        # Next try with an expression based range
        ranges = []
        ranges.append(QgsRendererRange(0, 1, QgsMarkerSymbol(), "0-1"))
        ranges.append(QgsRendererRange(1, 2, QgsMarkerSymbol(), "1-2"))
        g = QgsGraduatedSymbolRenderer("id / 2", ranges)

        QgsRuleBasedRenderer.refineRuleRanges(self.r1, g)
        self.assertEqual(self.r1.children()[0].filterExpression(), '(id / 2) >= 0.0000 AND (id / 2) <= 1.0000')
        self.assertEqual(self.r1.children()[1].filterExpression(), '(id / 2) > 1.0000 AND (id / 2) <= 2.0000')

        # Last try with an expression which is just a quoted field name
        ranges = []
        ranges.append(QgsRendererRange(0, 1, QgsMarkerSymbol(), "0-1"))
        ranges.append(QgsRendererRange(1, 2, QgsMarkerSymbol(), "1-2"))
        g = QgsGraduatedSymbolRenderer('"id"', ranges)

        QgsRuleBasedRenderer.refineRuleRanges(self.r3, g)
        self.assertEqual(self.r3.children()[0].filterExpression(), '"id" >= 0.0000 AND "id" <= 1.0000')
        self.assertEqual(self.r3.children()[1].filterExpression(), '"id" > 1.0000 AND "id" <= 2.0000')

    def testConvertFromCategorisedRenderer(self):
        # Test converting categorised renderer to rule based
        vl = self.mapsettings.layers()[0]
        # First, try with a field based category (id)
        cats = []
        cats.append(QgsRendererCategory(1, QgsMarkerSymbol(), "id 1"))
        cats.append(QgsRendererCategory(2, QgsMarkerSymbol(), "id 2"))
        cats.append(QgsRendererCategory('a\'b', QgsMarkerSymbol(), "id a'b"))
        cats.append(QgsRendererCategory('a\nb', QgsMarkerSymbol(), "id a\\nb"))
        cats.append(QgsRendererCategory('a\\b', QgsMarkerSymbol(), "id a\\\\b"))
        cats.append(QgsRendererCategory('a\tb', QgsMarkerSymbol(), "id a\\tb"))
        cats.append(QgsRendererCategory(['c', 'd'], QgsMarkerSymbol(), "c/d"))
        c = QgsCategorizedSymbolRenderer("id", cats)

        r = QgsRuleBasedRenderer.convertFromRenderer(c, vl)
        self.assertEqual(len(r.rootRule().children()), 7)
        self.assertEqual(r.rootRule().children()[0].filterExpression(), '"id" = 1')
        self.assertEqual(r.rootRule().children()[1].filterExpression(), '"id" = 2')
        self.assertEqual(r.rootRule().children()[2].filterExpression(), '"id" = \'a\'\'b\'')
        self.assertEqual(r.rootRule().children()[3].filterExpression(), '"id" = \'a\\nb\'')
        self.assertEqual(r.rootRule().children()[4].filterExpression(), '"id" = \'a\\\\b\'')
        self.assertEqual(r.rootRule().children()[5].filterExpression(), '"id" = \'a\\tb\'')
        self.assertEqual(r.rootRule().children()[6].filterExpression(), '"id" IN (\'c\',\'d\')')

        # Next try with an expression based category
        cats = []
        cats.append(QgsRendererCategory(1, QgsMarkerSymbol(), "result 1"))
        cats.append(QgsRendererCategory(2, QgsMarkerSymbol(), "result 2"))
        cats.append(QgsRendererCategory([3, 4], QgsMarkerSymbol(), "result 3/4"))
        c = QgsCategorizedSymbolRenderer("id + 1", cats)

        r = QgsRuleBasedRenderer.convertFromRenderer(c, vl)
        self.assertEqual(len(r.rootRule().children()), 3)
        self.assertEqual(r.rootRule().children()[0].filterExpression(), 'id + 1 = 1')
        self.assertEqual(r.rootRule().children()[1].filterExpression(), 'id + 1 = 2')
        self.assertEqual(r.rootRule().children()[2].filterExpression(), 'id + 1 IN (3,4)')

        # Last try with an expression which is just a quoted field name
        cats = []
        cats.append(QgsRendererCategory(1, QgsMarkerSymbol(), "result 1"))
        cats.append(QgsRendererCategory(2, QgsMarkerSymbol(), "result 2"))
        cats.append(QgsRendererCategory([3, 4], QgsMarkerSymbol(), "result 3/4"))
        c = QgsCategorizedSymbolRenderer('"id"', cats)

        r = QgsRuleBasedRenderer.convertFromRenderer(c, vl)
        self.assertEqual(len(r.rootRule().children()), 3)
        self.assertEqual(r.rootRule().children()[0].filterExpression(), '"id" = 1')
        self.assertEqual(r.rootRule().children()[1].filterExpression(), '"id" = 2')
        self.assertEqual(r.rootRule().children()[2].filterExpression(), '"id" IN (3,4)')

        # Next try with a complex name
        cats = []
        cats.append(QgsRendererCategory(1, QgsMarkerSymbol(), "fa_cy-fie+ld 1"))
        cats.append(QgsRendererCategory(2, QgsMarkerSymbol(), "fa_cy-fie+ld 2"))
        c = QgsCategorizedSymbolRenderer("fa_cy-fie+ld", cats)

        r = QgsRuleBasedRenderer.convertFromRenderer(c, vl)
        self.assertEqual(r.rootRule().children()[0].filterExpression(), '"fa_cy-fie+ld" = 1')
        self.assertEqual(r.rootRule().children()[1].filterExpression(), '"fa_cy-fie+ld" = 2')

    def testConvertFromCategorisedRendererNoLayer(self):
        # Test converting categorised renderer to rule based

        # First, try with a field based category (id)
        cats = []
        cats.append(QgsRendererCategory(1, QgsMarkerSymbol(), "id 1"))
        cats.append(QgsRendererCategory(2, QgsMarkerSymbol(), "id 2"))
        cats.append(QgsRendererCategory('a\'b', QgsMarkerSymbol(), "id a'b"))
        cats.append(QgsRendererCategory('a\nb', QgsMarkerSymbol(), "id a\\nb"))
        cats.append(QgsRendererCategory('a\\b', QgsMarkerSymbol(), "id a\\\\b"))
        cats.append(QgsRendererCategory('a\tb', QgsMarkerSymbol(), "id a\\tb"))
        cats.append(QgsRendererCategory(['c', 'd'], QgsMarkerSymbol(), "c/d"))
        c = QgsCategorizedSymbolRenderer("id", cats)

        r = QgsRuleBasedRenderer.convertFromRenderer(c)
        self.assertEqual(len(r.rootRule().children()), 7)
        self.assertEqual(r.rootRule().children()[0].filterExpression(), '"id" = 1')
        self.assertEqual(r.rootRule().children()[1].filterExpression(), '"id" = 2')
        self.assertEqual(r.rootRule().children()[2].filterExpression(), '"id" = \'a\'\'b\'')
        self.assertEqual(r.rootRule().children()[3].filterExpression(), '"id" = \'a\\nb\'')
        self.assertEqual(r.rootRule().children()[4].filterExpression(), '"id" = \'a\\\\b\'')
        self.assertEqual(r.rootRule().children()[5].filterExpression(), '"id" = \'a\\tb\'')
        self.assertEqual(r.rootRule().children()[6].filterExpression(), '"id" IN (\'c\',\'d\')')

        # Next try with an expression based category
        cats = []
        cats.append(QgsRendererCategory(1, QgsMarkerSymbol(), "result 1"))
        cats.append(QgsRendererCategory(2, QgsMarkerSymbol(), "result 2"))
        cats.append(QgsRendererCategory([3, 4], QgsMarkerSymbol(), "result 3/4"))
        c = QgsCategorizedSymbolRenderer("id + 1", cats)

        r = QgsRuleBasedRenderer.convertFromRenderer(c)
        self.assertEqual(len(r.rootRule().children()), 3)
        self.assertEqual(r.rootRule().children()[0].filterExpression(), 'id + 1 = 1')
        self.assertEqual(r.rootRule().children()[1].filterExpression(), 'id + 1 = 2')
        self.assertEqual(r.rootRule().children()[2].filterExpression(), 'id + 1 IN (3,4)')

        # Last try with an expression which is just a quoted field name
        cats = []
        cats.append(QgsRendererCategory(1, QgsMarkerSymbol(), "result 1"))
        cats.append(QgsRendererCategory(2, QgsMarkerSymbol(), "result 2"))
        cats.append(QgsRendererCategory([3, 4], QgsMarkerSymbol(), "result 3/4"))
        c = QgsCategorizedSymbolRenderer('"id"', cats)

        r = QgsRuleBasedRenderer.convertFromRenderer(c)
        self.assertEqual(len(r.rootRule().children()), 3)
        self.assertEqual(r.rootRule().children()[0].filterExpression(), '"id" = 1')
        self.assertEqual(r.rootRule().children()[1].filterExpression(), '"id" = 2')
        self.assertEqual(r.rootRule().children()[2].filterExpression(), '"id" IN (3,4)')

        # Next try with a complex name -- in this case since we don't have a layer or
        # actual field names available, we must assume the complex field name is actually an expression
        cats = []
        cats.append(QgsRendererCategory(1, QgsMarkerSymbol(), "fa_cy-fie+ld 1"))
        cats.append(QgsRendererCategory(2, QgsMarkerSymbol(), "fa_cy-fie+ld 2"))
        c = QgsCategorizedSymbolRenderer("fa_cy-fie+ld", cats)

        r = QgsRuleBasedRenderer.convertFromRenderer(c)
        self.assertEqual(r.rootRule().children()[0].filterExpression(), 'fa_cy-fie+ld = 1')
        self.assertEqual(r.rootRule().children()[1].filterExpression(), 'fa_cy-fie+ld = 2')

    def testConvertFromGraduatedRenderer(self):
        # Test converting graduated renderer to rule based
        vl = self.mapsettings.layers()[0]
        # First, try with a field based category (id)
        ranges = []
        ranges.append(QgsRendererRange(0, 1, QgsMarkerSymbol(), "0-1"))
        ranges.append(QgsRendererRange(1, 2, QgsMarkerSymbol(), "1-2"))
        g = QgsGraduatedSymbolRenderer("id", ranges)

        r = QgsRuleBasedRenderer.convertFromRenderer(g, vl)
        self.assertEqual(r.rootRule().children()[0].filterExpression(), '"id" >= 0.000000 AND "id" <= 1.000000')
        self.assertEqual(r.rootRule().children()[1].filterExpression(), '"id" > 1.000000 AND "id" <= 2.000000')

        # Next try with an expression based range
        ranges = []
        ranges.append(QgsRendererRange(0, 1, QgsMarkerSymbol(), "0-1"))
        ranges.append(QgsRendererRange(1, 2, QgsMarkerSymbol(), "1-2"))
        g = QgsGraduatedSymbolRenderer("id / 2", ranges)

        r = QgsRuleBasedRenderer.convertFromRenderer(g, vl)
        self.assertEqual(r.rootRule().children()[0].filterExpression(), '(id / 2) >= 0.000000 AND (id / 2) <= 1.000000')
        self.assertEqual(r.rootRule().children()[1].filterExpression(), '(id / 2) > 1.000000 AND (id / 2) <= 2.000000')

        # Last try with an expression which is just a quoted field name
        ranges = []
        ranges.append(QgsRendererRange(0, 1, QgsMarkerSymbol(), "0-1"))
        ranges.append(QgsRendererRange(1, 2, QgsMarkerSymbol(), "1-2"))
        g = QgsGraduatedSymbolRenderer('"id"', ranges)

        r = QgsRuleBasedRenderer.convertFromRenderer(g, vl)
        self.assertEqual(r.rootRule().children()[0].filterExpression(), '"id" >= 0.000000 AND "id" <= 1.000000')
        self.assertEqual(r.rootRule().children()[1].filterExpression(), '"id" > 1.000000 AND "id" <= 2.000000')

        # Test with a complex field name
        ranges = []
        ranges.append(QgsRendererRange(0, 1, QgsMarkerSymbol(), "0-1"))
        ranges.append(QgsRendererRange(1, 2, QgsMarkerSymbol(), "1-2"))
        g = QgsGraduatedSymbolRenderer("fa_cy-fie+ld", ranges)

        r = QgsRuleBasedRenderer.convertFromRenderer(g, vl)
        self.assertEqual(r.rootRule().children()[0].filterExpression(), '"fa_cy-fie+ld" >= 0.000000 AND "fa_cy-fie+ld" <= 1.000000')
        self.assertEqual(r.rootRule().children()[1].filterExpression(), '"fa_cy-fie+ld" > 1.000000 AND "fa_cy-fie+ld" <= 2.000000')

    def testConvertFromGraduatedRendererNoLayer(self):
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

        # Next try with a complex name -- in this case since we don't have a layer or
        # actual field names available, we must assume the complex field name is actually an expression
        ranges = []
        ranges.append(QgsRendererRange(0, 1, QgsMarkerSymbol(), "0-1"))
        ranges.append(QgsRendererRange(1, 2, QgsMarkerSymbol(), "1-2"))
        g = QgsGraduatedSymbolRenderer("fa_cy-fie+ld", ranges)

        r = QgsRuleBasedRenderer.convertFromRenderer(g)
        self.assertEqual(r.rootRule().children()[0].filterExpression(), '(fa_cy-fie+ld) >= 0.000000 AND (fa_cy-fie+ld) <= 1.000000')
        self.assertEqual(r.rootRule().children()[1].filterExpression(), '(fa_cy-fie+ld) > 1.000000 AND (fa_cy-fie+ld) <= 2.000000')

    def testWillRenderFeatureTwoElse(self):
        """Regression #21287, also test rulesForFeature since there were no tests any where and I've found a couple of issues"""

        vl = self.mapsettings.layers()[0]
        ft = vl.getFeature(0)  # 'id' = 1

        ctx = QgsRenderContext.fromMapSettings(self.mapsettings)
        ctx.expressionContext().setFeature(ft)

        # Create rulebased style
        sym2 = QgsFillSymbol.createSimple({'color': '#71bd6c', 'outline_color': 'black'})
        sym3 = QgsFillSymbol.createSimple({'color': '#1f78b4', 'outline_color': 'black'})
        sym4 = QgsFillSymbol.createSimple({'color': '#ff00ff', 'outline_color': 'black'})

        self.rx2 = QgsRuleBasedRenderer.Rule(sym2, 0, 0, '"id" = 200')
        self.rx3 = QgsRuleBasedRenderer.Rule(sym3, 1000, 100000000, 'ELSE')  # <<< - match this!
        self.rx4 = QgsRuleBasedRenderer.Rule(sym4, 1, 999, 'ELSE')

        rootrule = QgsRuleBasedRenderer.Rule(None)
        rootrule.appendChild(self.rx2)
        rootrule.appendChild(self.rx3)
        rootrule.appendChild(self.rx4)  # <- failed in regression #21287

        vl.setRenderer(QgsRuleBasedRenderer(rootrule))
        renderer = vl.renderer()

        # Render with else rule and all activated
        renderer.startRender(ctx, vl.fields())
        self.assertTrue(renderer.willRenderFeature(ft, ctx))

        # No context? All rules
        self.assertEqual(len(rootrule.rulesForFeature(ft)), 2)
        self.assertTrue(set(rootrule.rulesForFeature(ft)), set([self.rx3, self.rx4]))

        # With context: only the matching one
        self.assertEqual(len(rootrule.rulesForFeature(ft, ctx)), 1)
        self.assertEqual(rootrule.rulesForFeature(ft, ctx)[0], self.rx3)
        renderer.stopRender(ctx)

    def testUsedAttributes(self):
        ctx = QgsRenderContext.fromMapSettings(self.mapsettings)

        # Create rulebased style
        sym2 = QgsFillSymbol.createSimple({'color': '#71bd6c', 'outline_color': 'black'})
        sym3 = QgsFillSymbol.createSimple({'color': '#1f78b4', 'outline_color': 'black'})

        self.rx2 = QgsRuleBasedRenderer.Rule(sym2, 0, 0, '"id" = 200')
        self.rx3 = QgsRuleBasedRenderer.Rule(sym3, 1000, 100000000, 'ELSE')

        rootrule = QgsRuleBasedRenderer.Rule(None)
        rootrule.appendChild(self.rx2)
        rootrule.appendChild(self.rx3)

        renderer = QgsRuleBasedRenderer(rootrule)

        self.assertCountEqual(renderer.usedAttributes(ctx), {'id'})

    def testPointsUsedAttributes(self):
        points_shp = os.path.join(TEST_DATA_DIR, 'points.shp')
        points_layer = QgsVectorLayer(points_shp, 'Points', 'ogr')
        QgsProject.instance().addMapLayer(points_layer)

        # Create rulebased style
        sym1 = QgsMarkerSymbol()
        l1 = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Triangle, 5)
        l1.setColor(QColor(255, 0, 0))
        l1.setStrokeStyle(Qt.NoPen)
        l1.setDataDefinedProperty(QgsSymbolLayer.PropertyAngle, QgsProperty.fromField("Heading"))
        sym1.changeSymbolLayer(0, l1)

        sym2 = QgsMarkerSymbol()
        l2 = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Triangle, 5)
        l2.setColor(QColor(0, 255, 0))
        l2.setStrokeStyle(Qt.NoPen)
        l2.setDataDefinedProperty(QgsSymbolLayer.PropertyAngle, QgsProperty.fromField("Heading"))
        sym2.changeSymbolLayer(0, l2)

        sym3 = QgsMarkerSymbol()
        l3 = QgsSimpleMarkerSymbolLayer(QgsSimpleMarkerSymbolLayer.Triangle, 5)
        l3.setColor(QColor(0, 0, 255))
        l3.setStrokeStyle(Qt.NoPen)
        l3.setDataDefinedProperty(QgsSymbolLayer.PropertyAngle, QgsProperty.fromField("Heading"))
        sym3.changeSymbolLayer(0, l3)

        r1 = QgsRuleBasedRenderer.Rule(sym1, 0, 0, '"Class" = \'B52\'')
        r2 = QgsRuleBasedRenderer.Rule(sym2, 0, 0, '"Class" = \'Biplane\'')
        r3 = QgsRuleBasedRenderer.Rule(sym3, 0, 0, '"Class" = \'Jet\'')

        rootrule = QgsRuleBasedRenderer.Rule(None)
        rootrule.appendChild(r1)
        rootrule.appendChild(r2)
        rootrule.appendChild(r3)

        renderer = QgsRuleBasedRenderer(rootrule)

        points_layer.setRenderer(renderer)

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(400, 400))
        ms.setOutputDpi(96)
        ms.setExtent(QgsRectangle(-133, 22, -70, 52))
        ms.setLayers([points_layer])

        ctx = QgsRenderContext.fromMapSettings(ms)
        ctx.expressionContext().appendScope(points_layer.createExpressionContextScope())

        # for symbol layer
        self.assertCountEqual(l1.usedAttributes(ctx), {'Heading'})
        # for symbol
        self.assertCountEqual(sym1.usedAttributes(ctx), {'Heading'})
        # for symbol renderer
        self.assertCountEqual(renderer.usedAttributes(ctx), {'Class', 'Heading'})

        QgsProject.instance().removeMapLayer(points_layer)

    def testConvertFromEmbedded(self):
        """
        Test converting an embedded symbol renderer to a rule based renderer
        """
        points_layer = QgsVectorLayer('Point', 'Polys', 'memory')
        f = QgsFeature()
        f.setGeometry(QgsGeometry.fromWkt('Point(-100 30)'))
        f.setEmbeddedSymbol(
            QgsMarkerSymbol.createSimple({'name': 'triangle', 'size': 10, 'color': '#ff0000', 'outline_style': 'no'}))
        self.assertTrue(points_layer.dataProvider().addFeature(f))
        f.setGeometry(QgsGeometry.fromWkt('Point(-110 40)'))
        f.setEmbeddedSymbol(
            QgsMarkerSymbol.createSimple({'name': 'square', 'size': 7, 'color': '#00ff00', 'outline_style': 'no'}))
        self.assertTrue(points_layer.dataProvider().addFeature(f))
        f.setGeometry(QgsGeometry.fromWkt('Point(-90 50)'))
        f.setEmbeddedSymbol(None)
        self.assertTrue(points_layer.dataProvider().addFeature(f))

        renderer = QgsEmbeddedSymbolRenderer(defaultSymbol=QgsMarkerSymbol.createSimple({'name': 'star', 'size': 10, 'color': '#ff00ff', 'outline_style': 'no'}))
        points_layer.setRenderer(renderer)

        rule_based = QgsRuleBasedRenderer.convertFromRenderer(renderer, points_layer)
        self.assertEqual(len(rule_based.rootRule().children()), 3)
        rule_0 = rule_based.rootRule().children()[0]
        self.assertEqual(rule_0.filterExpression(), '$id=1')
        self.assertEqual(rule_0.label(), '1')
        self.assertEqual(rule_0.symbol().color().name(), '#ff0000')
        rule_1 = rule_based.rootRule().children()[1]
        self.assertEqual(rule_1.filterExpression(), '$id=2')
        self.assertEqual(rule_1.label(), '2')
        self.assertEqual(rule_1.symbol().color().name(), '#00ff00')
        rule_2 = rule_based.rootRule().children()[2]
        self.assertEqual(rule_2.filterExpression(), 'ELSE')
        self.assertEqual(rule_2.label(), 'All other features')
        self.assertEqual(rule_2.symbol().color().name(), '#ff00ff')

    def testNullsCount(self):

        vl = QgsVectorLayer('Point?crs=epsg:4326&field=number:int',
                            'test', 'memory')

        f = QgsFeature(vl.fields())
        f.setAttribute(0, 0)
        f.setGeometry(QgsGeometry.fromWkt('point(7 45)'))
        vl.dataProvider().addFeatures([f])
        f = QgsFeature(vl.fields())
        f.setGeometry(QgsGeometry.fromWkt('point(7 45)'))
        f.setAttribute(0, 1)
        vl.dataProvider().addFeatures([f])
        f.setGeometry(QgsGeometry.fromWkt('point(7 45)'))
        f = QgsFeature(vl.fields())
        vl.dataProvider().addFeatures([f])

        cats = []
        cats.append(QgsRendererCategory(1, QgsMarkerSymbol(), 'one'))
        cats.append(QgsRendererCategory(0, QgsMarkerSymbol(), 'zero'))
        cats.append(QgsRendererCategory(None, QgsMarkerSymbol(), 'NULL'))
        renderer = QgsCategorizedSymbolRenderer("number", cats)

        vl.setRenderer(renderer)

        counter = vl.countSymbolFeatures()
        counter.waitForFinished()

        self.assertEqual(counter.featureCount('0'), 1)
        self.assertEqual(counter.featureCount('1'), 1)
        self.assertEqual(counter.featureCount('2'), 1)


if __name__ == '__main__':
    unittest.main()
