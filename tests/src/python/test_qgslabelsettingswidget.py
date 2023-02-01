"""QGIS Unit tests for QgsLabelSettingsWidget and subclasses.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '04/02/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsPropertyCollection,
                       QgsPalLayerSettings,
                       QgsLabelObstacleSettings,
                       QgsProperty)
from qgis.gui import (QgsLabelSettingsWidgetBase,
                      QgsLabelObstacleSettingsWidget)

from qgis.testing import start_app, unittest
from qgis.PyQt.QtTest import QSignalSpy

start_app()


class TestQgsLabelSettingsWidget(unittest.TestCase):

    def testBase(self):
        """ test base class """
        w = QgsLabelSettingsWidgetBase()
        spy = QSignalSpy(w.changed)

        props = QgsPropertyCollection()
        props.setProperty(QgsPalLayerSettings.ObstacleFactor, QgsProperty.fromValue(5))
        props.setProperty(QgsPalLayerSettings.IsObstacle, QgsProperty.fromValue(True))
        w.setDataDefinedProperties(props)
        self.assertEqual(len(spy), 0)
        dd_props = w.dataDefinedProperties()
        prop = dd_props.property(QgsPalLayerSettings.ObstacleFactor)
        self.assertEqual(prop.asExpression(), '5')
        prop = dd_props.property(QgsPalLayerSettings.IsObstacle)
        self.assertEqual(prop.asExpression(), 'TRUE')

    def testObstacles(self):
        w = QgsLabelObstacleSettingsWidget()
        settings = QgsLabelObstacleSettings()
        settings.setFactor(0.4)
        settings.setType(QgsLabelObstacleSettings.PolygonBoundary)
        spy = QSignalSpy(w.changed)
        w.setSettings(settings)
        self.assertEqual(len(spy), 0)
        settings = w.settings()
        self.assertEqual(settings.factor(), 0.4)
        self.assertEqual(settings.type(), QgsLabelObstacleSettings.PolygonBoundary)
        settings.setFactor(1.2)
        settings.setType(QgsLabelObstacleSettings.PolygonInterior)
        w.setSettings(settings)
        self.assertEqual(len(spy), 0)
        settings = w.settings()
        self.assertEqual(settings.factor(), 1.2)
        self.assertEqual(settings.type(), QgsLabelObstacleSettings.PolygonInterior)

        props = QgsPropertyCollection()
        props.setProperty(QgsPalLayerSettings.ObstacleFactor, QgsProperty.fromValue(5))
        w.setDataDefinedProperties(props)

        props = QgsPropertyCollection()
        self.assertFalse(props.isActive(QgsPalLayerSettings.ObstacleFactor))
        w.updateDataDefinedProperties(props)
        self.assertTrue(props.isActive(QgsPalLayerSettings.ObstacleFactor))
        props = w.dataDefinedProperties()
        prop = props.property(QgsPalLayerSettings.ObstacleFactor)
        self.assertEqual(prop.asExpression(), '5')


if __name__ == '__main__':
    unittest.main()
