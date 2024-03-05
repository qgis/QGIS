"""QGIS Unit tests for various projection selection widgets.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '12/11/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'

from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtWidgets import QComboBox
from qgis.core import (
    QgsApplication,
    QgsSettings,
    QgsCoordinateReferenceSystem,
    QgsProject
)
from qgis.gui import (
    QgsProjectionSelectionDialog,
    QgsProjectionSelectionTreeWidget,
    QgsProjectionSelectionWidget,
    QgsCoordinateReferenceSystemProxyModel
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsProjectionSelectionWidgets(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()

        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain(cls.__name__)
        QCoreApplication.setApplicationName(cls.__name__)
        QgsSettings().clear()
        start_app()

        QgsSettings().setValue("/projections/defaultProjectCrs", "EPSG:4326")

    def testShowingHiding(self):
        """ test showing and hiding options """
        QgsProject.instance().setCrs(QgsCoordinateReferenceSystem())
        w = QgsProjectionSelectionWidget()

        # layer crs
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.LayerCrs, False)
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.CrsOption.LayerCrs))
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.LayerCrs, True)
        # should still be hidden, because layer crs not set
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.CrsOption.LayerCrs))
        w.setLayerCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        self.assertTrue(w.optionVisible(QgsProjectionSelectionWidget.CrsOption.LayerCrs))
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.LayerCrs, False)
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.CrsOption.LayerCrs))

        # project crs
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.ProjectCrs, False)
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.CrsOption.ProjectCrs))
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.ProjectCrs, True)
        # should still be hidden, because project crs was not set
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.CrsOption.ProjectCrs))
        QgsProject.instance().setCrs(QgsCoordinateReferenceSystem('EPSG:3113'))
        w = QgsProjectionSelectionWidget()
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.ProjectCrs, True)
        self.assertTrue(w.optionVisible(QgsProjectionSelectionWidget.CrsOption.ProjectCrs))
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.ProjectCrs, False)
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.CrsOption.ProjectCrs))

        # default crs
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.DefaultCrs, False)
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.CrsOption.DefaultCrs))
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.DefaultCrs, True)
        self.assertTrue(w.optionVisible(QgsProjectionSelectionWidget.CrsOption.DefaultCrs))

        # current crs
        w = QgsProjectionSelectionWidget()
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.CurrentCrs, False)
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.CrsOption.CurrentCrs))
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.CurrentCrs, True)
        self.assertTrue(w.optionVisible(QgsProjectionSelectionWidget.CrsOption.CurrentCrs))

        w = QgsProjectionSelectionWidget()
        w.setCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.CurrentCrs, False)
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.CrsOption.CurrentCrs))
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.CurrentCrs, True)
        self.assertTrue(w.optionVisible(QgsProjectionSelectionWidget.CrsOption.CurrentCrs))

        # not set
        w = QgsProjectionSelectionWidget()
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.CrsNotSet, False)
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.CrsOption.CrsNotSet))
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.CrsNotSet, True)
        self.assertTrue(w.optionVisible(QgsProjectionSelectionWidget.CrsOption.CrsNotSet))
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.CrsNotSet, False)
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.CrsOption.CrsNotSet))

    def testShowingNotSetOption(self):
        """ test showing the not set option """
        w = QgsProjectionSelectionWidget()
        # start with an invalid CRS
        w.setCrs(QgsCoordinateReferenceSystem())
        # add the not-set option
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.CrsNotSet, True)
        self.assertTrue(w.optionVisible(QgsProjectionSelectionWidget.CrsOption.CrsNotSet))
        # current crs (which would show "invalid") should be hidden
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.CrsOption.CurrentCrs))
        # hide not-set option
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.CrsNotSet, False)
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.CrsOption.CrsNotSet))
        # and now current crs option ('invalid') should be reshown
        self.assertTrue(w.optionVisible(QgsProjectionSelectionWidget.CrsOption.CurrentCrs))

        # repeat with a slightly different workflow
        w = QgsProjectionSelectionWidget()
        # start with an invalid CRS
        w.setCrs(QgsCoordinateReferenceSystem())
        # add the not-set option
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.CrsNotSet, True)
        self.assertTrue(w.optionVisible(QgsProjectionSelectionWidget.CrsOption.CrsNotSet))
        # current crs (which would show "invalid") should be hidden
        self.assertFalse(w.optionVisible(QgsProjectionSelectionWidget.CrsOption.CurrentCrs))
        # now set a current crs
        w.setCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        # both current and not set options should be shown
        self.assertTrue(w.optionVisible(QgsProjectionSelectionWidget.CrsOption.CurrentCrs))
        self.assertTrue(w.optionVisible(QgsProjectionSelectionWidget.CrsOption.CrsNotSet))

    def testRecent(self):
        registry = QgsApplication.coordinateReferenceSystemRegistry()
        registry.clearRecent()
        QgsProject.instance().setCrs(QgsCoordinateReferenceSystem('EPSG:3113'))
        w = QgsProjectionSelectionWidget()
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.LayerCrs, True)
        w.setLayerCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        w.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.CurrentCrs, True)
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.ProjectCrs, True)

        self.assertIsInstance(w.children()[0], QComboBox)
        cb = w.children()[0]
        self.assertEqual(cb.count(), 4)
        self.assertEqual(cb.itemText(0), 'EPSG:4326 - WGS 84')
        self.assertEqual(cb.itemText(1), 'Project CRS: EPSG:3113 - GDA94 / BCSG02')
        self.assertEqual(cb.itemText(2), 'Default CRS: EPSG:4326 - WGS 84')
        self.assertEqual(cb.itemText(3), 'Layer CRS: EPSG:3111 - GDA94 / Vicgrid')

        # push some recent crs
        registry.pushRecent(QgsCoordinateReferenceSystem('EPSG:3857'))
        self.assertEqual(cb.count(), 5)
        self.assertEqual(cb.itemText(0), 'EPSG:4326 - WGS 84')
        self.assertEqual(cb.itemText(1), 'Project CRS: EPSG:3113 - GDA94 / BCSG02')
        self.assertEqual(cb.itemText(2), 'Default CRS: EPSG:4326 - WGS 84')
        self.assertEqual(cb.itemText(3), 'Layer CRS: EPSG:3111 - GDA94 / Vicgrid')
        self.assertEqual(cb.itemText(4), 'EPSG:3857 - WGS 84 / Pseudo-Mercator')

        registry.pushRecent(QgsCoordinateReferenceSystem('EPSG:28356'))
        self.assertEqual(cb.count(), 6)
        self.assertEqual(cb.itemText(0), 'EPSG:4326 - WGS 84')
        self.assertEqual(cb.itemText(1), 'Project CRS: EPSG:3113 - GDA94 / BCSG02')
        self.assertEqual(cb.itemText(2), 'Default CRS: EPSG:4326 - WGS 84')
        self.assertEqual(cb.itemText(3), 'Layer CRS: EPSG:3111 - GDA94 / Vicgrid')
        self.assertEqual(cb.itemText(4), 'EPSG:28356 - GDA94 / MGA zone 56')
        self.assertEqual(cb.itemText(5), 'EPSG:3857 - WGS 84 / Pseudo-Mercator')

        # push a recent CRS which is already in the list (same as project crs)
        # this should not be shown twice
        registry.pushRecent(QgsCoordinateReferenceSystem('EPSG:3111'))
        self.assertEqual(cb.count(), 6)
        self.assertEqual(cb.itemText(0), 'EPSG:4326 - WGS 84')
        self.assertEqual(cb.itemText(1), 'Project CRS: EPSG:3113 - GDA94 / BCSG02')
        self.assertEqual(cb.itemText(2), 'Default CRS: EPSG:4326 - WGS 84')
        self.assertEqual(cb.itemText(3), 'Layer CRS: EPSG:3111 - GDA94 / Vicgrid')
        self.assertEqual(cb.itemText(4), 'EPSG:28356 - GDA94 / MGA zone 56')
        self.assertEqual(cb.itemText(5), 'EPSG:3857 - WGS 84 / Pseudo-Mercator')

        registry.removeRecent(QgsCoordinateReferenceSystem('EPSG:3857'))
        self.assertEqual(cb.count(), 5)
        self.assertEqual(cb.itemText(0), 'EPSG:4326 - WGS 84')
        self.assertEqual(cb.itemText(1), 'Project CRS: EPSG:3113 - GDA94 / BCSG02')
        self.assertEqual(cb.itemText(2), 'Default CRS: EPSG:4326 - WGS 84')
        self.assertEqual(cb.itemText(3), 'Layer CRS: EPSG:3111 - GDA94 / Vicgrid')
        self.assertEqual(cb.itemText(4), 'EPSG:28356 - GDA94 / MGA zone 56')

        spy = QSignalSpy(w.crsChanged)
        cb.setCurrentIndex(1)
        self.assertEqual(len(spy), 1)
        self.assertEqual(spy[-1][0], QgsCoordinateReferenceSystem('EPSG:3113'))
        cb.setCurrentIndex(0)
        self.assertEqual(len(spy), 2)
        self.assertEqual(spy[-1][0], QgsCoordinateReferenceSystem('EPSG:4326'))
        cb.setCurrentIndex(2)
        self.assertEqual(len(spy), 3)
        self.assertEqual(spy[-1][0], QgsCoordinateReferenceSystem('EPSG:4326'))
        cb.setCurrentIndex(3)
        self.assertEqual(len(spy), 4)
        self.assertEqual(spy[-1][0], QgsCoordinateReferenceSystem('EPSG:3111'))
        cb.setCurrentIndex(4)
        self.assertEqual(len(spy), 5)
        self.assertEqual(spy[-1][0], QgsCoordinateReferenceSystem('EPSG:28356'))

    def testFilters(self):
        registry = QgsApplication.coordinateReferenceSystemRegistry()
        registry.clearRecent()
        # a horizontal crs
        registry.pushRecent(QgsCoordinateReferenceSystem('EPSG:28356'))
        # a vertical crs
        registry.pushRecent(QgsCoordinateReferenceSystem('ESRI:115866'))

        QgsProject.instance().setCrs(
            QgsCoordinateReferenceSystem('EPSG:3113'))
        w = QgsProjectionSelectionWidget()

        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.LayerCrs, True)
        # some vertical crses
        w.setLayerCrs(QgsCoordinateReferenceSystem('ESRI:115851'))
        w.setCrs(QgsCoordinateReferenceSystem('ESRI:115852'))
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.CurrentCrs, True)
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.ProjectCrs, True)

        # by default vertical won't be shown
        self.assertIsInstance(w.children()[0], QComboBox)
        cb = w.children()[0]
        self.assertEqual(cb.count(), 3)
        self.assertEqual(cb.itemText(0),
                         'Project CRS: EPSG:3113 - GDA94 / BCSG02')
        self.assertEqual(cb.itemText(1), 'Default CRS: EPSG:4326 - WGS 84')
        self.assertEqual(cb.itemText(2),
                         'EPSG:28356 - GDA94 / MGA zone 56')

        # filter the combo to show horizontal and vertical
        w.setFilters(QgsCoordinateReferenceSystemProxyModel.Filters(
            QgsCoordinateReferenceSystemProxyModel.Filter.FilterHorizontal |
            QgsCoordinateReferenceSystemProxyModel.Filter.FilterVertical
        ))

        self.assertEqual(cb.count(), 6)
        self.assertEqual(cb.itemText(0), 'ESRI:115852 - SIRGAS-CON_DGF01P01')
        self.assertEqual(cb.itemText(1),
                         'Project CRS: EPSG:3113 - GDA94 / BCSG02')
        self.assertEqual(cb.itemText(2), 'Default CRS: EPSG:4326 - WGS 84')
        self.assertEqual(cb.itemText(3),
                         'Layer CRS: ESRI:115851 - SIRGAS-CON_DGF00P01')
        self.assertEqual(cb.itemText(4),
                         'ESRI:115866 - SIRGAS-CON_SIR17P01')
        self.assertEqual(cb.itemText(5),
                         'EPSG:28356 - GDA94 / MGA zone 56')

        # only vertical
        w.setFilters(QgsCoordinateReferenceSystemProxyModel.Filters(
            QgsCoordinateReferenceSystemProxyModel.Filter.FilterVertical
        ))

        self.assertEqual(cb.count(), 3)
        self.assertEqual(cb.itemText(0), 'ESRI:115852 - SIRGAS-CON_DGF01P01')
        self.assertEqual(cb.itemText(1),
                         'Layer CRS: ESRI:115851 - SIRGAS-CON_DGF00P01')
        self.assertEqual(cb.itemText(2),
                         'ESRI:115866 - SIRGAS-CON_SIR17P01')

    def testFilteredCrs(self):
        registry = QgsApplication.coordinateReferenceSystemRegistry()
        registry.clearRecent()
        QgsProject.instance().setCrs(QgsCoordinateReferenceSystem('EPSG:3113'))
        w = QgsProjectionSelectionWidget()

        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.LayerCrs, True)
        w.setLayerCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        w.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.CurrentCrs, True)
        w.setOptionVisible(QgsProjectionSelectionWidget.CrsOption.ProjectCrs, True)

        self.assertIsInstance(w.children()[0], QComboBox)
        cb = w.children()[0]
        self.assertEqual(cb.count(), 4)
        self.assertEqual(cb.itemText(0), 'EPSG:4326 - WGS 84')
        self.assertEqual(cb.itemText(1), 'Project CRS: EPSG:3113 - GDA94 / BCSG02')
        self.assertEqual(cb.itemText(2), 'Default CRS: EPSG:4326 - WGS 84')
        self.assertEqual(cb.itemText(3), 'Layer CRS: EPSG:3111 - GDA94 / Vicgrid')

        w.setFilter([QgsCoordinateReferenceSystem('EPSG:3111'), QgsCoordinateReferenceSystem('EPSG:3113')])
        self.assertEqual(cb.count(), 2)
        self.assertEqual(cb.itemText(0), 'Project CRS: EPSG:3113 - GDA94 / BCSG02')
        self.assertEqual(cb.itemText(1), 'Layer CRS: EPSG:3111 - GDA94 / Vicgrid')

        w.setFilter([])
        self.assertEqual(cb.count(), 4)
        self.assertEqual(cb.itemText(0), 'EPSG:4326 - WGS 84')
        self.assertEqual(cb.itemText(1), 'Project CRS: EPSG:3113 - GDA94 / BCSG02')
        self.assertEqual(cb.itemText(2), 'Default CRS: EPSG:4326 - WGS 84')
        self.assertEqual(cb.itemText(3), 'Layer CRS: EPSG:3111 - GDA94 / Vicgrid')

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
        self.assertFalse(w.hasValidSelection())
        w.setCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        self.assertEqual(w.crs().authid(), 'EPSG:3111')
        self.assertTrue(w.hasValidSelection())

    def testTreeWidgetUnknownCrs(self):
        w = QgsProjectionSelectionTreeWidget()
        self.assertFalse(w.hasValidSelection())
        crs = QgsCoordinateReferenceSystem.fromWkt('GEOGCS["WGS 84",DATUM["unknown",SPHEROID["WGS84",6378137,298.257223563]],PRIMEM["Greenwich",0],UNIT["degree",0.0174532925199433]]')
        self.assertTrue(crs.isValid())
        w.setCrs(crs)
        self.assertTrue(w.crs().isValid())
        self.assertFalse(w.crs().authid())
        self.assertTrue(w.hasValidSelection())
        self.assertEqual(w.crs().toWkt(QgsCoordinateReferenceSystem.WktVariant.WKT2_2018), 'GEOGCRS["WGS 84",DATUM["unknown",ELLIPSOID["WGS84",6378137,298.257223563,LENGTHUNIT["metre",1,ID["EPSG",9001]]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],CS[ellipsoidal,2],AXIS["longitude",east,ORDER[1],ANGLEUNIT["degree",0.0174532925199433]],AXIS["latitude",north,ORDER[2],ANGLEUNIT["degree",0.0174532925199433]]]')

    def testTreeWidgetNotSetOption(self):
        """ test allowing no projection option for QgsProjectionSelectionTreeWidget """
        w = QgsProjectionSelectionTreeWidget()
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
        w.setCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        self.assertEqual(w.crs().authid(), 'EPSG:3111')

    def testDialogNotSetOption(self):
        """ test allowing no projection option for QgsProjectionSelectionTreeWidget """
        w = QgsProjectionSelectionDialog()
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
        w.setCrs(QgsCoordinateReferenceSystem('EPSG:3111'))
        self.assertEqual(len(spy), 1)
        w.setCrs(QgsCoordinateReferenceSystem('EPSG:4326'))
        self.assertEqual(len(spy), 2)


if __name__ == '__main__':
    unittest.main()
