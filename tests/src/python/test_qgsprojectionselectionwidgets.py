# -*- coding: utf-8 -*-
"""QGIS Unit tests for various projection selection widgets.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '12/11/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'

import qgis  # NOQA

from qgis.PyQt.QtTest import QSignalSpy
from qgis.gui import (QgsProjectionSelectionWidget,
                      QgsProjectionSelectionTreeWidget,
                      QgsProjectionSelectionDialog)
from qgis.core import QgsCoordinateReferenceSystem, QgsProject, QgsProjUtils
from qgis.PyQt.QtWidgets import QComboBox
from qgis.testing import start_app, unittest


start_app()


class TestQgsProjectionSelectionWidgets(unittest.TestCase):

    def testShowingHiding(self):
        """ test showing and hiding options """
        w = QgsProjectionSelectionWidget()

        # layer crs
        w.setOptionVisible(QgsProjectionSelectionWidget.LayerCrs, False)
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.LayerCrs))
        w.setOptionVisible(QgsProjectionSelectionWidget.LayerCrs, True)
        # should still be hidden, because layer crs not set
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.LayerCrs))
        w.setLayerCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        self.assertTrue(w.optionVisible(QgsProjectionSelectionWidget.LayerCrs))
        w.setOptionVisible(QgsProjectionSelectionWidget.LayerCrs, False)
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.LayerCrs))

        # project crs
        w.setOptionVisible(QgsProjectionSelectionWidget.ProjectCrs, False)
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.ProjectCrs))
        w.setOptionVisible(QgsProjectionSelectionWidget.ProjectCrs, True)
        # should still be hidden, because project crs was not set
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.ProjectCrs))
        QgsProject.instance().setCrs(QgsCoordinateReferenceSystem('EPSG:3113'))
        w = QgsProjectionSelectionWidget()
        w.setOptionVisible(QgsProjectionSelectionWidget.ProjectCrs, True)
        self.assertTrue(w.optionVisible(QgsProjectionSelectionWidget.ProjectCrs))
        w.setOptionVisible(QgsProjectionSelectionWidget.ProjectCrs, False)
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.ProjectCrs))

        # default crs
        w.setOptionVisible(QgsProjectionSelectionWidget.DefaultCrs, False)
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.DefaultCrs))
        w.setOptionVisible(QgsProjectionSelectionWidget.DefaultCrs, True)
        self.assertTrue(w.optionVisible(QgsProjectionSelectionWidget.DefaultCrs))

        # current crs
        w = QgsProjectionSelectionWidget()
        w.setOptionVisible(QgsProjectionSelectionWidget.CurrentCrs, False)
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.CurrentCrs))
        w.setOptionVisible(QgsProjectionSelectionWidget.CurrentCrs, True)
        self.assertTrue(w.optionVisible(QgsProjectionSelectionWidget.CurrentCrs))

        w = QgsProjectionSelectionWidget()
        w.setCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        w.setOptionVisible(QgsProjectionSelectionWidget.CurrentCrs, False)
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.CurrentCrs))
        w.setOptionVisible(QgsProjectionSelectionWidget.CurrentCrs, True)
        self.assertTrue(w.optionVisible(QgsProjectionSelectionWidget.CurrentCrs))

        # not set
        w = QgsProjectionSelectionWidget()
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsNotSet, False)
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.CrsNotSet))
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsNotSet, True)
        self.assertTrue(w.optionVisible(QgsProjectionSelectionWidget.CrsNotSet))
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsNotSet, False)
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.CrsNotSet))

    def testShowingNotSetOption(self):
        """ test showing the not set option """

        w = QgsProjectionSelectionWidget()
        # start with an invalid CRS
        w.setCrs(QgsCoordinateReferenceSystem())
        # add the not-set option
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsNotSet, True)
        self.assertTrue(w.optionVisible(QgsProjectionSelectionWidget.CrsNotSet))
        # current crs (which would show "invalid") should be hidden
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.CurrentCrs))
        # hide not-set option
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsNotSet, False)
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.CrsNotSet))
        # and now current crs option ('invalid') should be reshown
        self.assertTrue(w.optionVisible(QgsProjectionSelectionWidget.CurrentCrs))

        # repeat with a slightly different workflow
        w = QgsProjectionSelectionWidget()
        # start with an invalid CRS
        w.setCrs(QgsCoordinateReferenceSystem())
        # add the not-set option
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsNotSet, True)
        self.assertTrue(w.optionVisible(QgsProjectionSelectionWidget.CrsNotSet))
        # current crs (which would show "invalid") should be hidden
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.CurrentCrs))
        # now set a current crs
        w.setCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        # both current and not set options should be shown
        self.assertTrue(w.optionVisible(QgsProjectionSelectionWidget.CurrentCrs))
        self.assertTrue(w.optionVisible(QgsProjectionSelectionWidget.CrsNotSet))

    def testFilter(self):
        w = QgsProjectionSelectionWidget()

        w.setOptionVisible(QgsProjectionSelectionWidget.LayerCrs, True)
        w.setLayerCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        w.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        w.setOptionVisible(QgsProjectionSelectionWidget.CurrentCrs, True)
        QgsProject.instance().setCrs(QgsCoordinateReferenceSystem('EPSG:3113'))
        w.setOptionVisible(QgsProjectionSelectionWidget.ProjectCrs, True)

        self.assertIsInstance(w.children()[0], QComboBox)
        cb = w.children()[0]
        self.assertEqual(cb.count(), 4)
        self.assertEqual(cb.itemText(0), 'EPSG:4326 - WGS 84')
        self.assertEqual(cb.itemText(1), 'Default CRS: EPSG:4326 - WGS 84')
        self.assertEqual(cb.itemText(2), 'Layer CRS: EPSG:3111 - GDA94 / Vicgrid')
        self.assertEqual(cb.itemText(3), 'EPSG:3111 - GDA94 / Vicgrid')

        w.setFilter([QgsCoordinateReferenceSystem('EPSG:3111')])
        self.assertEqual(cb.count(), 2)
        self.assertEqual(cb.itemText(0), 'Layer CRS: EPSG:3111 - GDA94 / Vicgrid')
        self.assertEqual(cb.itemText(1), 'EPSG:3111 - GDA94 / Vicgrid')

        QgsProject.instance().setCrs(QgsCoordinateReferenceSystem())

    def testSignal(self):
        w = QgsProjectionSelectionWidget()
        w.show()
        spy = QSignalSpy(w.crsChanged)
        w.setCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        self.assertEqual(w.crs().authid(), 'EPSG:3111')
        self.assertEqual(len(spy), 1)
        # setting the same crs doesn't emit the signal
        w.setCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        self.assertEqual(len(spy), 1)

    def testTreeWidgetGettersSetters(self):
        """ basic tests for QgsProjectionSelectionTreeWidget """
        w = QgsProjectionSelectionTreeWidget()
        w.show()
        self.assertFalse(w.hasValidSelection())
        w.setCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        self.assertEqual(w.crs().authid(), 'EPSG:3111')
        self.assertTrue(w.hasValidSelection())

    def testTreeWidgetUnknownCrs(self):
        w = QgsProjectionSelectionTreeWidget()
        w.show()
        self.assertFalse(w.hasValidSelection())
        w.setCrs(QgsCoordinateReferenceSystem.fromWkt('GEOGCS["WGS 84",DATUM["unknown",SPHEROID["WGS84",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433]]'))
        self.assertTrue(w.crs().isValid())
        self.assertFalse(w.crs().authid())
        self.assertTrue(w.hasValidSelection())
        self.assertEqual(w.crs().toWkt(QgsCoordinateReferenceSystem.WKT2_2018), 'GEOGCRS["WGS 84",DATUM["unknown",ELLIPSOID["WGS84",6378137,298.257223563,LENGTHUNIT["metre",1,ID["EPSG",9001]]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],CS[ellipsoidal,2],AXIS["longitude",east,ORDER[1],ANGLEUNIT["degree",0.0174532925199433]],AXIS["latitude",north,ORDER[2],ANGLEUNIT["degree",0.0174532925199433]]]')

    def testTreeWidgetNotSetOption(self):
        """ test allowing no projection option for QgsProjectionSelectionTreeWidget """
        w = QgsProjectionSelectionTreeWidget()
        w.show()
        w.setShowNoProjection(True)
        self.assertTrue(w.showNoProjection())
        w.setShowNoProjection(False)
        self.assertFalse(w.showNoProjection())

        w.setShowNoProjection(True)
        # no projection should be a valid selection
        w.setCrs(QgsCoordinateReferenceSystem())
        self.assertTrue(w.hasValidSelection())
        self.assertFalse(w.crs().isValid())

    def testDialogGettersSetters(self):
        """ basic tests for QgsProjectionSelectionTreeWidget """
        w = QgsProjectionSelectionDialog()
        w.show()
        w.setCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        self.assertEqual(w.crs().authid(), 'EPSG:3111')

    def testDialogNotSetOption(self):
        """ test allowing no projection option for QgsProjectionSelectionTreeWidget """
        w = QgsProjectionSelectionDialog()
        w.show()
        w.setShowNoProjection(True)
        self.assertTrue(w.showNoProjection())
        w.setShowNoProjection(False)
        self.assertFalse(w.showNoProjection())

        w.setShowNoProjection(True)
        w.setCrs(QgsCoordinateReferenceSystem())
        self.assertFalse(w.crs().isValid())

    def testTreeWidgetDeferredLoad(self):
        """
        Test that crs setting made before widget is initialized is respected
        """
        w = QgsProjectionSelectionTreeWidget()
        spy = QSignalSpy(w.crsSelected)
        self.assertFalse(w.hasValidSelection())
        w.setCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        self.assertEqual(len(spy), 1)
        self.assertTrue(w.hasValidSelection())
        self.assertEqual(w.crs().authid(), 'EPSG:3111')
        self.assertEqual(len(spy), 1)

        w = QgsProjectionSelectionTreeWidget()
        spy = QSignalSpy(w.crsSelected)
        self.assertFalse(w.hasValidSelection())
        w.setCrs(QgsCoordinateReferenceSystem())
        self.assertEqual(len(spy), 1)
        self.assertTrue(w.hasValidSelection())
        self.assertFalse(w.crs().isValid())
        self.assertEqual(len(spy), 1)
        w.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        self.assertEqual(len(spy), 2)

        # expect same behavior if we show
        w = QgsProjectionSelectionTreeWidget()
        spy = QSignalSpy(w.crsSelected)
        self.assertFalse(w.hasValidSelection())
        w.setCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        self.assertEqual(len(spy), 1)
        w.show()
        self.assertTrue(w.hasValidSelection())
        self.assertEqual(w.crs().authid(), 'EPSG:3111')
        self.assertEqual(len(spy), 1)

        w = QgsProjectionSelectionTreeWidget()
        spy = QSignalSpy(w.crsSelected)
        self.assertFalse(w.hasValidSelection())
        w.setCrs(QgsCoordinateReferenceSystem())
        self.assertEqual(len(spy), 1)
        w.show()
        self.assertTrue(w.hasValidSelection())
        self.assertFalse(w.crs().isValid())
        self.assertEqual(len(spy), 1)

        # no double signals if same crs set
        w = QgsProjectionSelectionTreeWidget()
        spy = QSignalSpy(w.crsSelected)
        self.assertFalse(w.hasValidSelection())
        w.setCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        w.setCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        self.assertEqual(len(spy), 1)

        # no double signals if same crs set
        w = QgsProjectionSelectionTreeWidget()
        spy = QSignalSpy(w.crsSelected)
        self.assertFalse(w.hasValidSelection())
        w.setCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        w.setCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        w.show()
        w.setCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        self.assertEqual(len(spy), 1)
        w.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        self.assertEqual(len(spy), 2)


if __name__ == '__main__':
    unittest.main()
