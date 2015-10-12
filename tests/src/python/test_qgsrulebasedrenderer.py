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

import qgis
import os

from PyQt4.QtCore import QSize

from qgis.core import (QgsVectorLayer,
                       QgsMapLayerRegistry,
                       QgsRectangle,
                       QgsMultiRenderChecker,
                       QgsRuleBasedRendererV2,
                       QgsFillSymbolV2,
                       QgsMarkerSymbolV2,
                       QgsRendererCategoryV2,
                       QgsCategorizedSymbolRendererV2,
                       QgsGraduatedSymbolRendererV2,
                       QgsRendererRangeV2
                       )
from utilities import (unitTestDataPath,
                       getQgisTestApp,
                       TestCase,
                       unittest
                       )
# Convenience instances in case you may need them
# not used in this test
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsRulebasedRenderer(TestCase):

    def setUp(self):
        myShpFile = os.path.join(TEST_DATA_DIR, 'rectangles.shp')
        layer = QgsVectorLayer(myShpFile, 'Points', 'ogr')
        QgsMapLayerRegistry.instance().addMapLayer(layer)

        # Create rulebased style
        sym1 = QgsFillSymbolV2.createSimple({'color': '#fdbf6f'})
        sym2 = QgsFillSymbolV2.createSimple({'color': '#71bd6c'})
        sym3 = QgsFillSymbolV2.createSimple({'color': '#1f78b4'})

        self.r1 = QgsRuleBasedRendererV2.Rule(sym1, 0, 0, '"id" = 1')
        self.r2 = QgsRuleBasedRendererV2.Rule(sym2, 0, 0, '"id" = 2')
        self.r3 = QgsRuleBasedRendererV2.Rule(sym3, 0, 0, 'ELSE')

        self.rootrule = QgsRuleBasedRendererV2.Rule(None)
        self.rootrule.appendChild(self.r1)
        self.rootrule.appendChild(self.r2)
        self.rootrule.appendChild(self.r3)

        self.renderer = QgsRuleBasedRendererV2(self.rootrule)
        layer.setRendererV2(self.renderer)
        self.mapsettings = CANVAS.mapSettings()
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
        #Test refining rule with categories (refs #10815)

        #First, try with a field based category (id)
        cats = []
        cats.append(QgsRendererCategoryV2(1, QgsMarkerSymbolV2(), "id 1"))
        cats.append(QgsRendererCategoryV2(2, QgsMarkerSymbolV2(), "id 2"))
        c = QgsCategorizedSymbolRendererV2("id", cats)

        QgsRuleBasedRendererV2.refineRuleCategories(self.r2, c)
        assert self.r2.children()[0].filterExpression() == '"id" = 1'
        assert self.r2.children()[1].filterExpression() == '"id" = 2'

        #Next try with an expression based category
        cats = []
        cats.append(QgsRendererCategoryV2(1, QgsMarkerSymbolV2(), "result 1"))
        cats.append(QgsRendererCategoryV2(2, QgsMarkerSymbolV2(), "result 2"))
        c = QgsCategorizedSymbolRendererV2("id + 1", cats)

        QgsRuleBasedRendererV2.refineRuleCategories(self.r1, c)
        assert self.r1.children()[0].filterExpression() == 'id + 1 = 1'
        assert self.r1.children()[1].filterExpression() == 'id + 1 = 2'

        #Last try with an expression which is just a quoted field name
        cats = []
        cats.append(QgsRendererCategoryV2(1, QgsMarkerSymbolV2(), "result 1"))
        cats.append(QgsRendererCategoryV2(2, QgsMarkerSymbolV2(), "result 2"))
        c = QgsCategorizedSymbolRendererV2('"id"', cats)

        QgsRuleBasedRendererV2.refineRuleCategories(self.r3, c)
        assert self.r3.children()[0].filterExpression() == '"id" = 1'
        assert self.r3.children()[1].filterExpression() == '"id" = 2'

    def testRefineWithRanges(self):
        #Test refining rule with ranges (refs #10815)

        #First, try with a field based category (id)
        ranges = []
        ranges.append(QgsRendererRangeV2(0, 1, QgsMarkerSymbolV2(), "0-1"))
        ranges.append(QgsRendererRangeV2(1, 2, QgsMarkerSymbolV2(), "1-2"))
        g = QgsGraduatedSymbolRendererV2("id", ranges)

        QgsRuleBasedRendererV2.refineRuleRanges(self.r2, g)
        assert self.r2.children()[0].filterExpression() == '"id" >= 0.0000 AND "id" <= 1.0000'
        assert self.r2.children()[1].filterExpression() == '"id" > 1.0000 AND "id" <= 2.0000'

        #Next try with an expression based range
        ranges = []
        ranges.append(QgsRendererRangeV2(0, 1, QgsMarkerSymbolV2(), "0-1"))
        ranges.append(QgsRendererRangeV2(1, 2, QgsMarkerSymbolV2(), "1-2"))
        g = QgsGraduatedSymbolRendererV2("id / 2", ranges)

        QgsRuleBasedRendererV2.refineRuleRanges(self.r1, g)
        assert self.r1.children()[0].filterExpression() == '(id / 2) >= 0.0000 AND (id / 2) <= 1.0000'
        assert self.r1.children()[1].filterExpression() == '(id / 2) > 1.0000 AND (id / 2) <= 2.0000'

        #Last try with an expression which is just a quoted field name
        ranges = []
        ranges.append(QgsRendererRangeV2(0, 1, QgsMarkerSymbolV2(), "0-1"))
        ranges.append(QgsRendererRangeV2(1, 2, QgsMarkerSymbolV2(), "1-2"))
        g = QgsGraduatedSymbolRendererV2('"id"', ranges)

        QgsRuleBasedRendererV2.refineRuleRanges(self.r3, g)
        assert self.r3.children()[0].filterExpression() == '"id" >= 0.0000 AND "id" <= 1.0000'
        assert self.r3.children()[1].filterExpression() == '"id" > 1.0000 AND "id" <= 2.0000'

if __name__ == '__main__':
    unittest.main()
