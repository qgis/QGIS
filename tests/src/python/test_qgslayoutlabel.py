# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutItemLabel.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2017 by Nyall Dawson'
__date__ = '23/10/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA

from qgis.testing import start_app, unittest
from qgis.PyQt.QtCore import QFileInfo, QDate, QDateTime
from qgis.core import (QgsVectorLayer,
                       QgsPrintLayout,
                       QgsLayout,
                       QgsLayoutItemLabel,
                       QgsProject,
                       QgsLayoutItemPage,
                       QgsLayoutPoint)
from utilities import unitTestDataPath

from test_qgslayoutitem import LayoutItemTestCase

start_app()


class TestQgsLayoutItemLabel(unittest.TestCase, LayoutItemTestCase):

    @classmethod
    def setUpClass(cls):
        cls.item_class = QgsLayoutItemLabel

    def testCase(self):
        TEST_DATA_DIR = unitTestDataPath()
        vectorFileInfo = QFileInfo(TEST_DATA_DIR + "/france_parts.shp")
        mVectorLayer = QgsVectorLayer(vectorFileInfo.filePath(), vectorFileInfo.completeBaseName(), "ogr")

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
        assert label.currentText() == ("__" + QDate.currentDate().toString() + "__")

        # $CURRENT_DATE() evaluation
        label.setText("__$CURRENT_DATE(dd)(ok)__")
        expected = "__" + QDateTime.currentDateTime().toString("dd") + "(ok)__"
        assert label.currentText() == expected

        # $CURRENT_DATE() evaluation (inside an expression)
        label.setText("__[%$CURRENT_DATE(dd) + 1%](ok)__")
        dd = QDate.currentDate().day()
        expected = "__%d(ok)__" % (dd + 1)
        assert label.currentText() == expected

        # expression evaluation (without associated feature)
        label.setText("__[%\"NAME_1\"%][%21*2%]__")
        assert label.currentText() == "__[NAME_1]42__"

    def feature_evaluation_test(self, layout, label, mVectorLayer):
        atlas = layout.atlas()
        atlas.setCoverageLayer(mVectorLayer)
        atlas.setEnabled(True)

        label.setText("[%\"NAME_1\"||'_ok'%]")
        atlas.beginRender()
        atlas.seekTo(0)
        assert label.currentText() == "Basse-Normandie_ok"

        atlas.seekTo(1)
        assert label.currentText() == "Bretagne_ok"

    def page_evaluation_test(self, layout, label, mVectorLayer):
        page = QgsLayoutItemPage(layout)
        page.setPageSize('A4')
        layout.pageCollection().addPage(page)
        label.setText("[%@layout_page||'/'||@layout_numpages%]")
        assert label.currentText() == "1/2"

        # move the the second page and re-evaluate
        label.attemptMove(QgsLayoutPoint(0, 320))
        assert label.currentText() == "2/2"


if __name__ == '__main__':
    unittest.main()
