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

    def test


if __name__ == '__main__':
    unittest.main()
