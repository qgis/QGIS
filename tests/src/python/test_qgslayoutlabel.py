"""QGIS Unit tests for QgsLayoutItemLabel.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "(C) 2017 by Nyall Dawson"
__date__ = "23/10/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

from qgis.PyQt.QtCore import QDate, QDateTime, QFileInfo
from qgis.PyQt.QtTest import QSignalSpy
from qgis.core import (
    QgsLayoutItemLabel,
    QgsLayoutItemPage,
    QgsLayoutPoint,
    QgsPrintLayout,
    QgsProject,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from test_qgslayoutitem import LayoutItemTestCase
from utilities import unitTestDataPath

start_app()


class TestQgsLayoutItemLabel(QgisTestCase, LayoutItemTestCase):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        cls.item_class = QgsLayoutItemLabel

    def testCase(self):
        TEST_DATA_DIR = unitTestDataPath()
        vectorFileInfo = QFileInfo(TEST_DATA_DIR + "/france_parts.shp")
        mVectorLayer = QgsVectorLayer(
            vectorFileInfo.filePath(), vectorFileInfo.completeBaseName(), "ogr"
        )

        QgsProject.instance().addMapLayers([mVectorLayer])

        layout = QgsPrintLayout(QgsProject.instance())
        layout.initializeDefaults()

        label = QgsLayoutItemLabel(layout)
        layout.addLayoutItem(label)

        self.evaluation_test(layout, label)
        self.feature_evaluation_test(layout, label, mVectorLayer)
        self.page_evaluation_test(layout, label, mVectorLayer)

    def evaluation_test(self, layout, label):
        # $CURRENT_DATE evaluation
        label.setText("__$CURRENT_DATE__")
        self.assertEqual(
            label.currentText(), ("__" + QDate.currentDate().toString() + "__")
        )

        # $CURRENT_DATE() evaluation
        label.setText("__$CURRENT_DATE(dd)(ok)__")
        expected = "__" + QDateTime.currentDateTime().toString("dd") + "(ok)__"
        self.assertEqual(label.currentText(), expected)

        # $CURRENT_DATE() evaluation (inside an expression)
        label.setText("__[%$CURRENT_DATE(dd) + 1%](ok)__")
        dd = QDate.currentDate().day()
        expected = "__%d(ok)__" % (dd + 1)
        self.assertEqual(label.currentText(), expected)

        # expression evaluation (without associated feature)
        label.setText("__[%try(\"NAME_1\", '[NAME_1]')%][%21*2%]__")
        self.assertEqual(label.currentText(), "__[NAME_1]42__")

    def feature_evaluation_test(self, layout, label, mVectorLayer):
        atlas = layout.atlas()
        atlas.setCoverageLayer(mVectorLayer)
        atlas.setEnabled(True)

        label.setText("[%\"NAME_1\"||'_ok'%]")
        atlas.beginRender()
        atlas.seekTo(0)
        self.assertEqual(label.currentText(), "Basse-Normandie_ok")

        atlas.seekTo(1)
        self.assertEqual(label.currentText(), "Bretagne_ok")

    def page_evaluation_test(self, layout, label, mVectorLayer):
        page = QgsLayoutItemPage(layout)
        page.setPageSize("A4")
        layout.pageCollection().addPage(page)
        label.setText("[%@layout_page||'/'||@layout_numpages%]")
        self.assertEqual(label.currentText(), "1/2")

        # move the the second page and re-evaluate
        label.attemptMove(QgsLayoutPoint(0, 320))
        self.assertEqual(label.currentText(), "2/2")

    def test_convert_to_static(self):
        layout = QgsPrintLayout(QgsProject.instance())
        layout.initializeDefaults()

        label = QgsLayoutItemLabel(layout)
        layout.addLayoutItem(label)
        layout.setName("my layout")

        # no text yet
        label.convertToStaticText()
        self.assertFalse(label.text())

        spy = QSignalSpy(label.changed)
        label.setText("already static text")
        self.assertEqual(len(spy), 1)
        label.convertToStaticText()
        self.assertEqual(label.text(), "already static text")
        self.assertEqual(len(spy), 1)

        label.setText("with [% 1+2+3 %] some [% @layout_name %] dynamic bits")
        self.assertEqual(len(spy), 2)
        self.assertEqual(
            label.text(), "with [% 1+2+3 %] some [% @layout_name %] dynamic bits"
        )

        label.convertToStaticText()
        self.assertEqual(label.text(), "with 6 some my layout dynamic bits")
        self.assertEqual(len(spy), 3)


if __name__ == "__main__":
    unittest.main()
