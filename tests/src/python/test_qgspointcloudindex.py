"""
***************************************************************************
    test_qgspointcloudindex.py
    ---------------------
    Date                 : November 2024
    Copyright            : (C) 2024 by David Koňařík
    Email                : dvdkon at konarici dot cz
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

from qgis.core import (
    Qgis,
    QgsPointCloudLayer,
    QgsPointCloudNodeId,
    QgsProviderRegistry,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()


class TestQgsPointCloudIndex(QgisTestCase):

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testIndex(self):
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/sunshine-coast/ept.json",
            "test",
            "ept",
        )
        self.assertTrue(layer.isValid())

        index = layer.dataProvider().index()
        self.assertTrue(bool(index))
        self.assertTrue(index.isValid())

        self.assertEqual(index.accessType(), Qgis.PointCloudAccessType.Local)

        root = index.getNode(index.root())
        self.assertEqual(root.id(), QgsPointCloudNodeId(0, 0, 0, 0))


if __name__ == "__main__":
    unittest.main()
