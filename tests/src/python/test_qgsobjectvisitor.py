"""QGIS Unit tests for QgsObjectVisitorInterface and related classes.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Mathieu Pellerin"
__date__ = "2025-010"
__copyright__ = "Copyright 2025, The QGIS Project"


from qgis.core import (
    QgsProject,
    QgsEmbeddedScriptVisitor,
    QgsVectorLayer,
    QgsAction,
    QgsActionManager,
    QgsEditFormConfig,
    Qgis,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()


class TestQgsObjectVisitor(QgisTestCase):

    def test_layersMatchingPath(self):
        """
        Test QgsEmbeddedScriptVisitor()
        """

        p = QgsProject()
        p.writeEntry("Macros", "/pythonCode", "macros123")
        p.writeEntry("ExpressionFunctions", "/pythonCode", "functions123")

        visitor = QgsEmbeddedScriptVisitor()
        p.accept(visitor)

        self.assertEqual(len(visitor.embeddedScripts()), 2)
        self.assertEqual(
            visitor.embeddedScripts()[0].type(),
            Qgis.EmbeddedScriptType.Macro,
        )
        self.assertEqual(visitor.embeddedScripts()[0].name(), "Macros")
        self.assertEqual(visitor.embeddedScripts()[0].script(), "macros123")
        self.assertEqual(
            visitor.embeddedScripts()[1].type(),
            Qgis.EmbeddedScriptType.ExpressionFunction,
        )
        self.assertEqual(visitor.embeddedScripts()[1].name(), "Expression functions")
        self.assertEqual(visitor.embeddedScripts()[1].script(), "functions123")

        layer = QgsVectorLayer("Point?field=fldtxt:string", "tmplayer", "memory")
        layer.actions().addAction(
            Qgis.AttributeActionType.GenericPython, "action", "action123"
        )
        editFormConfig = layer.editFormConfig()
        editFormConfig.setInitCodeSource(Qgis.AttributeFormPythonInitCodeSource.Dialog)
        editFormConfig.setInitFunction("test")
        editFormConfig.setInitCode("init123")
        layer.setEditFormConfig(editFormConfig)

        p.addMapLayer(layer)

        visitor = QgsEmbeddedScriptVisitor()
        p.accept(visitor)

        self.assertEqual(len(visitor.embeddedScripts()), 4)
        self.assertEqual(
            visitor.embeddedScripts()[2].type(),
            Qgis.EmbeddedScriptType.Action,
        )
        self.assertEqual(
            visitor.embeddedScripts()[2].name(), "tmplayer: Action ’action’"
        )
        self.assertEqual(visitor.embeddedScripts()[2].script(), "action123")
        self.assertEqual(
            visitor.embeddedScripts()[3].type(),
            Qgis.EmbeddedScriptType.FormInitCode,
        )
        self.assertEqual(
            visitor.embeddedScripts()[3].name(), "tmplayer: Attribute form init code"
        )
        self.assertEqual(
            visitor.embeddedScripts()[3].script(),
            "# Calling function ’test’\n\ninit123",
        )


if __name__ == "__main__":
    unittest.main()
