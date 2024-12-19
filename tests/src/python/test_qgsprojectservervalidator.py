"""QGIS Unit tests for QgsProjectServerValidator

From build dir, run:
ctest -R PyQgsProjectServerValidator -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Etienne Trimaille"
__date__ = "27/03/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

from qgis.core import QgsProject, QgsProjectServerValidator, QgsVectorLayer
import unittest
from qgis.testing import start_app, QgisTestCase

app = start_app()


class TestQgsprojectServerValidator(QgisTestCase):

    def test_project_server_validator(self):
        """Test project server validator."""
        project = QgsProject()
        layer = QgsVectorLayer("Point?field=fldtxt:string", "layer_1", "memory")
        project.addMapLayers([layer])

        # Valid
        valid, results = QgsProjectServerValidator.validate(project)
        self.assertTrue(valid)
        self.assertFalse(results)

        layer_1 = QgsVectorLayer("Point?field=fldtxt:string", "layer_1", "memory")
        project.addMapLayers([layer_1])

        # Not valid, same layer name
        valid, results = QgsProjectServerValidator.validate(project)
        self.assertFalse(valid)
        self.assertEqual(1, len(results))
        self.assertEqual(
            QgsProjectServerValidator.ValidationError.DuplicatedNames, results[0].error
        )

        # Not valid, short name is invalid
        layer_1.setShortName("layer_1_invalid_#")
        valid, results = QgsProjectServerValidator.validate(project)
        self.assertFalse(valid)
        self.assertEqual(1, len(results))
        self.assertEqual(
            QgsProjectServerValidator.ValidationError.LayerShortName, results[0].error
        )

        # Not valid, same short name as the first layer name
        layer_1.setShortName("layer_1")
        valid, results = QgsProjectServerValidator.validate(project)
        self.assertFalse(valid)
        self.assertEqual(1, len(results))
        self.assertEqual(
            QgsProjectServerValidator.ValidationError.DuplicatedNames, results[0].error
        )

        # Valid
        layer_1.setShortName("layer_1_bis")
        valid, results = QgsProjectServerValidator.validate(project)
        self.assertTrue(valid)
        self.assertEqual(0, len(results))

        # Not valid, a group with same name as the first layer
        group = project.layerTreeRoot().addGroup("layer_1")
        valid, results = QgsProjectServerValidator.validate(project)
        self.assertFalse(valid)
        self.assertEqual(1, len(results))
        self.assertEqual(
            QgsProjectServerValidator.ValidationError.DuplicatedNames, results[0].error
        )

        # Valid
        group.setCustomProperty("wmsShortName", "my_group1")
        valid, results = QgsProjectServerValidator.validate(project)
        self.assertTrue(valid)
        self.assertEqual(0, len(results))

        # Not valid, the project title is invalid
        project.setTitle("@ layer 1")
        valid, results = QgsProjectServerValidator.validate(project)
        self.assertFalse(valid)
        self.assertEqual(1, len(results))
        self.assertEqual(
            QgsProjectServerValidator.ValidationError.ProjectShortName, results[0].error
        )

        # Valid project title
        project.setTitle("project_title")
        valid, results = QgsProjectServerValidator.validate(project)
        self.assertTrue(valid)
        self.assertEqual(0, len(results))

        # Valid despite the bad project title, use project short name
        project.setTitle("@ layer 1")
        project.writeEntry("WMSRootName", "/", "project_short_name")
        valid, results = QgsProjectServerValidator.validate(project)
        self.assertTrue(valid)
        self.assertEqual(0, len(results))

        # Not valid project short name
        project.setTitle("project_title")
        project.writeEntry("WMSRootName", "/", "project with space")
        valid, results = QgsProjectServerValidator.validate(project)
        self.assertFalse(valid)
        self.assertEqual(1, len(results))
        self.assertEqual(
            QgsProjectServerValidator.ValidationError.ProjectShortName, results[0].error
        )

        # Not valid, duplicated project short name
        project.writeEntry("WMSRootName", "/", "layer_1")
        valid, results = QgsProjectServerValidator.validate(project)
        self.assertEqual(1, len(results))
        self.assertEqual(
            QgsProjectServerValidator.ValidationError.ProjectRootNameConflict,
            results[0].error,
        )


if __name__ == "__main__":
    unittest.main()
