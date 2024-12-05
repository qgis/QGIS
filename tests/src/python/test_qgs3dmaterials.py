"""QGIS Unit tests for 3d materials.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

from qgis.PyQt.QtGui import QColor
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import QgsReadWriteContext
from qgis._3d import (
    QgsSimpleLineMaterialSettings,
    QgsPhongMaterialSettings,
    QgsGoochMaterialSettings,
    QgsMetalRoughMaterialSettings,
    QgsPhongTexturedMaterialSettings,
    QgsNullMaterialSettings,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsSimpleLineMaterialSettings(QgisTestCase):

    def test_getters_setters(self):
        # Create fresh instance
        settings = QgsSimpleLineMaterialSettings()

        # Test default value
        self.assertEqual(settings.ambient(), QColor.fromRgbF(0.1, 0.1, 0.1, 1.0))

        # Test setter/getter
        settings.setAmbient(QColor(255, 0, 0))
        self.assertEqual(settings.ambient(), QColor(255, 0, 0))

    def test_clone(self):
        settings = QgsSimpleLineMaterialSettings()
        settings.setAmbient(QColor(255, 0, 0))

        cloned = settings.clone()
        self.assertIsInstance(cloned, QgsSimpleLineMaterialSettings)
        self.assertEqual(cloned.ambient(), QColor(255, 0, 0))

    def test_equality(self):
        settings1 = QgsSimpleLineMaterialSettings()
        settings2 = QgsSimpleLineMaterialSettings()

        # Should be equal with default values
        self.assertEqual(settings1, settings2)

        # Change one property at a time
        settings2.setAmbient(QColor(255, 0, 0))
        self.assertNotEqual(settings1, settings2)

        settings1.setAmbient(QColor(255, 0, 0))
        self.assertEqual(settings1, settings2)

    def test_equals_method(self):
        settings1 = QgsSimpleLineMaterialSettings()
        settings2 = QgsSimpleLineMaterialSettings()

        self.assertTrue(settings1.equals(settings2))

        settings2.setAmbient(QColor(255, 0, 0))
        self.assertFalse(settings1.equals(settings2))

        settings1.setAmbient(QColor(255, 0, 0))
        self.assertTrue(settings1.equals(settings2))

    def test_xml_roundtrip(self):
        settings = QgsSimpleLineMaterialSettings()
        settings.setAmbient(QColor(255, 0, 0))

        # Write to XML
        doc = QDomDocument("settings")
        element = doc.createElement("settings")
        settings.writeXml(element, QgsReadWriteContext())

        # Read from XML
        settings2 = QgsSimpleLineMaterialSettings()
        settings2.readXml(element, QgsReadWriteContext())

        # Verify round trip
        self.assertEqual(settings, settings2)
        self.assertEqual(settings2.ambient(), QColor(255, 0, 0))


class TestQgsPhongMaterialSettings(QgisTestCase):

    def test_getters_setters(self):
        settings = QgsPhongMaterialSettings()

        # Test default values
        self.assertEqual(settings.ambient(), QColor.fromRgbF(0.1, 0.1, 0.1, 1.0))
        self.assertIn(settings.diffuse().name(), ("#b2b2b2", "#b3b3b3"))
        self.assertEqual(settings.specular(), QColor.fromRgbF(1.0, 1.0, 1.0, 1.0))
        self.assertEqual(settings.shininess(), 0.0)
        self.assertEqual(settings.opacity(), 1.0)
        self.assertEqual(settings.ambientCoefficient(), 1.0)
        self.assertEqual(settings.diffuseCoefficient(), 1.0)
        self.assertEqual(settings.specularCoefficient(), 1.0)

        # Test setters/getters
        settings.setAmbient(QColor(255, 0, 0))
        self.assertEqual(settings.ambient(), QColor(255, 0, 0))

        settings.setDiffuse(QColor(0, 255, 0))
        self.assertEqual(settings.diffuse(), QColor(0, 255, 0))

        settings.setSpecular(QColor(0, 0, 255))
        self.assertEqual(settings.specular(), QColor(0, 0, 255))

        settings.setShininess(0.5)
        self.assertEqual(settings.shininess(), 0.5)

        settings.setOpacity(0.7)
        self.assertEqual(settings.opacity(), 0.7)

        settings.setAmbientCoefficient(0.8)
        self.assertEqual(settings.ambientCoefficient(), 0.8)

        settings.setDiffuseCoefficient(0.9)
        self.assertEqual(settings.diffuseCoefficient(), 0.9)

        settings.setSpecularCoefficient(0.6)
        self.assertEqual(settings.specularCoefficient(), 0.6)

    def test_clone(self):
        settings = QgsPhongMaterialSettings()
        settings.setAmbient(QColor(255, 0, 0))
        settings.setDiffuse(QColor(0, 255, 0))
        settings.setSpecular(QColor(0, 0, 255))
        settings.setShininess(0.5)
        settings.setOpacity(0.7)
        settings.setAmbientCoefficient(0.8)
        settings.setDiffuseCoefficient(0.9)
        settings.setSpecularCoefficient(0.6)

        cloned = settings.clone()
        self.assertIsInstance(cloned, QgsPhongMaterialSettings)

        self.assertEqual(cloned.ambient(), QColor(255, 0, 0))
        self.assertEqual(cloned.diffuse(), QColor(0, 255, 0))
        self.assertEqual(cloned.specular(), QColor(0, 0, 255))
        self.assertEqual(cloned.shininess(), 0.5)
        self.assertEqual(cloned.opacity(), 0.7)
        self.assertEqual(cloned.ambientCoefficient(), 0.8)
        self.assertEqual(cloned.diffuseCoefficient(), 0.9)
        self.assertEqual(cloned.specularCoefficient(), 0.6)

    def test_equality(self):
        settings1 = QgsPhongMaterialSettings()
        settings2 = QgsPhongMaterialSettings()

        self.assertEqual(settings1, settings2)

        settings2.setAmbient(QColor(255, 0, 0))
        self.assertNotEqual(settings1, settings2)
        settings1.setAmbient(QColor(255, 0, 0))
        self.assertEqual(settings1, settings2)

        settings2.setDiffuse(QColor(0, 255, 0))
        self.assertNotEqual(settings1, settings2)
        settings1.setDiffuse(QColor(0, 255, 0))
        self.assertEqual(settings1, settings2)

        settings2.setSpecular(QColor(0, 0, 255))
        self.assertNotEqual(settings1, settings2)
        settings1.setSpecular(QColor(0, 0, 255))
        self.assertEqual(settings1, settings2)

        settings2.setShininess(0.5)
        self.assertNotEqual(settings1, settings2)
        settings1.setShininess(0.5)
        self.assertEqual(settings1, settings2)

        settings2.setOpacity(0.7)
        self.assertNotEqual(settings1, settings2)
        settings1.setOpacity(0.7)
        self.assertEqual(settings1, settings2)

        settings2.setAmbientCoefficient(0.8)
        self.assertNotEqual(settings1, settings2)
        settings1.setAmbientCoefficient(0.8)
        self.assertEqual(settings1, settings2)

        settings2.setDiffuseCoefficient(0.9)
        self.assertNotEqual(settings1, settings2)
        settings1.setDiffuseCoefficient(0.9)
        self.assertEqual(settings1, settings2)

        settings2.setSpecularCoefficient(0.6)
        self.assertNotEqual(settings1, settings2)
        settings1.setSpecularCoefficient(0.6)
        self.assertEqual(settings1, settings2)

    def test_equals_method(self):
        settings1 = QgsPhongMaterialSettings()
        settings2 = QgsPhongMaterialSettings()

        self.assertTrue(settings1.equals(settings2))

        settings2.setAmbient(QColor(255, 0, 0))
        self.assertFalse(settings1.equals(settings2))

        settings1.setAmbient(QColor(255, 0, 0))
        self.assertTrue(settings1.equals(settings2))

    def test_xml_roundtrip(self):
        settings = QgsPhongMaterialSettings()
        settings.setAmbient(QColor(255, 0, 0))
        settings.setDiffuse(QColor(0, 255, 0))
        settings.setSpecular(QColor(0, 0, 255))
        settings.setShininess(0.5)
        settings.setOpacity(0.7)
        settings.setAmbientCoefficient(0.8)
        settings.setDiffuseCoefficient(0.9)
        settings.setSpecularCoefficient(0.6)

        doc = QDomDocument("settings")
        element = doc.createElement("settings")
        settings.writeXml(element, QgsReadWriteContext())

        settings2 = QgsPhongMaterialSettings()
        settings2.readXml(element, QgsReadWriteContext())

        self.assertEqual(settings, settings2)


class TestQgsGoochMaterialSettings(QgisTestCase):

    def test_getters_setters(self):
        settings = QgsGoochMaterialSettings()

        # Test default values
        self.assertEqual(settings.warm(), QColor(107, 0, 107))
        self.assertEqual(settings.cool(), QColor(255, 130, 0))
        self.assertIn(settings.diffuse().name(), ("#b2b2b2", "#b3b3b3"))
        self.assertEqual(settings.specular(), QColor.fromRgbF(1.0, 1.0, 1.0, 1.0))
        self.assertEqual(settings.shininess(), 100.0)
        self.assertEqual(settings.alpha(), 0.25)
        self.assertEqual(settings.beta(), 0.5)

        # Test setters/getters
        settings.setWarm(QColor(100, 0, 100))
        self.assertEqual(settings.warm(), QColor(100, 0, 100))

        settings.setCool(QColor(200, 100, 0))
        self.assertEqual(settings.cool(), QColor(200, 100, 0))

        settings.setDiffuse(QColor(0, 255, 0))
        self.assertEqual(settings.diffuse(), QColor(0, 255, 0))

        settings.setSpecular(QColor(0, 0, 255))
        self.assertEqual(settings.specular(), QColor(0, 0, 255))

        settings.setShininess(50.0)
        self.assertEqual(settings.shininess(), 50.0)

        settings.setAlpha(0.4)
        self.assertEqual(settings.alpha(), 0.4)

        settings.setBeta(0.6)
        self.assertEqual(settings.beta(), 0.6)

    def test_clone(self):
        settings = QgsGoochMaterialSettings()
        settings.setWarm(QColor(100, 0, 100))
        settings.setCool(QColor(200, 100, 0))
        settings.setDiffuse(QColor(0, 255, 0))
        settings.setSpecular(QColor(0, 0, 255))
        settings.setShininess(50.0)
        settings.setAlpha(0.3)
        settings.setBeta(0.6)

        cloned = settings.clone()
        self.assertIsInstance(cloned, QgsGoochMaterialSettings)
        self.assertEqual(cloned.warm(), QColor(100, 0, 100))
        self.assertEqual(cloned.cool(), QColor(200, 100, 0))
        self.assertEqual(cloned.diffuse(), QColor(0, 255, 0))
        self.assertEqual(cloned.specular(), QColor(0, 0, 255))
        self.assertEqual(cloned.shininess(), 50.0)
        self.assertEqual(cloned.alpha(), 0.3)
        self.assertEqual(cloned.beta(), 0.6)

    def test_equality(self):
        settings1 = QgsGoochMaterialSettings()
        settings2 = QgsGoochMaterialSettings()

        self.assertEqual(settings1, settings2)

        settings2.setWarm(QColor(100, 0, 100))
        self.assertNotEqual(settings1, settings2)
        settings1.setWarm(QColor(100, 0, 100))
        self.assertEqual(settings1, settings2)

        settings2.setCool(QColor(200, 100, 0))
        self.assertNotEqual(settings1, settings2)
        settings1.setCool(QColor(200, 100, 0))
        self.assertEqual(settings1, settings2)

        settings2.setDiffuse(QColor(0, 255, 0))
        self.assertNotEqual(settings1, settings2)
        settings1.setDiffuse(QColor(0, 255, 0))
        self.assertEqual(settings1, settings2)

        settings2.setSpecular(QColor(0, 0, 255))
        self.assertNotEqual(settings1, settings2)
        settings1.setSpecular(QColor(0, 0, 255))
        self.assertEqual(settings1, settings2)

        settings2.setShininess(50.0)
        self.assertNotEqual(settings1, settings2)
        settings1.setShininess(50.0)
        self.assertEqual(settings1, settings2)

        settings2.setAlpha(0.3)
        self.assertNotEqual(settings1, settings2)
        settings1.setAlpha(0.3)
        self.assertEqual(settings1, settings2)

        settings2.setBeta(0.6)
        self.assertNotEqual(settings1, settings2)
        settings1.setBeta(0.6)
        self.assertEqual(settings1, settings2)

    def test_equals_method(self):
        settings1 = QgsGoochMaterialSettings()
        settings2 = QgsGoochMaterialSettings()

        self.assertTrue(settings1.equals(settings2))

        settings2.setWarm(QColor(100, 0, 100))
        self.assertFalse(settings1.equals(settings2))

        settings1.setWarm(QColor(100, 0, 100))
        self.assertTrue(settings1.equals(settings2))

    def test_xml_roundtrip(self):
        settings = QgsGoochMaterialSettings()
        settings.setWarm(QColor(100, 0, 100))
        settings.setCool(QColor(200, 100, 0))
        settings.setDiffuse(QColor(0, 255, 0))
        settings.setSpecular(QColor(0, 0, 255))
        settings.setShininess(50.0)
        settings.setAlpha(0.3)
        settings.setBeta(0.6)

        doc = QDomDocument("settings")
        element = doc.createElement("settings")
        settings.writeXml(element, QgsReadWriteContext())

        settings2 = QgsGoochMaterialSettings()
        settings2.readXml(element, QgsReadWriteContext())

        self.assertEqual(settings, settings2)


class TestQgsMetalRoughMaterialSettings(unittest.TestCase):

    def test_getters_setters(self):
        settings = QgsMetalRoughMaterialSettings()

        # Test default values
        self.assertEqual(settings.baseColor(), QColor.fromRgbF(0.5, 0.5, 0.5, 1.0))
        self.assertEqual(settings.metalness(), 0.0)
        self.assertEqual(settings.roughness(), 0.0)

        # Test setters/getters
        settings.setBaseColor(QColor(255, 0, 0))
        self.assertEqual(settings.baseColor(), QColor(255, 0, 0))

        settings.setMetalness(0.5)
        self.assertEqual(settings.metalness(), 0.5)

        settings.setRoughness(0.7)
        self.assertEqual(settings.roughness(), 0.7)

    def test_clone(self):
        settings = QgsMetalRoughMaterialSettings()
        settings.setBaseColor(QColor(255, 0, 0))
        settings.setMetalness(0.5)
        settings.setRoughness(0.7)

        cloned = settings.clone()
        self.assertIsInstance(cloned, QgsMetalRoughMaterialSettings)
        self.assertEqual(cloned.baseColor(), QColor(255, 0, 0))
        self.assertEqual(cloned.metalness(), 0.5)
        self.assertEqual(cloned.roughness(), 0.7)

    def test_equality(self):
        settings1 = QgsMetalRoughMaterialSettings()
        settings2 = QgsMetalRoughMaterialSettings()

        self.assertEqual(settings1, settings2)

        settings2.setBaseColor(QColor(255, 0, 0))
        self.assertNotEqual(settings1, settings2)
        settings1.setBaseColor(QColor(255, 0, 0))
        self.assertEqual(settings1, settings2)

        settings2.setMetalness(0.5)
        self.assertNotEqual(settings1, settings2)
        settings1.setMetalness(0.5)
        self.assertEqual(settings1, settings2)

        settings2.setRoughness(0.7)
        self.assertNotEqual(settings1, settings2)
        settings1.setRoughness(0.7)
        self.assertEqual(settings1, settings2)

    def test_equals_method(self):
        settings1 = QgsMetalRoughMaterialSettings()
        settings2 = QgsMetalRoughMaterialSettings()

        self.assertTrue(settings1.equals(settings2))

        settings2.setBaseColor(QColor(255, 0, 0))
        self.assertFalse(settings1.equals(settings2))

        settings1.setBaseColor(QColor(255, 0, 0))
        self.assertTrue(settings1.equals(settings2))

    def test_xml_roundtrip(self):
        settings = QgsMetalRoughMaterialSettings()
        settings.setBaseColor(QColor(255, 0, 0))
        settings.setMetalness(0.5)
        settings.setRoughness(0.7)

        doc = QDomDocument("settings")
        element = doc.createElement("settings")
        settings.writeXml(element, QgsReadWriteContext())

        settings2 = QgsMetalRoughMaterialSettings()
        settings2.readXml(element, QgsReadWriteContext())

        self.assertEqual(settings, settings2)


class TestQgsPhongTexturedMaterialSettings(QgisTestCase):

    def test_getters_setters(self):
        settings = QgsPhongTexturedMaterialSettings()

        # Test default values
        self.assertEqual(settings.ambient(), QColor.fromRgbF(0.1, 0.1, 0.1, 1.0))
        self.assertEqual(settings.specular(), QColor.fromRgbF(1.0, 1.0, 1.0, 1.0))
        self.assertEqual(settings.shininess(), 0.0)
        self.assertEqual(settings.diffuseTexturePath(), "")
        self.assertEqual(settings.textureScale(), 1.0)
        self.assertEqual(settings.textureRotation(), 0.0)
        self.assertEqual(settings.opacity(), 1.0)

        # Test setters/getters
        settings.setAmbient(QColor(255, 0, 0))
        self.assertEqual(settings.ambient(), QColor(255, 0, 0))

        settings.setSpecular(QColor(0, 0, 255))
        self.assertEqual(settings.specular(), QColor(0, 0, 255))

        settings.setShininess(0.5)
        self.assertEqual(settings.shininess(), 0.5)

        settings.setDiffuseTexturePath("/path/to/texture.png")
        self.assertEqual(settings.diffuseTexturePath(), "/path/to/texture.png")

        settings.setTextureScale(2.0)
        self.assertEqual(settings.textureScale(), 2.0)

        settings.setTextureRotation(45.0)
        self.assertEqual(settings.textureRotation(), 45.0)

        settings.setOpacity(0.7)
        self.assertEqual(settings.opacity(), 0.7)

    def test_clone(self):
        settings = QgsPhongTexturedMaterialSettings()
        settings.setAmbient(QColor(255, 0, 0))
        settings.setSpecular(QColor(0, 0, 255))
        settings.setShininess(0.5)
        settings.setDiffuseTexturePath("/path/to/texture.png")
        settings.setTextureScale(2.0)
        settings.setTextureRotation(45.0)
        settings.setOpacity(0.7)

        cloned = settings.clone()
        self.assertIsInstance(cloned, QgsPhongTexturedMaterialSettings)
        self.assertEqual(cloned.ambient(), QColor(255, 0, 0))
        self.assertEqual(cloned.specular(), QColor(0, 0, 255))
        self.assertEqual(cloned.shininess(), 0.5)
        self.assertEqual(cloned.diffuseTexturePath(), "/path/to/texture.png")
        self.assertEqual(cloned.textureScale(), 2.0)
        self.assertEqual(cloned.textureRotation(), 45.0)
        self.assertEqual(cloned.opacity(), 0.7)

    def test_equality(self):
        settings1 = QgsPhongTexturedMaterialSettings()
        settings2 = QgsPhongTexturedMaterialSettings()

        self.assertEqual(settings1, settings2)

        settings2.setAmbient(QColor(255, 0, 0))
        self.assertNotEqual(settings1, settings2)
        settings1.setAmbient(QColor(255, 0, 0))
        self.assertEqual(settings1, settings2)

        settings2.setSpecular(QColor(0, 0, 255))
        self.assertNotEqual(settings1, settings2)
        settings1.setSpecular(QColor(0, 0, 255))
        self.assertEqual(settings1, settings2)

        settings2.setShininess(0.5)
        self.assertNotEqual(settings1, settings2)
        settings1.setShininess(0.5)
        self.assertEqual(settings1, settings2)

        settings2.setDiffuseTexturePath("/path/to/texture.png")
        self.assertNotEqual(settings1, settings2)
        settings1.setDiffuseTexturePath("/path/to/texture.png")
        self.assertEqual(settings1, settings2)

        settings2.setTextureScale(2.0)
        self.assertNotEqual(settings1, settings2)
        settings1.setTextureScale(2.0)
        self.assertEqual(settings1, settings2)

        settings2.setTextureRotation(45.0)
        self.assertNotEqual(settings1, settings2)
        settings1.setTextureRotation(45.0)
        self.assertEqual(settings1, settings2)

        settings2.setOpacity(0.7)
        self.assertNotEqual(settings1, settings2)
        settings1.setOpacity(0.7)
        self.assertEqual(settings1, settings2)

    def test_equals_method(self):
        settings1 = QgsPhongTexturedMaterialSettings()
        settings2 = QgsPhongTexturedMaterialSettings()

        self.assertTrue(settings1.equals(settings2))

        settings2.setAmbient(QColor(255, 0, 0))
        self.assertFalse(settings1.equals(settings2))

        settings1.setAmbient(QColor(255, 0, 0))
        self.assertTrue(settings1.equals(settings2))

    def test_xml_roundtrip(self):
        settings = QgsPhongTexturedMaterialSettings()
        settings.setAmbient(QColor(255, 0, 0))
        settings.setSpecular(QColor(0, 0, 255))
        settings.setShininess(0.5)
        settings.setDiffuseTexturePath("/path/to/texture.png")
        settings.setTextureScale(2.0)
        settings.setTextureRotation(45.0)
        settings.setOpacity(0.7)

        doc = QDomDocument("settings")
        element = doc.createElement("settings")
        settings.writeXml(element, QgsReadWriteContext())

        settings2 = QgsPhongTexturedMaterialSettings()
        settings2.readXml(element, QgsReadWriteContext())

        self.assertEqual(settings, settings2)

    def test_requires_texture_coordinates(self):
        settings = QgsPhongTexturedMaterialSettings()
        self.assertFalse(settings.requiresTextureCoordinates())

        settings.setDiffuseTexturePath("/path/to/texture.png")
        self.assertTrue(settings.requiresTextureCoordinates())

        settings.setDiffuseTexturePath("")
        self.assertFalse(settings.requiresTextureCoordinates())


class TestQgsNullMaterialSettings(QgisTestCase):

    def test_clone(self):
        settings = QgsNullMaterialSettings()
        cloned = settings.clone()
        self.assertIsInstance(cloned, QgsNullMaterialSettings)
        self.assertTrue(settings.equals(cloned))

    def test_equals(self):
        settings1 = QgsNullMaterialSettings()
        settings2 = QgsNullMaterialSettings()
        self.assertTrue(settings1.equals(settings2))

    def test_xml_roundtrip(self):
        settings = QgsNullMaterialSettings()

        doc = QDomDocument("settings")
        element = doc.createElement("settings")
        settings.writeXml(element, QgsReadWriteContext())

        settings2 = QgsNullMaterialSettings()
        settings2.readXml(element, QgsReadWriteContext())

        self.assertTrue(settings.equals(settings2))


if __name__ == "__main__":
    unittest.main()
