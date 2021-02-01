# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsVectorLayer.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Tim Sutton'
__date__ = '20/08/2012'
__copyright__ = 'Copyright 2012, The QGIS Project'

import qgis  # NOQA

import os
import tempfile
import shutil

from qgis.PyQt.QtCore import QDate, QDateTime, QVariant, Qt, QDateTime, QDate, QTime
from qgis.PyQt.QtGui import QPainter, QColor
from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (QgsWkbTypes,
                       QgsAction,
                       QgsCoordinateTransformContext,
                       QgsDataProvider,
                       QgsDefaultValue,
                       QgsEditorWidgetSetup,
                       QgsVectorLayer,
                       QgsRectangle,
                       QgsFeature,
                       QgsFeatureRequest,
                       QgsGeometry,
                       QgsPointXY,
                       QgsField,
                       QgsFieldConstraints,
                       QgsFields,
                       QgsVectorLayerJoinInfo,
                       QgsSymbol,
                       QgsSingleSymbolRenderer,
                       QgsCoordinateReferenceSystem,
                       QgsVectorLayerCache,
                       QgsReadWriteContext,
                       QgsProject,
                       QgsUnitTypes,
                       QgsAggregateCalculator,
                       QgsPoint,
                       QgsExpressionContext,
                       QgsExpressionContextScope,
                       QgsExpressionContextUtils,
                       QgsLineSymbol,
                       QgsMapLayerStyle,
                       QgsMapLayerDependency,
                       QgsPalLayerSettings,
                       QgsVectorLayerSimpleLabeling,
                       QgsSingleCategoryDiagramRenderer,
                       QgsDiagramLayerSettings,
                       QgsTextFormat,
                       QgsVectorLayerSelectedFeatureSource,
                       QgsExpression,
                       NULL)
from qgis.gui import (QgsAttributeTableModel,
                      QgsGui
                      )
from qgis.PyQt.QtTest import QSignalSpy
from qgis.testing import start_app, unittest
from featuresourcetestbase import FeatureSourceTestCase
from utilities import unitTestDataPath

TEST_DATA_DIR = unitTestDataPath()

start_app()


class TestQgsVectorLayerShapefile(unittest.TestCase, FeatureSourceTestCase):

    """
    Tests a vector layer against the feature source tests, using a real layer source (not a memory layer)
    """
    @classmethod
    def getSource(cls):
        vl = QgsVectorLayer(os.path.join(TEST_DATA_DIR, 'provider', 'shapefile.shp'), 'test')
        assert (vl.isValid())
        return vl

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        QgsGui.editorWidgetRegistry().initEditors()
        # Create test layer for FeatureSourceTestCase
        cls.source = cls.getSource()

    def treat_time_as_string(self):
        return True

    def treat_datetime_as_string(self):
        return True


if __name__ == '__main__':
    unittest.main()
