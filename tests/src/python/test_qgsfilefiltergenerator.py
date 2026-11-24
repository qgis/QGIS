"""QGIS Unit tests for QgsFileFilterGenerator.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import math

from qgis.core import QgsFileFilterGenerator
from qgis.testing import unittest


class TestQgsFileFilterGenerator(unittest.TestCase):
    def test_file_filter(self):
        self.assertEqual(
            QgsFileFilterGenerator.fileFilter(
                QgsFileFilterGenerator.StandardFileFormat.QML
            ),
            "QGIS Layer Style Files (*.qml *.QML)",
        )

    def test_filter_string(self):
        self.assertEqual(
            QgsFileFilterGenerator.filterString(
                [QgsFileFilterGenerator.StandardFileFormat.QML]
            ),
            "QGIS Layer Style Files (*.qml *.QML)",
        )
        self.assertEqual(
            QgsFileFilterGenerator.filterString(
                [
                    QgsFileFilterGenerator.StandardFileFormat.QML,
                    QgsFileFilterGenerator.StandardFileFormat.SLD,
                ]
            ),
            "QGIS Layer Style Files (*.qml *.QML);;SLD Files (*.sld *.SLD)",
        )
        self.assertEqual(
            QgsFileFilterGenerator.filterString(
                [QgsFileFilterGenerator.StandardFileFormat.QML], True
            ),
            "QGIS Layer Style Files (*.qml *.QML);;All Files (*.*)",
        )
        self.assertEqual(
            QgsFileFilterGenerator.filterString(
                [
                    QgsFileFilterGenerator.StandardFileFormat.QML,
                    QgsFileFilterGenerator.StandardFileFormat.SLD,
                ],
                True,
            ),
            "QGIS Layer Style Files (*.qml *.QML);;SLD Files (*.sld *.SLD);;All Files (*.*)",
        )

        self.assertEqual(
            QgsFileFilterGenerator.filterString(
                [
                    QgsFileFilterGenerator.StandardFileFormat.QML,
                    QgsFileFilterGenerator.StandardFileFormat.AllFiles,
                    QgsFileFilterGenerator.StandardFileFormat.SLD,
                ],
                True,
            ),
            "QGIS Layer Style Files (*.qml *.QML);;SLD Files (*.sld *.SLD);;All Files (*.*)",
        )
        self.assertEqual(
            QgsFileFilterGenerator.filterString(
                [
                    QgsFileFilterGenerator.StandardFileFormat.QML,
                    QgsFileFilterGenerator.StandardFileFormat.AllFiles,
                    QgsFileFilterGenerator.StandardFileFormat.SLD,
                ]
            ),
            "QGIS Layer Style Files (*.qml *.QML);;SLD Files (*.sld *.SLD);;All Files (*.*)",
        )


if __name__ == "__main__":
    unittest.main()
