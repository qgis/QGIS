"""QGIS Unit tests for labeling engine rules

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os
import tempfile

from qgis.PyQt.QtXml import QDomDocument

from qgis.core import (
    Qgis,
    QgsLabelingEngineRuleRegistry,
    QgsAbstractLabelingEngineRule,
    QgsLabelingEngineRuleMinimumDistanceLabelToFeature,
    QgsLabelingEngineRuleMinimumDistanceLabelToLabel,
    QgsLabelingEngineRuleMaximumDistanceLabelToFeature,
    QgsLabelingEngineRuleAvoidLabelOverlapWithFeature,
    QgsProject,
    QgsVectorLayer,
    QgsMapUnitScale,
    QgsReadWriteContext,
    QgsLabelingEngineSettings,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestRule(QgsAbstractLabelingEngineRule):

    def id(self):
        return "test"

    def displayType(self):
        return "my type"

    def prepare(self, context):
        pass

    def writeXml(self, doc, element, context):
        pass

    def modifyProblem(self):
        pass

    def readXml(self, element, context):
        pass

    def clone(self):
        return TestRule()


class TestQgsLabelingEngineRule(QgisTestCase):

    def testRegistry(self):
        registry = QgsLabelingEngineRuleRegistry()
        self.assertTrue(registry.ruleIds())
        for rule_id in registry.ruleIds():
            self.assertEqual(registry.create(rule_id).id(), rule_id)

        self.assertIsNone(registry.create("bad"))
        self.assertFalse(registry.displayType("bad"))

        self.assertIn("minimumDistanceLabelToFeature", registry.ruleIds())

        self.assertFalse(registry.addRule(None))

        self.assertTrue(registry.addRule(TestRule()))

        self.assertIn("test", registry.ruleIds())
        self.assertIsInstance(registry.create("test"), TestRule)
        self.assertEqual(registry.displayType("test"), "my type")

        # no duplicates
        self.assertFalse(registry.addRule(TestRule()))

        registry.removeRule("test")

        self.assertNotIn("test", registry.ruleIds())
        self.assertIsNone(registry.create("test"))

        registry.removeRule("test")

    def testMinimumDistanceLabelToFeature(self):
        p = QgsProject()
        vl = QgsVectorLayer("Point", "layer 1", "memory")
        vl2 = QgsVectorLayer("Point", "layer 2", "memory")
        p.addMapLayers([vl, vl2])

        rule = QgsLabelingEngineRuleMinimumDistanceLabelToFeature()
        rule.setLabeledLayer(vl)
        rule.setTargetLayer(vl2)
        rule.setDistance(14)
        rule.setDistanceUnit(Qgis.RenderUnit.Inches)
        rule.setDistanceUnitScale(QgsMapUnitScale(15, 25))
        rule.setCost(6.6)
        rule.setName("my rule")
        rule.setActive(False)
        self.assertEqual(
            rule.__repr__(),
            "<QgsLabelingEngineRuleMinimumDistanceLabelToFeature: my rule>",
        )

        self.assertEqual(rule.labeledLayer(), vl)
        self.assertEqual(rule.targetLayer(), vl2)
        self.assertEqual(rule.distance(), 14)
        self.assertEqual(rule.distanceUnit(), Qgis.RenderUnit.Inches)
        self.assertEqual(rule.distanceUnitScale().minScale, 15)
        self.assertEqual(rule.distanceUnitScale().maxScale, 25)
        self.assertEqual(rule.cost(), 6.6)
        self.assertEqual(rule.name(), "my rule")
        self.assertFalse(rule.active())

        rule2 = rule.clone()
        self.assertEqual(rule2.labeledLayer(), vl)
        self.assertEqual(rule2.targetLayer(), vl2)
        self.assertEqual(rule2.distance(), 14)
        self.assertEqual(rule2.distanceUnit(), Qgis.RenderUnit.Inches)
        self.assertEqual(rule2.distanceUnitScale().minScale, 15)
        self.assertEqual(rule2.distanceUnitScale().maxScale, 25)
        self.assertEqual(rule2.cost(), 6.6)
        self.assertEqual(rule2.name(), "my rule")
        self.assertFalse(rule2.active())

        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        rule.writeXml(doc, elem, QgsReadWriteContext())

        rule3 = QgsLabelingEngineRuleMinimumDistanceLabelToFeature()
        rule3.readXml(elem, QgsReadWriteContext())
        rule3.resolveReferences(p)
        self.assertEqual(rule3.labeledLayer(), vl)
        self.assertEqual(rule3.targetLayer(), vl2)
        self.assertEqual(rule3.distance(), 14)
        self.assertEqual(rule3.distanceUnit(), Qgis.RenderUnit.Inches)
        self.assertEqual(rule3.distanceUnitScale().minScale, 15)
        self.assertEqual(rule3.distanceUnitScale().maxScale, 25)
        self.assertEqual(rule3.cost(), 6.6)

    def testMinimumDistanceLabelToLabel(self):
        p = QgsProject()
        vl = QgsVectorLayer("Point", "layer 1", "memory")
        vl2 = QgsVectorLayer("Point", "layer 2", "memory")
        p.addMapLayers([vl, vl2])

        rule = QgsLabelingEngineRuleMinimumDistanceLabelToLabel()
        rule.setLabeledLayer(vl)
        rule.setTargetLayer(vl2)
        rule.setDistance(14)
        rule.setDistanceUnit(Qgis.RenderUnit.Inches)
        rule.setDistanceUnitScale(QgsMapUnitScale(15, 25))
        rule.setName("my rule")
        self.assertEqual(
            rule.__repr__(),
            "<QgsLabelingEngineRuleMinimumDistanceLabelToLabel: my rule>",
        )

        self.assertEqual(rule.labeledLayer(), vl)
        self.assertEqual(rule.targetLayer(), vl2)
        self.assertEqual(rule.distance(), 14)
        self.assertEqual(rule.distanceUnit(), Qgis.RenderUnit.Inches)
        self.assertEqual(rule.distanceUnitScale().minScale, 15)
        self.assertEqual(rule.distanceUnitScale().maxScale, 25)
        self.assertEqual(rule.name(), "my rule")
        self.assertTrue(rule.active())

        rule2 = rule.clone()
        self.assertEqual(rule2.labeledLayer(), vl)
        self.assertEqual(rule2.targetLayer(), vl2)
        self.assertEqual(rule2.distance(), 14)
        self.assertEqual(rule2.distanceUnit(), Qgis.RenderUnit.Inches)
        self.assertEqual(rule2.distanceUnitScale().minScale, 15)
        self.assertEqual(rule2.distanceUnitScale().maxScale, 25)
        self.assertEqual(rule2.name(), "my rule")
        self.assertTrue(rule2.active())

        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        rule.writeXml(doc, elem, QgsReadWriteContext())

        rule3 = QgsLabelingEngineRuleMinimumDistanceLabelToLabel()
        rule3.readXml(elem, QgsReadWriteContext())
        rule3.resolveReferences(p)
        self.assertEqual(rule3.labeledLayer(), vl)
        self.assertEqual(rule3.targetLayer(), vl2)
        self.assertEqual(rule3.distance(), 14)
        self.assertEqual(rule3.distanceUnit(), Qgis.RenderUnit.Inches)
        self.assertEqual(rule3.distanceUnitScale().minScale, 15)
        self.assertEqual(rule3.distanceUnitScale().maxScale, 25)

    def testMaximumDistanceLabelToFeature(self):
        p = QgsProject()
        vl = QgsVectorLayer("Point", "layer 1", "memory")
        vl2 = QgsVectorLayer("Point", "layer 2", "memory")
        p.addMapLayers([vl, vl2])

        rule = QgsLabelingEngineRuleMaximumDistanceLabelToFeature()
        rule.setLabeledLayer(vl)
        rule.setTargetLayer(vl2)
        rule.setDistance(14)
        rule.setDistanceUnit(Qgis.RenderUnit.Inches)
        rule.setDistanceUnitScale(QgsMapUnitScale(15, 25))
        rule.setCost(6.6)
        rule.setName("my rule")
        self.assertEqual(
            rule.__repr__(),
            "<QgsLabelingEngineRuleMaximumDistanceLabelToFeature: my rule>",
        )

        self.assertEqual(rule.labeledLayer(), vl)
        self.assertEqual(rule.targetLayer(), vl2)
        self.assertEqual(rule.distance(), 14)
        self.assertEqual(rule.distanceUnit(), Qgis.RenderUnit.Inches)
        self.assertEqual(rule.distanceUnitScale().minScale, 15)
        self.assertEqual(rule.distanceUnitScale().maxScale, 25)
        self.assertEqual(rule.cost(), 6.6)
        self.assertEqual(rule.name(), "my rule")

        rule2 = rule.clone()
        self.assertEqual(rule2.labeledLayer(), vl)
        self.assertEqual(rule2.targetLayer(), vl2)
        self.assertEqual(rule2.distance(), 14)
        self.assertEqual(rule2.distanceUnit(), Qgis.RenderUnit.Inches)
        self.assertEqual(rule2.distanceUnitScale().minScale, 15)
        self.assertEqual(rule2.distanceUnitScale().maxScale, 25)
        self.assertEqual(rule2.cost(), 6.6)
        self.assertEqual(rule2.name(), "my rule")

        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        rule.writeXml(doc, elem, QgsReadWriteContext())

        rule3 = QgsLabelingEngineRuleMaximumDistanceLabelToFeature()
        rule3.readXml(elem, QgsReadWriteContext())
        rule3.resolveReferences(p)
        self.assertEqual(rule3.labeledLayer(), vl)
        self.assertEqual(rule3.targetLayer(), vl2)
        self.assertEqual(rule3.distance(), 14)
        self.assertEqual(rule3.distanceUnit(), Qgis.RenderUnit.Inches)
        self.assertEqual(rule3.distanceUnitScale().minScale, 15)
        self.assertEqual(rule3.distanceUnitScale().maxScale, 25)
        self.assertEqual(rule3.cost(), 6.6)

    def testAvoidLabelOverlapWithFeature(self):
        p = QgsProject()
        vl = QgsVectorLayer("Point", "layer 1", "memory")
        vl2 = QgsVectorLayer("Point", "layer 2", "memory")
        p.addMapLayers([vl, vl2])

        rule = QgsLabelingEngineRuleAvoidLabelOverlapWithFeature()
        rule.setLabeledLayer(vl)
        rule.setTargetLayer(vl2)
        rule.setName("my rule")
        self.assertEqual(
            rule.__repr__(),
            "<QgsLabelingEngineRuleAvoidLabelOverlapWithFeature: my rule>",
        )

        self.assertEqual(rule.labeledLayer(), vl)
        self.assertEqual(rule.targetLayer(), vl2)
        self.assertEqual(rule.name(), "my rule")

        rule2 = rule.clone()
        self.assertEqual(rule2.labeledLayer(), vl)
        self.assertEqual(rule2.targetLayer(), vl2)
        self.assertEqual(rule2.name(), "my rule")

        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        rule.writeXml(doc, elem, QgsReadWriteContext())

        rule3 = QgsLabelingEngineRuleAvoidLabelOverlapWithFeature()
        rule3.readXml(elem, QgsReadWriteContext())
        rule3.resolveReferences(p)
        self.assertEqual(rule3.labeledLayer(), vl)
        self.assertEqual(rule3.targetLayer(), vl2)

    def test_settings(self):
        """
        Test attaching rules to QgsLabelingEngineSettings
        """
        p = QgsProject()
        vl = QgsVectorLayer("Point", "layer 1", "memory")
        vl2 = QgsVectorLayer("Point", "layer 2", "memory")
        p.addMapLayers([vl, vl2])

        self.assertFalse(p.labelingEngineSettings().rules())

        rule = QgsLabelingEngineRuleMaximumDistanceLabelToFeature()
        rule.setLabeledLayer(vl)
        rule.setTargetLayer(vl2)
        rule.setCost(6.6)
        rule.setName("first rule")

        label_engine_settings = p.labelingEngineSettings()
        label_engine_settings.addRule(rule)
        self.assertEqual(
            [r.id() for r in label_engine_settings.rules()],
            ["maximumDistanceLabelToFeature"],
        )

        rule2 = QgsLabelingEngineRuleAvoidLabelOverlapWithFeature()
        rule2.setLabeledLayer(vl2)
        rule2.setTargetLayer(vl)
        rule2.setName("the second rule of labeling")
        rule2.setActive(False)
        label_engine_settings.addRule(rule2)
        self.assertEqual(
            [r.id() for r in label_engine_settings.rules()],
            ["maximumDistanceLabelToFeature", "avoidLabelOverlapWithFeature"],
        )

        p.setLabelingEngineSettings(label_engine_settings)

        label_engine_settings = p.labelingEngineSettings()
        self.assertEqual(
            [r.id() for r in label_engine_settings.rules()],
            ["maximumDistanceLabelToFeature", "avoidLabelOverlapWithFeature"],
        )
        self.assertEqual(
            [r.name() for r in label_engine_settings.rules()],
            ["first rule", "the second rule of labeling"],
        )
        self.assertEqual(
            [r.active() for r in label_engine_settings.rules()], [True, False]
        )

        # save, restore project
        with tempfile.TemporaryDirectory() as temp_dir:
            self.assertTrue(p.write(os.path.join(temp_dir, "p.qgs")))

            p2 = QgsProject()
            self.assertTrue(p2.read(os.path.join(temp_dir, "p.qgs")))

            label_engine_settings = p2.labelingEngineSettings()
            self.assertEqual(
                [r.id() for r in label_engine_settings.rules()],
                ["maximumDistanceLabelToFeature", "avoidLabelOverlapWithFeature"],
            )
            self.assertEqual(
                [r.name() for r in label_engine_settings.rules()],
                ["first rule", "the second rule of labeling"],
            )
            self.assertEqual(
                [r.active() for r in label_engine_settings.rules()], [True, False]
            )

            # check layers, settings
            rule1 = label_engine_settings.rules()[0]
            self.assertIsInstance(
                rule1, QgsLabelingEngineRuleMaximumDistanceLabelToFeature
            )
            self.assertEqual(rule1.cost(), 6.6)
            self.assertEqual(rule1.labeledLayer().name(), "layer 1")
            self.assertEqual(rule1.targetLayer().name(), "layer 2")

            rule2 = label_engine_settings.rules()[1]
            self.assertIsInstance(
                rule2, QgsLabelingEngineRuleAvoidLabelOverlapWithFeature
            )
            self.assertEqual(rule2.labeledLayer().name(), "layer 2")
            self.assertEqual(rule2.targetLayer().name(), "layer 1")

        # test setRules
        rule = QgsLabelingEngineRuleMinimumDistanceLabelToFeature()
        rule.setLabeledLayer(vl)
        rule.setTargetLayer(vl2)
        rule.setCost(6.6)

        label_engine_settings.setRules([rule])
        self.assertEqual(
            [r.id() for r in label_engine_settings.rules()],
            ["minimumDistanceLabelToFeature"],
        )


if __name__ == "__main__":
    unittest.main()
