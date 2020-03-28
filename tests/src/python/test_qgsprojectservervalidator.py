"""QGIS Unit tests for QgsProjectServerValidator

From build dir, run:
ctest -R PyQgsProjectServerValidator -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Etienne Trimaille'
__date__ = '27/03/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

from qgis.core import (
    QgsProject,
    QgsVectorLayer,
    QgsProjectServerValidator,
)
from qgis.testing import start_app, unittest

app = start_app()


class TestQgsprojectServerValidator(unittest.TestCase):

    def test_project_server_validator(self):
        """Test project server validator."""
        validator = QgsProjectServerValidator()
        project = QgsProject()
        layer = QgsVectorLayer("Point?field=fldtxt:string", "layer_1", "memory")
        project.addMapLayers([layer])

        valid, results = validator.validate(project.layerTreeRoot())
        self.assertTrue(valid)
        self.assertFalse(results)

        layer_1 = QgsVectorLayer("Point?field=fldtxt:string", "layer_1", "memory")
        project.addMapLayers([layer_1])

        valid, results = validator.validate(project.layerTreeRoot())
        self.assertFalse(valid)
        self.assertEqual(results[0].section, 'Duplicated names')


if __name__ == '__main__':
    unittest.main()
