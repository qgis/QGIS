"""QGIS Unit tests for QgsSymbolLayerUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Stefanos Natsis"
__date__ = "2024-01"
__copyright__ = "Copyright 2024, The QGIS Project"

import os

from qgis.PyQt.QtCore import QDir, QMimeData, QPointF, QSize, QSizeF, Qt
from qgis.PyQt.QtXml import QDomDocument, QDomElement
from qgis.PyQt.QtGui import QColor, QImage, QPolygonF
from qgis.core import (
    QgsAbstractMeshLayerLabeling,
    QgsCoordinateReferenceSystem,
    QgsFontUtils,
    QgsMapSettings,
    QgsMeshLayer,
    QgsMeshLayerSimpleLabeling,
)
import unittest
from utilities import unitTestDataPath
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsMeshLayerLabeling(QgisTestCase):

    @classmethod
    def control_path_prefix(cls):
        return "mesh_labeling"

    def testSimpleLabelVertices(self):
        ml = QgsMeshLayer(
            os.path.join(unitTestDataPath(), "mesh", "quad_flower.2dm"), "mdal", "mdal"
        )
        self.assertTrue(ml.isValid())
        ml.setCrs(QgsCoordinateReferenceSystem("EPSG:3857"))

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(400, 400))
        ms.setOutputDpi(96)
        ms.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        ms.setExtent(ml.extent().buffered(200))
        ms.setLayers([ml])

        s = QgsAbstractMeshLayerLabeling.defaultSettingsForLayer(ml)
        s.fieldName = "$vertex_index"
        s.isExpression = True
        f = s.format()
        f.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        f.setSize(20)
        f.buffer().setEnabled(True)
        s.setFormat(f)

        ml.setLabelsEnabled(True)
        # Label on vertices:
        ml.setLabeling(QgsMeshLayerSimpleLabeling(s, False))

        self.assertTrue(
            self.render_map_settings_check(
                "simple_label_vertices", "simple_label_vertices", ms
            )
        )

        ml.setLabelsEnabled(False)

        self.assertTrue(
            self.render_map_settings_check(
                "simple_label_disabled", "simple_label_disabled", ms
            )
        )

    def testSimpleLabelFaces(self):
        ml = QgsMeshLayer(
            os.path.join(unitTestDataPath(), "mesh", "quad_flower.2dm"), "mdal", "mdal"
        )
        self.assertTrue(ml.isValid())
        ml.setCrs(QgsCoordinateReferenceSystem("EPSG:3857"))

        ms = QgsMapSettings()
        ms.setOutputSize(QSize(400, 400))
        ms.setOutputDpi(96)
        ms.setDestinationCrs(QgsCoordinateReferenceSystem("EPSG:3857"))
        ms.setExtent(ml.extent().buffered(200))
        ms.setLayers([ml])

        s = QgsAbstractMeshLayerLabeling.defaultSettingsForLayer(ml)
        s.fieldName = "$face_index"
        s.isExpression = True
        f = s.format()
        f.setFont(QgsFontUtils.getStandardTestFont("Bold"))
        f.setSize(20)
        f.buffer().setEnabled(True)
        s.setFormat(f)

        ml.setLabelsEnabled(True)
        # Label on faces:
        ml.setLabeling(QgsMeshLayerSimpleLabeling(s, True))

        self.assertTrue(
            self.render_map_settings_check(
                "simple_label_faces", "simple_label_faces", ms
            )
        )

        ml.setLabelsEnabled(False)

        self.assertTrue(
            self.render_map_settings_check(
                "simple_label_disabled", "simple_label_disabled", ms
            )
        )


if __name__ == "__main__":
    unittest.main()
