"""QGIS Unit tests for QgsJsonEdit

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Damiano Lombardi"
__date__ = "2021-05-10"
__copyright__ = "Copyright 2021, The QGIS Project"


from qgis.gui import QgsJsonEditWidget
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsJsonEdit(QgisTestCase):

    def testSettersGetters(self):
        """test widget handling of null values"""
        w = QgsJsonEditWidget()

        jsonText = '{"someText": "JSON edit widget test"}'

        w.setJsonText(jsonText)
        self.assertEqual(w.jsonText(), jsonText)


if __name__ == "__main__":
    unittest.main()
