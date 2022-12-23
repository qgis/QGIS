# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsProjectStyleSettings.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Mathieu Pellerin'
__date__ = '09/05/2022'
__copyright__ = 'Copyright 2019, The QGIS Project'

import qgis  # NOQA
import os

from qgis.core import (QgsProject,
                       QgsProjectStyleSettings,
                       QgsProjectStyleDatabaseModel,
                       QgsProjectStyleDatabaseProxyModel,
                       QgsReadWriteContext,
                       QgsSymbol,
                       QgsWkbTypes,
                       QgsStyle,
                       QgsGradientColorRamp,
                       QgsTextFormat,
                       Qgis)

from qgis.PyQt.QtCore import (
    Qt,
    QModelIndex,
    QTemporaryDir,
    QCoreApplication,
    QEvent
)
from qgis.PyQt.QtGui import QFont, QColor

from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument, QDomElement
from qgis.testing import start_app, unittest
from utilities import (unitTestDataPath)

try:
    from qgis.core import QgsCombinedStyleModel
except ImportError:
    QgsCombinedStyleModel = None

app = start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsProjectViewSettings(unittest.TestCase):

    def testDefaultSymbol(self):
        p = QgsProjectStyleSettings()
        self.assertFalse(p.defaultSymbol(Qgis.SymbolType.Marker))
        self.assertFalse(p.defaultSymbol(Qgis.SymbolType.Line))
        self.assertFalse(p.defaultSymbol(Qgis.SymbolType.Fill))

        marker = QgsSymbol.defaultSymbol(QgsWkbTypes.PointGeometry)
        p.setDefaultSymbol(Qgis.SymbolType.Marker, marker)
        self.assertTrue(p.defaultSymbol(Qgis.SymbolType.Marker))

        line = QgsSymbol.defaultSymbol(QgsWkbTypes.LineGeometry)
        p.setDefaultSymbol(Qgis.SymbolType.Line, line)
        self.assertTrue(p.defaultSymbol(Qgis.SymbolType.Line))

        fill = QgsSymbol.defaultSymbol(QgsWkbTypes.PolygonGeometry)
        p.setDefaultSymbol(Qgis.SymbolType.Fill, fill)
        self.assertTrue(p.defaultSymbol(Qgis.SymbolType.Fill))

    def testDefaultColorRamp(self):
        p = QgsProjectStyleSettings()
        self.assertFalse(p.defaultColorRamp())

        ramp = QgsGradientColorRamp(QColor(255, 255, 255), QColor(255, 0, 0))
        p.setDefaultColorRamp(ramp)
        self.assertTrue(p.defaultColorRamp())

    def testDefaultTextFormat(self):
        p = QgsProjectStyleSettings()
        self.assertFalse(p.defaultTextFormat().isValid())

        textFormat = QgsTextFormat()
        textFormat.setFont(QFont())
        p.setDefaultTextFormat(textFormat)
        self.assertTrue(p.defaultTextFormat().isValid())

    def testRandomizeDefaultSymbolColor(self):
        p = QgsProjectStyleSettings()
        self.assertTrue(p.randomizeDefaultSymbolColor())
        p.setRandomizeDefaultSymbolColor(False)
        self.assertFalse(p.randomizeDefaultSymbolColor())

    def testDefaultSymbolOpacity(self):
        p = QgsProjectStyleSettings()
        self.assertEqual(p.defaultSymbolOpacity(), 1.0)
        p.setDefaultSymbolOpacity(0.25)
        self.assertEqual(p.defaultSymbolOpacity(), 0.25)

    def testProjectStyle(self):
        project = QgsProject()
        settings = project.styleSettings()
        self.assertIsInstance(settings.projectStyle(), QgsStyle)
        self.assertEqual(settings.projectStyle().name(), 'Project Styles')

        text_format = QgsTextFormat()
        text_format.setColor(QColor(255, 0, 0))
        self.assertTrue(settings.projectStyle().addTextFormat('my text format', text_format))
        self.assertTrue(settings.projectStyle().saveTextFormat('my text format', text_format, True, []))
        self.assertEqual(settings.projectStyle().textFormatCount(), 1)

        tmp_dir = QTemporaryDir()
        tmp_project_file = "{}/project.qgs".format(tmp_dir.path())
        self.assertTrue(project.write(tmp_project_file))

        project.deleteLater()
        del project
        QCoreApplication.sendPostedEvents(None, QEvent.DeferredDelete)

        project2 = QgsProject()
        self.assertTrue(project2.read(tmp_project_file))
        self.assertEqual(project2.styleSettings().projectStyle().textFormatCount(), 1)
        self.assertEqual(project2.styleSettings().projectStyle().textFormat('my text format').color().name(), '#ff0000')

        project2.clear()
        self.assertEqual(project2.styleSettings().projectStyle().textFormatCount(), 0)

    @unittest.skipIf(QgsCombinedStyleModel is None, "QgsCombinedStyleModel not available")
    def testStylePaths(self):
        p = QgsProjectStyleSettings()
        spy = QSignalSpy(p.styleDatabasesChanged)

        model = QgsProjectStyleDatabaseModel(p)
        model_with_default = QgsProjectStyleDatabaseModel(p)
        model_with_default.setShowDefaultStyle(True)
        proxy_model = QgsProjectStyleDatabaseProxyModel(model_with_default)
        proxy_model.setFilters(QgsProjectStyleDatabaseProxyModel.Filter.FilterHideReadOnly)

        project_style = QgsStyle()
        project_style.setName('project')
        model_with_project_style = QgsProjectStyleDatabaseModel(p)
        model_with_project_style.setShowDefaultStyle(True)
        model_with_project_style.setProjectStyle(project_style)

        self.assertFalse(p.styleDatabasePaths())
        self.assertFalse(p.styles())
        self.assertEqual(p.combinedStyleModel().rowCount(), 0)
        self.assertEqual(model.rowCount(QModelIndex()), 0)
        self.assertFalse(model.data(model.index(0, 0, QModelIndex()), Qt.DisplayRole))
        self.assertEqual(model_with_default.rowCount(QModelIndex()), 1)
        self.assertEqual(model_with_default.data(model_with_default.index(0, 0, QModelIndex()), Qt.DisplayRole),
                         'Default')
        self.assertEqual(model_with_default.data(model_with_default.index(0, 0, QModelIndex()),
                                                 QgsProjectStyleDatabaseModel.StyleRole), QgsStyle.defaultStyle())

        self.assertEqual(model_with_project_style.rowCount(QModelIndex()), 2)
        self.assertEqual(
            model_with_project_style.data(model_with_project_style.index(0, 0, QModelIndex()), Qt.DisplayRole),
            'project')
        self.assertEqual(model_with_project_style.data(model_with_project_style.index(0, 0, QModelIndex()),
                                                       QgsProjectStyleDatabaseModel.StyleRole), project_style)
        self.assertEqual(
            model_with_project_style.data(model_with_project_style.index(1, 0, QModelIndex()), Qt.DisplayRole),
            'Default')
        self.assertEqual(model_with_project_style.data(model_with_project_style.index(1, 0, QModelIndex()),
                                                       QgsProjectStyleDatabaseModel.StyleRole), QgsStyle.defaultStyle())

        self.assertEqual(proxy_model.rowCount(QModelIndex()), 1)
        self.assertEqual(proxy_model.data(proxy_model.index(0, 0, QModelIndex()), Qt.DisplayRole), 'Default')
        self.assertEqual(
            proxy_model.data(proxy_model.index(0, 0, QModelIndex()), QgsProjectStyleDatabaseModel.StyleRole),
            QgsStyle.defaultStyle())

        p.addStyleDatabasePath(unitTestDataPath() + '/style1.db')
        self.assertEqual(len(spy), 1)
        self.assertEqual(p.styleDatabasePaths(), [unitTestDataPath() + '/style1.db'])
        self.assertEqual(p.combinedStyleModel().rowCount(), 1)
        self.assertEqual(p.combinedStyleModel().data(p.combinedStyleModel().index(0, 0)), 'style1')
        self.assertEqual(len(p.styles()), 1)
        self.assertEqual(p.styles()[0].fileName(), unitTestDataPath() + '/style1.db')
        self.assertEqual(p.styles()[0].name(), 'style1')
        self.assertEqual(model.rowCount(QModelIndex()), 1)
        self.assertEqual(model.data(model.index(0, 0, QModelIndex()), Qt.DisplayRole), 'style1')
        self.assertEqual(model.data(model.index(0, 0, QModelIndex()), QgsProjectStyleDatabaseModel.StyleRole),
                         p.styles()[0])
        self.assertEqual(model.data(model.index(0, 0, QModelIndex()), QgsProjectStyleDatabaseModel.PathRole),
                         unitTestDataPath() + '/style1.db')

        self.assertEqual(model_with_default.rowCount(QModelIndex()), 2)
        self.assertEqual(model_with_default.data(model_with_default.index(0, 0, QModelIndex()), Qt.DisplayRole),
                         'Default')
        self.assertEqual(model_with_default.data(model_with_default.index(0, 0, QModelIndex()),
                                                 QgsProjectStyleDatabaseModel.StyleRole), QgsStyle.defaultStyle())
        self.assertEqual(model_with_default.data(model_with_default.index(1, 0, QModelIndex()), Qt.DisplayRole),
                         'style1')
        self.assertEqual(model_with_default.data(model_with_default.index(1, 0, QModelIndex()),
                                                 QgsProjectStyleDatabaseModel.StyleRole), p.styles()[0])
        self.assertEqual(model_with_default.data(model_with_default.index(1, 0, QModelIndex()),
                                                 QgsProjectStyleDatabaseModel.PathRole),
                         unitTestDataPath() + '/style1.db')

        self.assertEqual(model_with_project_style.rowCount(QModelIndex()), 3)
        self.assertEqual(
            model_with_project_style.data(model_with_project_style.index(0, 0, QModelIndex()), Qt.DisplayRole),
            'project')
        self.assertEqual(model_with_project_style.data(model_with_project_style.index(0, 0, QModelIndex()),
                                                       QgsProjectStyleDatabaseModel.StyleRole), project_style)
        self.assertEqual(
            model_with_project_style.data(model_with_project_style.index(1, 0, QModelIndex()), Qt.DisplayRole),
            'Default')
        self.assertEqual(model_with_project_style.data(model_with_project_style.index(1, 0, QModelIndex()),
                                                       QgsProjectStyleDatabaseModel.StyleRole), QgsStyle.defaultStyle())
        self.assertEqual(
            model_with_project_style.data(model_with_project_style.index(2, 0, QModelIndex()), Qt.DisplayRole),
            'style1')
        self.assertEqual(model_with_project_style.data(model_with_project_style.index(2, 0, QModelIndex()),
                                                       QgsProjectStyleDatabaseModel.StyleRole), p.styles()[0])
        self.assertEqual(model_with_project_style.data(model_with_project_style.index(2, 0, QModelIndex()),
                                                       QgsProjectStyleDatabaseModel.PathRole),
                         unitTestDataPath() + '/style1.db')

        self.assertEqual(proxy_model.rowCount(QModelIndex()), 2)
        self.assertEqual(proxy_model.data(proxy_model.index(0, 0, QModelIndex()), Qt.DisplayRole), 'Default')
        self.assertEqual(
            proxy_model.data(proxy_model.index(0, 0, QModelIndex()), QgsProjectStyleDatabaseModel.StyleRole),
            QgsStyle.defaultStyle())
        self.assertEqual(proxy_model.data(proxy_model.index(1, 0, QModelIndex()), Qt.DisplayRole), 'style1')
        self.assertEqual(
            proxy_model.data(proxy_model.index(1, 0, QModelIndex()), QgsProjectStyleDatabaseModel.StyleRole),
            p.styles()[0])
        self.assertEqual(proxy_model.data(proxy_model.index(1, 0, QModelIndex()),
                                          QgsProjectStyleDatabaseModel.PathRole), unitTestDataPath() + '/style1.db')

        # try re-adding path which is already present
        p.addStyleDatabasePath(unitTestDataPath() + '/style1.db')
        self.assertEqual(len(spy), 1)
        self.assertEqual(p.styleDatabasePaths(), [unitTestDataPath() + '/style1.db'])
        self.assertEqual(p.combinedStyleModel().rowCount(), 1)
        self.assertEqual(p.combinedStyleModel().data(p.combinedStyleModel().index(0, 0)), 'style1')
        self.assertEqual(model.rowCount(QModelIndex()), 1)
        self.assertEqual(model.data(model.index(0, 0, QModelIndex()), Qt.DisplayRole), 'style1')
        self.assertEqual(model.data(model.index(0, 0, QModelIndex()), QgsProjectStyleDatabaseModel.StyleRole),
                         p.styles()[0])
        self.assertEqual(model_with_default.rowCount(QModelIndex()), 2)
        self.assertEqual(model_with_default.data(model_with_default.index(0, 0, QModelIndex()), Qt.DisplayRole),
                         'Default')
        self.assertEqual(model_with_default.data(model_with_default.index(0, 0, QModelIndex()),
                                                 QgsProjectStyleDatabaseModel.StyleRole), QgsStyle.defaultStyle())
        self.assertEqual(model_with_default.data(model_with_default.index(1, 0, QModelIndex()), Qt.DisplayRole),
                         'style1')
        self.assertEqual(model_with_default.data(model_with_default.index(1, 0, QModelIndex()),
                                                 QgsProjectStyleDatabaseModel.StyleRole), p.styles()[0])

        p.addStyleDatabasePath(unitTestDataPath() + '/style2.db')
        self.assertEqual(len(spy), 2)
        self.assertEqual(p.styleDatabasePaths(), [unitTestDataPath() + '/style1.db', unitTestDataPath() + '/style2.db'])
        self.assertEqual(p.styles()[0].fileName(), unitTestDataPath() + '/style1.db')
        self.assertEqual(p.styles()[0].name(), 'style1')
        self.assertEqual(p.styles()[1].fileName(), unitTestDataPath() + '/style2.db')
        self.assertEqual(p.styles()[1].name(), 'style2')
        self.assertEqual(p.combinedStyleModel().rowCount(), 2)
        self.assertEqual(p.combinedStyleModel().data(p.combinedStyleModel().index(0, 0)), 'style1')
        self.assertEqual(p.combinedStyleModel().data(p.combinedStyleModel().index(1, 0)), 'style2')
        self.assertEqual(model.rowCount(QModelIndex()), 2)
        self.assertEqual(model.data(model.index(0, 0, QModelIndex()), Qt.DisplayRole), 'style1')
        self.assertEqual(model.data(model.index(0, 0, QModelIndex()), QgsProjectStyleDatabaseModel.StyleRole),
                         p.styles()[0])
        self.assertEqual(model.data(model.index(1, 0, QModelIndex()), Qt.DisplayRole), 'style2')
        self.assertEqual(model.data(model.index(1, 0, QModelIndex()), QgsProjectStyleDatabaseModel.StyleRole),
                         p.styles()[1])
        self.assertEqual(model_with_default.rowCount(QModelIndex()), 3)
        self.assertEqual(model_with_default.data(model_with_default.index(0, 0, QModelIndex()), Qt.DisplayRole),
                         'Default')
        self.assertEqual(model_with_default.data(model_with_default.index(0, 0, QModelIndex()),
                                                 QgsProjectStyleDatabaseModel.StyleRole), QgsStyle.defaultStyle())
        self.assertEqual(model_with_default.data(model_with_default.index(1, 0, QModelIndex()), Qt.DisplayRole),
                         'style1')
        self.assertEqual(model_with_default.data(model_with_default.index(1, 0, QModelIndex()),
                                                 QgsProjectStyleDatabaseModel.StyleRole), p.styles()[0])
        self.assertEqual(model_with_default.data(model_with_default.index(2, 0, QModelIndex()), Qt.DisplayRole),
                         'style2')
        self.assertEqual(model_with_default.data(model_with_default.index(2, 0, QModelIndex()),
                                                 QgsProjectStyleDatabaseModel.StyleRole), p.styles()[1])

        self.assertEqual(model_with_project_style.rowCount(QModelIndex()), 4)
        self.assertEqual(
            model_with_project_style.data(model_with_project_style.index(0, 0, QModelIndex()), Qt.DisplayRole),
            'project')
        self.assertEqual(model_with_project_style.data(model_with_project_style.index(0, 0, QModelIndex()),
                                                       QgsProjectStyleDatabaseModel.StyleRole), project_style)
        self.assertEqual(
            model_with_project_style.data(model_with_project_style.index(1, 0, QModelIndex()), Qt.DisplayRole),
            'Default')
        self.assertEqual(model_with_project_style.data(model_with_project_style.index(1, 0, QModelIndex()),
                                                       QgsProjectStyleDatabaseModel.StyleRole), QgsStyle.defaultStyle())
        self.assertEqual(
            model_with_project_style.data(model_with_project_style.index(2, 0, QModelIndex()), Qt.DisplayRole),
            'style1')
        self.assertEqual(model_with_project_style.data(model_with_project_style.index(2, 0, QModelIndex()),
                                                       QgsProjectStyleDatabaseModel.StyleRole), p.styles()[0])
        self.assertEqual(
            model_with_project_style.data(model_with_project_style.index(3, 0, QModelIndex()), Qt.DisplayRole),
            'style2')
        self.assertEqual(model_with_project_style.data(model_with_project_style.index(3, 0, QModelIndex()),
                                                       QgsProjectStyleDatabaseModel.StyleRole), p.styles()[1])

        self.assertEqual(p.styleAtPath(unitTestDataPath() + '/style1.db'), p.styles()[0])
        self.assertEqual(p.styleAtPath(unitTestDataPath() + '/style2.db'), p.styles()[1])
        self.assertFalse(p.styleAtPath('.xxx'))

        p.setStyleDatabasePaths([unitTestDataPath() + '/style3.db'])
        self.assertEqual(len(spy), 3)
        self.assertEqual(p.styleDatabasePaths(), [unitTestDataPath() + '/style3.db'])

        self.assertEqual(p.combinedStyleModel().rowCount(), 1)
        self.assertEqual(p.combinedStyleModel().data(p.combinedStyleModel().index(0, 0)), 'style3')

        self.assertEqual(model.rowCount(QModelIndex()), 1)
        self.assertEqual(model.data(model.index(0, 0, QModelIndex()), Qt.DisplayRole), 'style3')
        self.assertEqual(model.data(model.index(0, 0, QModelIndex()), QgsProjectStyleDatabaseModel.StyleRole),
                         p.styles()[0])
        self.assertEqual(model_with_default.rowCount(QModelIndex()), 2)
        self.assertEqual(model_with_default.data(model_with_default.index(0, 0, QModelIndex()), Qt.DisplayRole),
                         'Default')
        self.assertEqual(model_with_default.data(model_with_default.index(0, 0, QModelIndex()),
                                                 QgsProjectStyleDatabaseModel.StyleRole), QgsStyle.defaultStyle())
        self.assertEqual(model_with_default.data(model_with_default.index(1, 0, QModelIndex()), Qt.DisplayRole),
                         'style3')
        self.assertEqual(model_with_default.data(model_with_default.index(1, 0, QModelIndex()),
                                                 QgsProjectStyleDatabaseModel.StyleRole), p.styles()[0])

        self.assertEqual(model_with_project_style.rowCount(QModelIndex()), 3)
        self.assertEqual(
            model_with_project_style.data(model_with_project_style.index(0, 0, QModelIndex()), Qt.DisplayRole),
            'project')
        self.assertEqual(model_with_project_style.data(model_with_project_style.index(0, 0, QModelIndex()),
                                                       QgsProjectStyleDatabaseModel.StyleRole), project_style)
        self.assertEqual(
            model_with_project_style.data(model_with_project_style.index(1, 0, QModelIndex()), Qt.DisplayRole),
            'Default')
        self.assertEqual(model_with_project_style.data(model_with_project_style.index(1, 0, QModelIndex()),
                                                       QgsProjectStyleDatabaseModel.StyleRole), QgsStyle.defaultStyle())
        self.assertEqual(
            model_with_project_style.data(model_with_project_style.index(2, 0, QModelIndex()), Qt.DisplayRole),
            'style3')
        self.assertEqual(model_with_project_style.data(model_with_project_style.index(2, 0, QModelIndex()),
                                                       QgsProjectStyleDatabaseModel.StyleRole), p.styles()[0])

        self.assertEqual(p.styles()[0].fileName(), unitTestDataPath() + '/style3.db')
        self.assertEqual(p.styles()[0].name(), 'style3')

        p.setStyleDatabasePaths([unitTestDataPath() + '/style3.db'])
        self.assertEqual(len(spy), 3)
        self.assertEqual(p.combinedStyleModel().rowCount(), 1)
        self.assertEqual(p.combinedStyleModel().data(p.combinedStyleModel().index(0, 0)), 'style3')

        self.assertEqual(model.rowCount(QModelIndex()), 1)
        self.assertEqual(model.data(model.index(0, 0, QModelIndex()), Qt.DisplayRole), 'style3')
        self.assertEqual(model.data(model.index(0, 0, QModelIndex()), QgsProjectStyleDatabaseModel.StyleRole),
                         p.styles()[0])
        self.assertEqual(model_with_default.rowCount(QModelIndex()), 2)
        self.assertEqual(model_with_default.data(model_with_default.index(0, 0, QModelIndex()), Qt.DisplayRole),
                         'Default')
        self.assertEqual(model_with_default.data(model_with_default.index(0, 0, QModelIndex()),
                                                 QgsProjectStyleDatabaseModel.StyleRole), QgsStyle.defaultStyle())
        self.assertEqual(model_with_default.data(model_with_default.index(1, 0, QModelIndex()), Qt.DisplayRole),
                         'style3')
        self.assertEqual(model_with_default.data(model_with_default.index(1, 0, QModelIndex()),
                                                 QgsProjectStyleDatabaseModel.StyleRole), p.styles()[0])

        p.setStyleDatabasePaths([])
        self.assertEqual(len(spy), 4)
        self.assertFalse(p.styleDatabasePaths())
        self.assertFalse(p.styles())
        self.assertEqual(p.combinedStyleModel().rowCount(), 0)

        self.assertEqual(model.rowCount(QModelIndex()), 0)
        self.assertEqual(model_with_default.rowCount(QModelIndex()), 1)
        self.assertEqual(model_with_default.data(model_with_default.index(0, 0, QModelIndex()), Qt.DisplayRole),
                         'Default')
        self.assertEqual(model_with_default.data(model_with_default.index(0, 0, QModelIndex()),
                                                 QgsProjectStyleDatabaseModel.StyleRole), QgsStyle.defaultStyle())

        # test using a .xml path
        p.addStyleDatabasePath(unitTestDataPath() + '/categorized.xml')
        self.assertEqual(p.styles()[0].fileName(), unitTestDataPath() + '/categorized.xml')
        self.assertEqual(p.combinedStyleModel().rowCount(), 4)
        self.assertEqual(p.combinedStyleModel().data(p.combinedStyleModel().index(0, 0)), 'categorized')
        self.assertEqual(p.combinedStyleModel().data(p.combinedStyleModel().index(1, 0)), ' ----c/- ')
        self.assertEqual(p.combinedStyleModel().data(p.combinedStyleModel().index(2, 0)), 'B ')
        self.assertEqual(p.combinedStyleModel().data(p.combinedStyleModel().index(3, 0)), 'a')

        self.assertEqual(model.rowCount(QModelIndex()), 1)
        self.assertEqual(model.data(model.index(0, 0, QModelIndex()), Qt.DisplayRole), 'categorized')
        self.assertEqual(model.data(model.index(0, 0, QModelIndex()), QgsProjectStyleDatabaseModel.StyleRole),
                         p.styles()[0])
        self.assertEqual(model_with_default.rowCount(QModelIndex()), 2)
        self.assertEqual(model_with_default.data(model_with_default.index(0, 0, QModelIndex()), Qt.DisplayRole),
                         'Default')
        self.assertEqual(model_with_default.data(model_with_default.index(0, 0, QModelIndex()),
                                                 QgsProjectStyleDatabaseModel.StyleRole), QgsStyle.defaultStyle())
        self.assertEqual(model_with_default.data(model_with_default.index(1, 0, QModelIndex()), Qt.DisplayRole),
                         'categorized')
        self.assertEqual(model_with_default.data(model_with_default.index(1, 0, QModelIndex()),
                                                 QgsProjectStyleDatabaseModel.StyleRole), p.styles()[0])
        # read only style should not be included
        self.assertEqual(proxy_model.rowCount(QModelIndex()), 1)
        self.assertEqual(proxy_model.data(proxy_model.index(0, 0, QModelIndex()), Qt.DisplayRole), 'Default')
        self.assertEqual(
            proxy_model.data(proxy_model.index(0, 0, QModelIndex()), QgsProjectStyleDatabaseModel.StyleRole),
            QgsStyle.defaultStyle())

    def testReadWrite(self):
        p = QgsProjectStyleSettings()

        line = QgsSymbol.defaultSymbol(QgsWkbTypes.LineGeometry)
        p.setDefaultSymbol(Qgis.SymbolType.Line, line)

        ramp = QgsGradientColorRamp(QColor(255, 255, 255), QColor(255, 0, 0))
        p.setDefaultColorRamp(ramp)

        textFormat = QgsTextFormat()
        textFormat.setFont(QFont())
        p.setDefaultTextFormat(textFormat)

        p.setRandomizeDefaultSymbolColor(False)
        p.setDefaultSymbolOpacity(0.25)

        p.setStyleDatabasePaths([unitTestDataPath() + '/style1.db', unitTestDataPath() + '/style2.db'])

        doc = QDomDocument("testdoc")
        elem = p.writeXml(doc, QgsReadWriteContext())

        p2 = QgsProjectStyleSettings()
        self.assertTrue(p2.readXml(elem, QgsReadWriteContext()))

        self.assertFalse(p2.defaultSymbol(Qgis.SymbolType.Marker))
        self.assertTrue(p2.defaultSymbol(Qgis.SymbolType.Line))
        self.assertFalse(p2.defaultSymbol(Qgis.SymbolType.Fill))
        self.assertTrue(p2.defaultColorRamp())
        self.assertTrue(p2.defaultTextFormat().isValid())
        self.assertFalse(p2.randomizeDefaultSymbolColor())
        self.assertEqual(p2.defaultSymbolOpacity(), 0.25)

        self.assertEqual(p2.styleDatabasePaths(),
                         [unitTestDataPath() + '/style1.db', unitTestDataPath() + '/style2.db'])


if __name__ == '__main__':
    unittest.main()
