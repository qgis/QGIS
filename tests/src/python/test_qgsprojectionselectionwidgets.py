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
from qgis.core import QgsCoordinateReferenceSystem, QgsProject
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


if __name__ == '__main__':
    unittest.main()
