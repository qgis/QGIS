# -*- coding: utf-8 -*-
'''
test_qgscomposerlabel.py
                     --------------------------------------
               Date                 : Oct 2012
               Copyright            : (C) 2012 by Dr. Hugo Mercier
               email                : hugo dot mercier at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
'''
import qgis  # NOQA

from qgis.testing import start_app, unittest
from qgis.PyQt.QtCore import QFileInfo, QDate, QDateTime
from qgis.core import QgsVectorLayer, QgsMapLayerRegistry, QgsMapRenderer, QgsComposition, QgsComposerLabel, QgsFeatureRequest, QgsFeature, QgsExpression
from utilities import unitTestDataPath

start_app()


class TestQgsComposerLabel(unittest.TestCase):

    def testCase(self):
        TEST_DATA_DIR = unitTestDataPath()
        vectorFileInfo = QFileInfo(TEST_DATA_DIR + "/france_parts.shp")
        mVectorLayer = QgsVectorLayer(vectorFileInfo.filePath(), vectorFileInfo.completeBaseName(), "ogr")

        QgsMapLayerRegistry.instance().addMapLayers([mVectorLayer])

        # create composition with composer map
        mMapRenderer = QgsMapRenderer()
        layerStringList = []
        layerStringList.append(mVectorLayer.id())
        mMapRenderer.setLayerSet(layerStringList)
        mMapRenderer.setProjectionsEnabled(False)

        mComposition = QgsComposition(mMapRenderer)
        mComposition.setPaperSize(297, 210)

        mLabel = QgsComposerLabel(mComposition)
        mComposition.addComposerLabel(mLabel)

        self.evaluation_test(mComposition, mLabel)
        self.feature_evaluation_test(mComposition, mLabel, mVectorLayer)
        self.page_evaluation_test(mComposition, mLabel, mVectorLayer)

    def evaluation_test(self, mComposition, mLabel):
        # $CURRENT_DATE evaluation
        mLabel.setText("__$CURRENT_DATE__")
        assert mLabel.displayText() == ("__" + QDate.currentDate().toString() + "__")

        # $CURRENT_DATE() evaluation
        mLabel.setText("__$CURRENT_DATE(dd)(ok)__")
        expected = "__" + QDateTime.currentDateTime().toString("dd") + "(ok)__"
        assert mLabel.displayText() == expected

        # $CURRENT_DATE() evaluation (inside an expression)
        mLabel.setText("__[%$CURRENT_DATE(dd) + 1%](ok)__")
        dd = QDate.currentDate().day()
        expected = "__%d(ok)__" % (dd + 1)
        assert mLabel.displayText() == expected

        # expression evaluation (without associated feature)
        mLabel.setText("__[%\"NAME_1\"%][%21*2%]__")
        assert mLabel.displayText() == "__[NAME_1]42__"

    def feature_evaluation_test(self, mComposition, mLabel, mVectorLayer):
        provider = mVectorLayer.dataProvider()

        fi = provider.getFeatures(QgsFeatureRequest())
        feat = QgsFeature()

        fi.nextFeature(feat)
        mLabel.setExpressionContext(feat, mVectorLayer)
        mLabel.setText("[%\"NAME_1\"||'_ok'%]")
        assert mLabel.displayText() == "Basse-Normandie_ok"

        fi.nextFeature(feat)
        mLabel.setExpressionContext(feat, mVectorLayer)
        assert mLabel.displayText() == "Bretagne_ok"

        # evaluation with local variables
        locs = {"$test": "OK"}
        mLabel.setExpressionContext(feat, mVectorLayer, locs)
        mLabel.setText("[%\"NAME_1\"||$test%]")
        assert mLabel.displayText() == "BretagneOK"

    def page_evaluation_test(self, mComposition, mLabel, mVectorLayer):
        mComposition.setNumPages(2)
        mLabel.setText("[%$page||'/'||$numpages%]")
        assert mLabel.displayText() == "1/2"

        # move the the second page and re-evaluate
        mLabel.setItemPosition(0, 320)
        assert mLabel.displayText() == "2/2"

        # use setSpecialColumn
        mLabel.setText("[%$var1 + 1%]")
        QgsExpression.setSpecialColumn("$var1", 41)
        assert mLabel.displayText() == "42"
        QgsExpression.setSpecialColumn("$var1", 99)
        assert mLabel.displayText() == "100"
        QgsExpression.unsetSpecialColumn("$var1")
        assert mLabel.displayText() == "[%$var1 + 1%]"

if __name__ == '__main__':
    unittest.main()
