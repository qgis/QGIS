"""QGIS Unit tests for QgsProviderUtils.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "30/06/2021"
__copyright__ = "Copyright 2021, The QGIS Project"

from qgis.core import (
    Qgis,
    QgsMapLayerType,
    QgsProviderRegistry,
    QgsProviderSublayerDetails,
    QgsProviderUtils,
    QgsWkbTypes,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

app = start_app()


class TestQgsProviderUtils(QgisTestCase):

    def test_sublayerDetailsAreIncomplete(self):
        """
        Test sublayerDetailsAreIncomplete
        """
        uri = unitTestDataPath() + "/mixed_types.TAB"

        # surface scan only
        sublayers = QgsProviderRegistry.instance().querySublayers(uri)
        self.assertEqual(len(sublayers), 1)
        self.assertEqual(sublayers[0].wkbType(), QgsWkbTypes.Type.Unknown)

        # need to resolve geometry types for complete details about this uri!
        self.assertTrue(QgsProviderUtils.sublayerDetailsAreIncomplete(sublayers))
        self.assertTrue(
            QgsProviderUtils.sublayerDetailsAreIncomplete(
                sublayers,
                QgsProviderUtils.SublayerCompletenessFlags(
                    QgsProviderUtils.SublayerCompletenessFlag.IgnoreUnknownFeatureCount
                ),
            )
        )
        self.assertTrue(
            QgsProviderUtils.sublayerDetailsAreIncomplete(
                sublayers,
                QgsProviderUtils.SublayerCompletenessFlags(
                    QgsProviderUtils.SublayerCompletenessFlag.IgnoreUnknownGeometryType
                ),
            )
        )
        # ...unless we are ignoring both unknown feature count and unknown geometry types
        self.assertFalse(
            QgsProviderUtils.sublayerDetailsAreIncomplete(
                sublayers,
                QgsProviderUtils.SublayerCompletenessFlags(
                    QgsProviderUtils.SublayerCompletenessFlag.IgnoreUnknownFeatureCount
                    | QgsProviderUtils.SublayerCompletenessFlag.IgnoreUnknownGeometryType
                ),
            )
        )

        # fake feature count, now we have complete details if we ignore unknown geometry type
        sublayers[0].setFeatureCount(5)
        self.assertFalse(
            QgsProviderUtils.sublayerDetailsAreIncomplete(
                sublayers,
                QgsProviderUtils.SublayerCompletenessFlags(
                    QgsProviderUtils.SublayerCompletenessFlag.IgnoreUnknownGeometryType
                ),
            )
        )

        # retry with retrieving geometry types
        sublayers = QgsProviderRegistry.instance().querySublayers(
            uri, Qgis.SublayerQueryFlag.ResolveGeometryType
        )
        # now we have all the details
        self.assertEqual(len(sublayers), 3)
        self.assertFalse(QgsProviderUtils.sublayerDetailsAreIncomplete(sublayers))
        self.assertFalse(
            QgsProviderUtils.sublayerDetailsAreIncomplete(
                sublayers,
                QgsProviderUtils.SublayerCompletenessFlags(
                    QgsProviderUtils.SublayerCompletenessFlag.IgnoreUnknownFeatureCount
                ),
            )
        )
        self.assertFalse(
            QgsProviderUtils.sublayerDetailsAreIncomplete(
                sublayers,
                QgsProviderUtils.SublayerCompletenessFlags(
                    QgsProviderUtils.SublayerCompletenessFlag.IgnoreUnknownGeometryType
                ),
            )
        )

        # this geopackage file requires manually requesting feature counts
        uri = unitTestDataPath() + "/mixed_layers.gpkg"

        # surface scan only
        sublayers = QgsProviderRegistry.instance().querySublayers(uri)
        self.assertEqual(len(sublayers), 4)
        self.assertEqual(sublayers[0].name(), "band1")
        self.assertEqual(sublayers[1].name(), "band2")
        self.assertEqual(sublayers[2].name(), "points")
        self.assertEqual(sublayers[2].featureCount(), Qgis.FeatureCountState.Uncounted)
        self.assertEqual(sublayers[3].name(), "lines")
        self.assertEqual(sublayers[3].featureCount(), Qgis.FeatureCountState.Uncounted)

        # need to count features for complete details about this uri!
        self.assertTrue(QgsProviderUtils.sublayerDetailsAreIncomplete(sublayers))
        self.assertTrue(
            QgsProviderUtils.sublayerDetailsAreIncomplete(
                sublayers,
                QgsProviderUtils.SublayerCompletenessFlags(
                    QgsProviderUtils.SublayerCompletenessFlag.IgnoreUnknownGeometryType
                ),
            )
        )
        # ...unless we are ignoring unknown feature counts, that is...
        self.assertFalse(
            QgsProviderUtils.sublayerDetailsAreIncomplete(
                sublayers,
                QgsProviderUtils.SublayerCompletenessFlags(
                    QgsProviderUtils.SublayerCompletenessFlag.IgnoreUnknownFeatureCount
                ),
            )
        )

        # retry with retrieving feature count
        sublayers = QgsProviderRegistry.instance().querySublayers(
            uri, Qgis.SublayerQueryFlag.CountFeatures
        )
        # now we have all the details
        self.assertEqual(len(sublayers), 4)
        self.assertFalse(
            QgsProviderUtils.sublayerDetailsAreIncomplete(
                sublayers,
                QgsProviderUtils.SublayerCompletenessFlags(
                    QgsProviderUtils.SublayerCompletenessFlag.IgnoreUnknownFeatureCount
                ),
            )
        )
        self.assertFalse(
            QgsProviderUtils.sublayerDetailsAreIncomplete(
                sublayers,
                QgsProviderUtils.SublayerCompletenessFlags(
                    QgsProviderUtils.SublayerCompletenessFlag.IgnoreUnknownGeometryType
                ),
            )
        )
        self.assertFalse(QgsProviderUtils.sublayerDetailsAreIncomplete(sublayers))
        self.assertEqual(sublayers[0].name(), "band1")
        self.assertEqual(sublayers[1].name(), "band2")
        self.assertEqual(sublayers[2].name(), "points")
        self.assertEqual(sublayers[2].featureCount(), 0)
        self.assertEqual(sublayers[3].name(), "lines")
        self.assertEqual(sublayers[3].featureCount(), 6)

        # test with sublayer with skippedContainerScan flag
        sl1 = QgsProviderSublayerDetails()
        sl1.setProviderKey("ogr")
        sl1.setType(QgsMapLayerType.VectorLayer)
        sl1.setWkbType(QgsWkbTypes.Type.Point)
        sl1.setFeatureCount(1)
        sl1.setSkippedContainerScan(False)
        self.assertFalse(
            QgsProviderUtils.sublayerDetailsAreIncomplete(
                [sl1],
                QgsProviderUtils.SublayerCompletenessFlags(
                    QgsProviderUtils.SublayerCompletenessFlag.IgnoreUnknownFeatureCount
                ),
            )
        )
        self.assertFalse(QgsProviderUtils.sublayerDetailsAreIncomplete([sl1]))
        sl2 = QgsProviderSublayerDetails()
        sl2.setProviderKey("ogr")
        sl2.setType(QgsMapLayerType.VectorLayer)
        sl2.setWkbType(QgsWkbTypes.Type.Point)
        sl2.setFeatureCount(1)
        sl2.setSkippedContainerScan(True)
        self.assertTrue(
            QgsProviderUtils.sublayerDetailsAreIncomplete(
                [sl2],
                QgsProviderUtils.SublayerCompletenessFlags(
                    QgsProviderUtils.SublayerCompletenessFlag.IgnoreUnknownFeatureCount
                ),
            )
        )
        self.assertTrue(QgsProviderUtils.sublayerDetailsAreIncomplete([sl2]))
        self.assertTrue(
            QgsProviderUtils.sublayerDetailsAreIncomplete(
                [sl1, sl2],
                QgsProviderUtils.SublayerCompletenessFlags(
                    QgsProviderUtils.SublayerCompletenessFlag.IgnoreUnknownFeatureCount
                ),
            )
        )
        self.assertTrue(QgsProviderUtils.sublayerDetailsAreIncomplete([sl1, sl2]))

    def test_suggestLayerNameFromFilePath(self):
        """
        test suggestLayerNameFromFilePath
        """
        self.assertEqual(QgsProviderUtils.suggestLayerNameFromFilePath(""), "")
        self.assertEqual(
            QgsProviderUtils.suggestLayerNameFromFilePath("/home/me/data/rivers.shp"),
            "rivers",
        )
        # adf files should return parent dir name
        self.assertEqual(
            QgsProviderUtils.suggestLayerNameFromFilePath(
                "/home/me/data/rivers/hdr.adf"
            ),
            "rivers",
        )
        # ept.json files should return parent dir name
        self.assertEqual(
            QgsProviderUtils.suggestLayerNameFromFilePath(
                "/home/me/data/rivers/ept.json"
            ),
            "rivers",
        )


if __name__ == "__main__":
    unittest.main()
