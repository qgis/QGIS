# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutAtlas

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '19/12/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA
import sip
import tempfile
import shutil
import os

from qgis.core import (QgsUnitTypes,
                       QgsLayout,
                       QgsPrintLayout,
                       QgsLayoutAtlas,
                       QgsLayoutItemPage,
                       QgsLayoutGuide,
                       QgsLayoutObject,
                       QgsProject,
                       QgsLayoutItemGroup,
                       QgsLayoutItem,
                       QgsProperty,
                       QgsLayoutPageCollection,
                       QgsLayoutMeasurement,
                       QgsFillSymbol,
                       QgsReadWriteContext,
                       QgsLayoutItemMap,
                       QgsLayoutItemLabel,
                       QgsLayoutSize,
                       QgsLayoutPoint,
                       QgsVectorLayer)
from qgis.PyQt.QtCore import QFileInfo
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from utilities import unitTestDataPath
from qgis.testing import start_app, unittest
from qgis.PyQt.QtTest import QSignalSpy

start_app()


class TestQgsLayoutAtlas(unittest.TestCase):

    def testReadWriteXml(self):
        p = QgsProject()
        vectorFileInfo = QFileInfo(unitTestDataPath() + "/france_parts.shp")
        vector_layer = QgsVectorLayer(vectorFileInfo.filePath(), vectorFileInfo.completeBaseName(), "ogr")
        self.assertTrue(vector_layer.isValid())
        p.addMapLayer(vector_layer)

        l = QgsPrintLayout(p)
        atlas = l.atlas()
        atlas.setEnabled(True)
        atlas.setHideCoverage(True)
        atlas.setFilenameExpression('filename exp')
        atlas.setCoverageLayer(vector_layer)
        atlas.setPageNameExpression('page name')
        atlas.setSortFeatures(True)
        atlas.setSortAscending(False)
        atlas.setSortExpression('sort exp')
        atlas.setFilterFeatures(True)
        atlas.setFilterExpression('filter exp')

        doc = QDomDocument("testdoc")
        elem = l.writeXml(doc, QgsReadWriteContext())

        l2 = QgsPrintLayout(p)
        self.assertTrue(l2.readXml(elem, doc, QgsReadWriteContext()))
        atlas2 = l2.atlas()
        self.assertTrue(atlas2.enabled())
        self.assertTrue(atlas2.hideCoverage())
        self.assertEqual(atlas2.filenameExpression(), 'filename exp')
        self.assertEqual(atlas2.coverageLayer(), vector_layer)
        self.assertEqual(atlas2.pageNameExpression(), 'page name')
        self.assertTrue(atlas2.sortFeatures())
        self.assertFalse(atlas2.sortAscending())
        self.assertEqual(atlas2.sortExpression(), 'sort exp')
        self.assertTrue(atlas2.filterFeatures())
        self.assertEqual(atlas2.filterExpression(), 'filter exp')

    def testIteration(self):
        p = QgsProject()
        vectorFileInfo = QFileInfo(unitTestDataPath() + "/france_parts.shp")
        vector_layer = QgsVectorLayer(vectorFileInfo.filePath(), vectorFileInfo.completeBaseName(), "ogr")
        self.assertTrue(vector_layer.isValid())
        p.addMapLayer(vector_layer)

        l = QgsPrintLayout(p)
        atlas = l.atlas()
        atlas.setEnabled(True)
        atlas.setCoverageLayer(vector_layer)

        atlas_feature_changed_spy = QSignalSpy(atlas.featureChanged)
        context_changed_spy = QSignalSpy(l.context().changed)

        self.assertTrue(atlas.beginRender())
        self.assertTrue(atlas.first())
        self.assertEqual(len(atlas_feature_changed_spy), 1)
        self.assertEqual(len(context_changed_spy), 1)
        self.assertEqual(atlas.currentFeatureNumber(), 0)
        self.assertEqual(l.context().feature()[4], 'Basse-Normandie')
        self.assertEqual(l.context().layer(), vector_layer)

        self.assertTrue(atlas.next())
        self.assertEqual(len(atlas_feature_changed_spy), 2)
        self.assertEqual(len(context_changed_spy), 2)
        self.assertEqual(atlas.currentFeatureNumber(), 1)
        self.assertEqual(l.context().feature()[4], 'Bretagne')

        self.assertTrue(atlas.next())
        self.assertEqual(len(atlas_feature_changed_spy), 3)
        self.assertEqual(len(context_changed_spy), 3)
        self.assertEqual(atlas.currentFeatureNumber(), 2)
        self.assertEqual(l.context().feature()[4], 'Pays de la Loire')

        self.assertTrue(atlas.next())
        self.assertEqual(len(atlas_feature_changed_spy), 4)
        self.assertEqual(len(context_changed_spy), 4)
        self.assertEqual(atlas.currentFeatureNumber(), 3)
        self.assertEqual(l.context().feature()[4], 'Centre')

        self.assertFalse(atlas.next())
        self.assertTrue(atlas.seekTo(2))
        self.assertEqual(len(atlas_feature_changed_spy), 5)
        self.assertEqual(len(context_changed_spy), 5)
        self.assertEqual(atlas.currentFeatureNumber(), 2)
        self.assertEqual(l.context().feature()[4], 'Pays de la Loire')

        self.assertTrue(atlas.last())
        self.assertEqual(len(atlas_feature_changed_spy), 6)
        self.assertEqual(len(context_changed_spy), 6)
        self.assertEqual(atlas.currentFeatureNumber(), 3)
        self.assertEqual(l.context().feature()[4], 'Centre')

        self.assertTrue(atlas.previous())
        self.assertEqual(len(atlas_feature_changed_spy), 7)
        self.assertEqual(len(context_changed_spy), 7)
        self.assertEqual(atlas.currentFeatureNumber(), 2)
        self.assertEqual(l.context().feature()[4], 'Pays de la Loire')

        self.assertTrue(atlas.previous())
        self.assertTrue(atlas.previous())
        self.assertEqual(len(atlas_feature_changed_spy), 9)
        self.assertFalse(atlas.previous())
        self.assertEqual(len(atlas_feature_changed_spy), 9)

        self.assertTrue(atlas.endRender())
        self.assertEqual(len(atlas_feature_changed_spy), 10)

    def testUpdateFeature(self):
        p = QgsProject()
        vectorFileInfo = QFileInfo(unitTestDataPath() + "/france_parts.shp")
        vector_layer = QgsVectorLayer(vectorFileInfo.filePath(), vectorFileInfo.completeBaseName(), "ogr")
        self.assertTrue(vector_layer.isValid())
        p.addMapLayer(vector_layer)

        l = QgsPrintLayout(p)
        atlas = l.atlas()
        atlas.setEnabled(True)
        atlas.setCoverageLayer(vector_layer)

        self.assertTrue(atlas.beginRender())
        self.assertTrue(atlas.first())
        self.assertEqual(atlas.currentFeatureNumber(), 0)
        self.assertEqual(l.context().feature()[4], 'Basse-Normandie')
        self.assertEqual(l.context().layer(), vector_layer)

        vector_layer.startEditing()
        self.assertTrue(vector_layer.changeAttributeValue(l.context().feature().id(), 4, 'Nah, Canberra mate!'))
        self.assertEqual(l.context().feature()[4], 'Basse-Normandie')
        l.atlas().refreshCurrentFeature()
        self.assertEqual(l.context().feature()[4], 'Nah, Canberra mate!')
        vector_layer.rollBack()

    def testFileName(self):
        p = QgsProject()
        vectorFileInfo = QFileInfo(unitTestDataPath() + "/france_parts.shp")
        vector_layer = QgsVectorLayer(vectorFileInfo.filePath(), vectorFileInfo.completeBaseName(), "ogr")
        self.assertTrue(vector_layer.isValid())
        p.addMapLayer(vector_layer)

        l = QgsPrintLayout(p)
        atlas = l.atlas()
        atlas.setEnabled(True)
        atlas.setCoverageLayer(vector_layer)
        atlas.setFilenameExpression("'output_' || \"NAME_1\"")

        self.assertTrue(atlas.beginRender())
        self.assertEqual(atlas.count(), 4)
        atlas.first()
        self.assertEqual(atlas.currentFilename(), 'output_Basse-Normandie')
        self.assertEqual(atlas.filePath('/tmp/output', 'png'), '/tmp/output/output_Basse-Normandie.png')
        self.assertEqual(atlas.filePath('/tmp/output', '.png'), '/tmp/output/output_Basse-Normandie.png')
        self.assertEqual(atlas.filePath('/tmp/output/', 'svg'), '/tmp/output/output_Basse-Normandie.svg')

        atlas.next()
        self.assertEqual(atlas.currentFilename(), 'output_Bretagne')
        self.assertEqual(atlas.filePath('/tmp/output', 'png'), '/tmp/output/output_Bretagne.png')
        atlas.next()
        self.assertEqual(atlas.currentFilename(), 'output_Pays de la Loire')
        self.assertEqual(atlas.filePath('/tmp/output', 'png'), '/tmp/output/output_Pays de la Loire.png')
        atlas.next()
        self.assertEqual(atlas.currentFilename(), 'output_Centre')
        self.assertEqual(atlas.filePath('/tmp/output', 'png'), '/tmp/output/output_Centre.png')

        # try changing expression, filename should be updated instantly
        atlas.setFilenameExpression("'export_' || \"NAME_1\"")
        self.assertEqual(atlas.currentFilename(), 'export_Centre')

        atlas.endRender()

    def testNameForPage(self):
        p = QgsProject()
        vectorFileInfo = QFileInfo(unitTestDataPath() + "/france_parts.shp")
        vector_layer = QgsVectorLayer(vectorFileInfo.filePath(), vectorFileInfo.completeBaseName(), "ogr")
        self.assertTrue(vector_layer.isValid())
        p.addMapLayer(vector_layer)

        l = QgsPrintLayout(p)
        atlas = l.atlas()
        atlas.setEnabled(True)
        atlas.setCoverageLayer(vector_layer)
        atlas.setPageNameExpression("\"NAME_1\"")

        self.assertTrue(atlas.beginRender())
        self.assertEqual(atlas.nameForPage(0), 'Basse-Normandie')
        self.assertEqual(atlas.nameForPage(1), 'Bretagne')
        self.assertEqual(atlas.nameForPage(2), 'Pays de la Loire')
        self.assertEqual(atlas.nameForPage(3), 'Centre')


if __name__ == '__main__':
    unittest.main()
