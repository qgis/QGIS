"""QGIS Unit tests for 3d lights.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import unittest

from qgis._3d import (
    QgsDirectionalLightSettings,
    QgsLightSource,
    QgsPointLightSettings,
    QgsSunLightSettings,
)
from qgis.core import (
    Qgis,
    QgsReadWriteContext,
    QgsVector3D,
)
from qgis.PyQt.QtCore import QDateTime, Qt
from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtXml import QDomDocument
from qgis.testing import QgisTestCase, start_app

start_app()


class TestQgsDirectionalLightSettings(QgisTestCase):
    def test_getters_setters(self):
        settings = QgsDirectionalLightSettings()

        self.assertEqual(settings.type(), Qgis.LightSourceType.Directional)

        self.assertEqual(settings.direction(), QgsVector3D(-0.32, 0.27, -0.91))
        self.assertEqual(settings.color(), QColor(Qt.GlobalColor.white))
        self.assertEqual(settings.intensity(), 1.0)
        self.assertTrue(settings.id())

        settings.setDirection(QgsVector3D(1.0, 2.0, 3.0))
        self.assertEqual(settings.direction(), QgsVector3D(1.0, 2.0, 3.0))

        settings.setColor(QColor(255, 0, 0))
        self.assertEqual(settings.color(), QColor(255, 0, 0))

        settings.setIntensity(2.5)
        self.assertEqual(settings.intensity(), 2.5)

    def test_clone(self):
        settings = QgsDirectionalLightSettings()
        settings.setDirection(QgsVector3D(1.0, 2.0, 3.0))
        settings.setColor(QColor(255, 0, 0))
        settings.setIntensity(2.5)

        cloned = settings.clone()
        self.assertIsInstance(cloned, QgsDirectionalLightSettings)
        self.assertEqual(cloned.direction(), QgsVector3D(1.0, 2.0, 3.0))
        self.assertEqual(cloned.color(), QColor(255, 0, 0))
        self.assertEqual(cloned.intensity(), 2.5)
        self.assertEqual(cloned.id(), settings.id())

    def test_equality(self):
        settings1 = QgsDirectionalLightSettings()
        # start with a clone so we get equal ID
        settings2 = settings1.clone()

        self.assertEqual(settings1, settings2)

        settings2.setDirection(QgsVector3D(1.0, 2.0, 3.0))
        self.assertNotEqual(settings1, settings2)
        settings1.setDirection(QgsVector3D(1.0, 2.0, 3.0))
        self.assertEqual(settings1, settings2)

        settings2.setColor(QColor(255, 0, 0))
        self.assertNotEqual(settings1, settings2)
        settings1.setColor(QColor(255, 0, 0))
        self.assertEqual(settings1, settings2)

        settings2.setIntensity(2.5)
        self.assertNotEqual(settings1, settings2)
        settings1.setIntensity(2.5)
        self.assertEqual(settings1, settings2)

        settings2 = QgsDirectionalLightSettings()
        settings2.setDirection(QgsVector3D(1.0, 2.0, 3.0))
        settings2.setColor(QColor(255, 0, 0))
        settings2.setIntensity(2.5)
        # should be not equal, different ID
        self.assertNotEqual(settings1.id(), settings2.id())
        self.assertNotEqual(settings1, settings2)

    def test_xml_roundtrip(self):
        settings = QgsDirectionalLightSettings()
        settings.setDirection(QgsVector3D(1.0, 2.0, 3.0))
        settings.setColor(QColor(255, 0, 0))
        settings.setIntensity(2.5)

        doc = QDomDocument("settings")
        element = settings.writeXml(doc, QgsReadWriteContext())

        settings2 = QgsDirectionalLightSettings()
        settings2.readXml(element, QgsReadWriteContext())

        self.assertEqual(settings, settings2)


class TestQgsPointLightSettings(QgisTestCase):
    def test_getters_setters(self):
        settings = QgsPointLightSettings()

        self.assertEqual(settings.type(), Qgis.LightSourceType.Point)

        self.assertEqual(settings.position(), QgsVector3D(0, 0, 0))
        self.assertEqual(settings.color(), QColor(Qt.GlobalColor.white))
        self.assertEqual(settings.intensity(), 1.0)
        self.assertEqual(settings.constantAttenuation(), 1.0)
        self.assertEqual(settings.linearAttenuation(), 0.0)
        self.assertEqual(settings.quadraticAttenuation(), 0.0)
        self.assertTrue(settings.id())

        settings.setPosition(QgsVector3D(10.0, 20.0, 30.0))
        self.assertEqual(settings.position(), QgsVector3D(10.0, 20.0, 30.0))

        settings.setColor(QColor(0, 255, 0))
        self.assertEqual(settings.color(), QColor(0, 255, 0))

        settings.setIntensity(1.5)
        self.assertEqual(settings.intensity(), 1.5)

        settings.setConstantAttenuation(0.5)
        self.assertEqual(settings.constantAttenuation(), 0.5)

        settings.setLinearAttenuation(0.2)
        self.assertEqual(settings.linearAttenuation(), 0.2)

        settings.setQuadraticAttenuation(0.1)
        self.assertEqual(settings.quadraticAttenuation(), 0.1)

    def test_clone(self):
        settings = QgsPointLightSettings()
        settings.setPosition(QgsVector3D(10.0, 20.0, 30.0))
        settings.setColor(QColor(0, 255, 0))
        settings.setIntensity(1.5)
        settings.setConstantAttenuation(0.5)
        settings.setLinearAttenuation(0.2)
        settings.setQuadraticAttenuation(0.1)

        cloned = settings.clone()
        self.assertIsInstance(cloned, QgsPointLightSettings)
        self.assertEqual(cloned.position(), QgsVector3D(10.0, 20.0, 30.0))
        self.assertEqual(cloned.color(), QColor(0, 255, 0))
        self.assertEqual(cloned.intensity(), 1.5)
        self.assertEqual(cloned.constantAttenuation(), 0.5)
        self.assertEqual(cloned.linearAttenuation(), 0.2)
        self.assertEqual(cloned.quadraticAttenuation(), 0.1)
        self.assertEqual(cloned.id(), settings.id())

    def test_equality(self):
        settings1 = QgsPointLightSettings()
        # start with a clone so we get equal ID
        settings2 = settings1.clone()

        self.assertEqual(settings1, settings2)

        settings2.setPosition(QgsVector3D(10.0, 20.0, 30.0))
        self.assertNotEqual(settings1, settings2)
        settings1.setPosition(QgsVector3D(10.0, 20.0, 30.0))
        self.assertEqual(settings1, settings2)

        settings2.setColor(QColor(0, 255, 0))
        self.assertNotEqual(settings1, settings2)
        settings1.setColor(QColor(0, 255, 0))
        self.assertEqual(settings1, settings2)

        settings2.setIntensity(1.5)
        self.assertNotEqual(settings1, settings2)
        settings1.setIntensity(1.5)
        self.assertEqual(settings1, settings2)

        settings2.setConstantAttenuation(0.5)
        self.assertNotEqual(settings1, settings2)
        settings1.setConstantAttenuation(0.5)
        self.assertEqual(settings1, settings2)

        settings2.setLinearAttenuation(0.2)
        self.assertNotEqual(settings1, settings2)
        settings1.setLinearAttenuation(0.2)
        self.assertEqual(settings1, settings2)

        settings2.setQuadraticAttenuation(0.1)
        self.assertNotEqual(settings1, settings2)
        settings1.setQuadraticAttenuation(0.1)
        self.assertEqual(settings1, settings2)

        settings2 = QgsPointLightSettings()
        settings2.setPosition(QgsVector3D(10.0, 20.0, 30.0))
        settings2.setColor(QColor(0, 255, 0))
        settings2.setIntensity(1.5)
        settings2.setConstantAttenuation(0.5)
        settings2.setLinearAttenuation(0.2)
        settings2.setQuadraticAttenuation(0.1)
        # should be not equal, different ID
        self.assertNotEqual(settings1.id(), settings2.id())
        self.assertNotEqual(settings1, settings2)

    def test_xml_roundtrip(self):
        settings = QgsPointLightSettings()
        settings.setPosition(QgsVector3D(10.0, 20.0, 30.0))
        settings.setColor(QColor(0, 255, 0))
        settings.setIntensity(1.5)
        settings.setConstantAttenuation(0.5)
        settings.setLinearAttenuation(0.2)
        settings.setQuadraticAttenuation(0.1)

        doc = QDomDocument("settings")
        element = settings.writeXml(doc, QgsReadWriteContext())

        settings2 = QgsPointLightSettings()
        settings2.readXml(element, QgsReadWriteContext())

        self.assertEqual(settings, settings2)


class TestQgsSunLightSettings(QgisTestCase):
    def test_getters_setters(self):
        settings = QgsSunLightSettings()

        self.assertEqual(settings.type(), Qgis.LightSourceType.Sun)

        self.assertEqual(settings.color(), QColor(Qt.GlobalColor.white))
        self.assertEqual(settings.intensity(), 1.0)
        self.assertEqual(settings.atmosphericPressure(), 1013.25)
        self.assertEqual(settings.temperature(), 15.0)
        self.assertEqual(settings.referenceElevation(), 0.0)
        self.assertTrue(settings.sunTime().isValid())

        self.assertTrue(settings.id())

        settings.setColor(QColor(255, 255, 200))
        self.assertEqual(settings.color(), QColor(255, 255, 200))

        settings.setIntensity(1.2)
        self.assertEqual(settings.intensity(), 1.2)

        test_time = QDateTime.fromString("2026-05-31T12:00:00Z", Qt.DateFormat.ISODate)
        settings.setSunTime(test_time)
        self.assertEqual(settings.sunTime(), test_time)

        settings.setAtmosphericPressure(1000.0)
        self.assertEqual(settings.atmosphericPressure(), 1000.0)

        settings.setTemperature(25.0)
        self.assertEqual(settings.temperature(), 25.0)

        settings.setReferenceElevation(150.0)
        self.assertEqual(settings.referenceElevation(), 150.0)

    def test_clone(self):
        settings = QgsSunLightSettings()
        settings.setColor(QColor(255, 255, 200))
        settings.setIntensity(1.2)
        test_time = QDateTime.fromString("2026-05-31T12:00:00Z", Qt.DateFormat.ISODate)
        settings.setSunTime(test_time)
        settings.setAtmosphericPressure(1000.0)
        settings.setTemperature(25.0)
        settings.setReferenceElevation(150.0)

        cloned = settings.clone()
        self.assertIsInstance(cloned, QgsSunLightSettings)
        self.assertEqual(cloned.color(), QColor(255, 255, 200))
        self.assertEqual(cloned.intensity(), 1.2)
        self.assertEqual(cloned.sunTime(), test_time)
        self.assertEqual(cloned.atmosphericPressure(), 1000.0)
        self.assertEqual(cloned.temperature(), 25.0)
        self.assertEqual(cloned.referenceElevation(), 150.0)
        self.assertEqual(cloned.id(), settings.id())

    def test_equality(self):
        settings1 = QgsSunLightSettings()
        # start with a clone so we get equal ID
        settings2 = settings1.clone()

        self.assertEqual(settings1, settings2)

        settings2.setColor(QColor(255, 255, 200))
        self.assertNotEqual(settings1, settings2)
        settings1.setColor(QColor(255, 255, 200))
        self.assertEqual(settings1, settings2)

        settings2.setIntensity(1.2)
        self.assertNotEqual(settings1, settings2)
        settings1.setIntensity(1.2)
        self.assertEqual(settings1, settings2)

        test_time = QDateTime.fromString("2026-05-31T12:00:00Z", Qt.DateFormat.ISODate)
        settings2.setSunTime(test_time)
        self.assertNotEqual(settings1, settings2)
        settings1.setSunTime(test_time)
        self.assertEqual(settings1, settings2)

        settings2.setAtmosphericPressure(1000.0)
        self.assertNotEqual(settings1, settings2)
        settings1.setAtmosphericPressure(1000.0)
        self.assertEqual(settings1, settings2)

        settings2.setTemperature(25.0)
        self.assertNotEqual(settings1, settings2)
        settings1.setTemperature(25.0)
        self.assertEqual(settings1, settings2)

        settings2.setReferenceElevation(150.0)
        self.assertNotEqual(settings1, settings2)
        settings1.setReferenceElevation(150.0)
        self.assertEqual(settings1, settings2)

        settings2 = QgsSunLightSettings()
        settings2.setColor(QColor(255, 255, 200))
        settings2.setIntensity(1.2)
        settings2.setSunTime(test_time)
        settings2.setAtmosphericPressure(1000.0)
        settings2.setTemperature(25.0)
        settings2.setReferenceElevation(150.0)
        # should be not equal, different ID
        self.assertNotEqual(settings1.id(), settings2.id())
        self.assertNotEqual(settings1, settings2)

    def test_xml_roundtrip(self):
        settings = QgsSunLightSettings()
        settings.setColor(QColor(255, 255, 200))
        settings.setIntensity(1.2)
        test_time = QDateTime.fromString("2026-05-31T12:00:00Z", Qt.DateFormat.ISODate)
        settings.setSunTime(test_time)
        settings.setAtmosphericPressure(1000.0)
        settings.setTemperature(25.0)
        settings.setReferenceElevation(150.0)

        doc = QDomDocument("settings")
        element = settings.writeXml(doc, QgsReadWriteContext())

        settings2 = QgsSunLightSettings()
        settings2.readXml(element, QgsReadWriteContext())

        self.assertEqual(settings, settings2)


if __name__ == "__main__":
    unittest.main()
