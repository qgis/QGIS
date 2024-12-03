"""QGIS Unit tests for QgsPointCloudAttributeModel

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "09/11/2020"
__copyright__ = "Copyright 2020, The QGIS Project"

from qgis.PyQt.QtCore import Qt
from qgis.core import (
    QgsPointCloudAttribute,
    QgsPointCloudAttributeCollection,
    QgsPointCloudAttributeModel,
    QgsPointCloudAttributeProxyModel,
    QgsPointCloudLayer,
    QgsProviderRegistry,
)
import unittest
from qgis.testing import start_app, QgisTestCase

from utilities import unitTestDataPath

start_app()


def create_attributes():
    collection = QgsPointCloudAttributeCollection()
    collection.push_back(
        QgsPointCloudAttribute("x", QgsPointCloudAttribute.DataType.Float)
    )
    collection.push_back(
        QgsPointCloudAttribute("y", QgsPointCloudAttribute.DataType.Float)
    )
    collection.push_back(
        QgsPointCloudAttribute("z", QgsPointCloudAttribute.DataType.Float)
    )
    collection.push_back(
        QgsPointCloudAttribute("cat", QgsPointCloudAttribute.DataType.Char)
    )
    collection.push_back(
        QgsPointCloudAttribute("red", QgsPointCloudAttribute.DataType.Int32)
    )
    return collection


class TestQgsFieldModel(QgisTestCase):

    def testGettersSetters(self):
        """test model getters/setters"""
        m = QgsPointCloudAttributeModel()
        self.assertEqual(m.attributes().count(), 0)

        m.setAllowEmptyAttributeName(True)
        self.assertTrue(m.allowEmptyAttributeName())
        m.setAllowEmptyAttributeName(False)
        self.assertFalse(m.allowEmptyAttributeName())

        attributes = create_attributes()
        m.setAttributes(attributes)
        self.assertEqual(
            [a.name() for a in m.attributes().attributes()],
            ["x", "y", "z", "cat", "red"],
        )

    def testIndexFromName(self):
        m = QgsPointCloudAttributeModel()

        i = m.indexFromName("fldtxt")
        self.assertFalse(i.isValid())

        m.setAttributes(create_attributes())
        i = m.indexFromName("fldtxt")
        self.assertFalse(i.isValid())
        i = m.indexFromName("y")
        self.assertTrue(i.isValid())
        self.assertEqual(i.row(), 1)
        i = m.indexFromName("")
        self.assertFalse(i.isValid())

        m.setAllowEmptyAttributeName(True)
        i = m.indexFromName("fldtxt")
        self.assertFalse(i.isValid())
        i = m.indexFromName("y")
        self.assertTrue(i.isValid())
        self.assertEqual(i.row(), 2)
        i = m.indexFromName("")
        self.assertTrue(i.isValid())
        self.assertEqual(i.row(), 0)

    def testRowCount(self):
        m = QgsPointCloudAttributeModel()
        self.assertEqual(m.rowCount(), 0)
        m.setAttributes(create_attributes())
        self.assertEqual(m.rowCount(), 5)
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(m.rowCount(), 6)
        m.setAllowEmptyAttributeName(False)
        self.assertEqual(m.rowCount(), 5)

    def testAttributeNameRole(self):
        m = QgsPointCloudAttributeModel()
        m.setAttributes(create_attributes())

        self.assertEqual(
            m.data(
                m.index(0, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole
            ),
            "x",
        )
        self.assertEqual(
            m.data(
                m.index(1, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole
            ),
            "y",
        )
        self.assertEqual(
            m.data(
                m.index(2, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole
            ),
            "z",
        )
        self.assertEqual(
            m.data(
                m.index(3, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole
            ),
            "cat",
        )
        self.assertEqual(
            m.data(
                m.index(4, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole
            ),
            "red",
        )
        self.assertEqual(
            m.data(
                m.index(5, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole
            ),
            None,
        )
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(
            m.data(
                m.index(0, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole
            ),
            None,
        )
        self.assertEqual(
            m.data(
                m.index(1, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole
            ),
            "x",
        )
        self.assertEqual(
            m.data(
                m.index(2, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole
            ),
            "y",
        )
        self.assertEqual(
            m.data(
                m.index(5, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole
            ),
            "red",
        )

    def testAttributeIndexRole(self):
        m = QgsPointCloudAttributeModel()
        m.setAttributes(create_attributes())

        self.assertEqual(
            m.data(
                m.index(0, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeIndexRole
            ),
            0,
        )
        self.assertEqual(
            m.data(
                m.index(1, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeIndexRole
            ),
            1,
        )
        self.assertEqual(
            m.data(
                m.index(2, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeIndexRole
            ),
            2,
        )
        self.assertEqual(
            m.data(
                m.index(3, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeIndexRole
            ),
            3,
        )
        self.assertEqual(
            m.data(
                m.index(4, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeIndexRole
            ),
            4,
        )
        self.assertEqual(
            m.data(
                m.index(5, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeIndexRole
            ),
            None,
        )
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(
            m.data(
                m.index(0, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeIndexRole
            ),
            None,
        )
        self.assertEqual(
            m.data(
                m.index(1, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeIndexRole
            ),
            0,
        )
        self.assertEqual(
            m.data(
                m.index(2, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeIndexRole
            ),
            1,
        )
        self.assertEqual(
            m.data(
                m.index(5, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeIndexRole
            ),
            4,
        )

    def testSizeRole(self):
        m = QgsPointCloudAttributeModel()
        m.setAttributes(create_attributes())

        self.assertEqual(
            m.data(
                m.index(0, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeSizeRole
            ),
            4,
        )
        self.assertEqual(
            m.data(
                m.index(1, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeSizeRole
            ),
            4,
        )
        self.assertEqual(
            m.data(
                m.index(2, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeSizeRole
            ),
            4,
        )
        self.assertEqual(
            m.data(
                m.index(3, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeSizeRole
            ),
            1,
        )
        self.assertEqual(
            m.data(
                m.index(4, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeSizeRole
            ),
            4,
        )
        self.assertEqual(
            m.data(
                m.index(5, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeSizeRole
            ),
            None,
        )
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(
            m.data(
                m.index(0, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeSizeRole
            ),
            None,
        )
        self.assertEqual(
            m.data(
                m.index(1, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeSizeRole
            ),
            4,
        )
        self.assertEqual(
            m.data(
                m.index(2, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeSizeRole
            ),
            4,
        )
        self.assertEqual(
            m.data(
                m.index(5, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeSizeRole
            ),
            4,
        )

    def testTypeRole(self):
        m = QgsPointCloudAttributeModel()
        m.setAttributes(create_attributes())

        self.assertEqual(
            m.data(
                m.index(0, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeTypeRole
            ),
            QgsPointCloudAttribute.DataType.Float,
        )
        self.assertEqual(
            m.data(
                m.index(1, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeTypeRole
            ),
            QgsPointCloudAttribute.DataType.Float,
        )
        self.assertEqual(
            m.data(
                m.index(2, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeTypeRole
            ),
            QgsPointCloudAttribute.DataType.Float,
        )
        self.assertEqual(
            m.data(
                m.index(3, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeTypeRole
            ),
            QgsPointCloudAttribute.DataType.Char,
        )
        self.assertEqual(
            m.data(
                m.index(4, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeTypeRole
            ),
            QgsPointCloudAttribute.DataType.Int32,
        )
        self.assertEqual(
            m.data(
                m.index(5, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeTypeRole
            ),
            None,
        )
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(
            m.data(
                m.index(0, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeTypeRole
            ),
            None,
        )
        self.assertEqual(
            m.data(
                m.index(1, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeTypeRole
            ),
            QgsPointCloudAttribute.DataType.Float,
        )
        self.assertEqual(
            m.data(
                m.index(2, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeTypeRole
            ),
            QgsPointCloudAttribute.DataType.Float,
        )
        self.assertEqual(
            m.data(
                m.index(5, 0), QgsPointCloudAttributeModel.FieldRoles.AttributeTypeRole
            ),
            QgsPointCloudAttribute.DataType.Int32,
        )

    def testIsEmptyRole(self):
        m = QgsPointCloudAttributeModel()
        m.setAttributes(create_attributes())

        self.assertFalse(
            m.data(m.index(0, 0), QgsPointCloudAttributeModel.FieldRoles.IsEmptyRole)
        )
        self.assertFalse(
            m.data(m.index(1, 0), QgsPointCloudAttributeModel.FieldRoles.IsEmptyRole)
        )
        self.assertFalse(
            m.data(m.index(2, 0), QgsPointCloudAttributeModel.FieldRoles.IsEmptyRole)
        )
        self.assertFalse(
            m.data(m.index(3, 0), QgsPointCloudAttributeModel.FieldRoles.IsEmptyRole)
        )
        self.assertFalse(
            m.data(m.index(4, 0), QgsPointCloudAttributeModel.FieldRoles.IsEmptyRole)
        )
        self.assertFalse(
            m.data(m.index(5, 0), QgsPointCloudAttributeModel.FieldRoles.IsEmptyRole)
        )
        m.setAllowEmptyAttributeName(True)
        self.assertTrue(
            m.data(m.index(0, 0), QgsPointCloudAttributeModel.FieldRoles.IsEmptyRole)
        )
        self.assertFalse(
            m.data(m.index(1, 0), QgsPointCloudAttributeModel.FieldRoles.IsEmptyRole)
        )
        self.assertFalse(
            m.data(m.index(2, 0), QgsPointCloudAttributeModel.FieldRoles.IsEmptyRole)
        )
        self.assertFalse(
            m.data(m.index(5, 0), QgsPointCloudAttributeModel.FieldRoles.IsEmptyRole)
        )

    def testIsNumericRole(self):
        m = QgsPointCloudAttributeModel()
        m.setAttributes(create_attributes())

        self.assertTrue(
            m.data(m.index(0, 0), QgsPointCloudAttributeModel.FieldRoles.IsNumericRole)
        )
        self.assertTrue(
            m.data(m.index(1, 0), QgsPointCloudAttributeModel.FieldRoles.IsNumericRole)
        )
        self.assertTrue(
            m.data(m.index(2, 0), QgsPointCloudAttributeModel.FieldRoles.IsNumericRole)
        )
        self.assertFalse(
            m.data(m.index(3, 0), QgsPointCloudAttributeModel.FieldRoles.IsNumericRole)
        )
        self.assertTrue(
            m.data(m.index(4, 0), QgsPointCloudAttributeModel.FieldRoles.IsNumericRole)
        )
        self.assertFalse(
            m.data(m.index(5, 0), QgsPointCloudAttributeModel.FieldRoles.IsNumericRole)
        )
        m.setAllowEmptyAttributeName(True)
        self.assertFalse(
            m.data(m.index(0, 0), QgsPointCloudAttributeModel.FieldRoles.IsNumericRole)
        )
        self.assertTrue(
            m.data(m.index(1, 0), QgsPointCloudAttributeModel.FieldRoles.IsNumericRole)
        )
        self.assertTrue(
            m.data(m.index(2, 0), QgsPointCloudAttributeModel.FieldRoles.IsNumericRole)
        )
        self.assertTrue(
            m.data(m.index(5, 0), QgsPointCloudAttributeModel.FieldRoles.IsNumericRole)
        )

    def testDisplayRole(self):
        m = QgsPointCloudAttributeModel()
        m.setAttributes(create_attributes())

        self.assertEqual(m.data(m.index(0, 0), Qt.ItemDataRole.DisplayRole), "x")
        self.assertEqual(m.data(m.index(1, 0), Qt.ItemDataRole.DisplayRole), "y")
        self.assertEqual(m.data(m.index(2, 0), Qt.ItemDataRole.DisplayRole), "z")
        self.assertEqual(m.data(m.index(3, 0), Qt.ItemDataRole.DisplayRole), "cat")
        self.assertEqual(m.data(m.index(4, 0), Qt.ItemDataRole.DisplayRole), "red")
        self.assertEqual(m.data(m.index(5, 0), Qt.ItemDataRole.DisplayRole), None)
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(m.data(m.index(0, 0), Qt.ItemDataRole.DisplayRole), None)
        self.assertEqual(m.data(m.index(1, 0), Qt.ItemDataRole.DisplayRole), "x")
        self.assertEqual(m.data(m.index(2, 0), Qt.ItemDataRole.DisplayRole), "y")
        self.assertEqual(m.data(m.index(5, 0), Qt.ItemDataRole.DisplayRole), "red")

    def testTooltip(self):
        m = QgsPointCloudAttributeModel()
        m.setAttributes(create_attributes())

        self.assertEqual(
            m.data(m.index(0, 0), Qt.ItemDataRole.ToolTipRole),
            "<b>x</b><br><font style='font-family:monospace; white-space: nowrap;'>Float</font>",
        )
        self.assertEqual(
            m.data(m.index(1, 0), Qt.ItemDataRole.ToolTipRole),
            "<b>y</b><br><font style='font-family:monospace; white-space: nowrap;'>Float</font>",
        )
        self.assertEqual(
            m.data(m.index(2, 0), Qt.ItemDataRole.ToolTipRole),
            "<b>z</b><br><font style='font-family:monospace; white-space: nowrap;'>Float</font>",
        )
        self.assertEqual(
            m.data(m.index(3, 0), Qt.ItemDataRole.ToolTipRole),
            "<b>cat</b><br><font style='font-family:monospace; white-space: nowrap;'>Character</font>",
        )
        self.assertEqual(
            m.data(m.index(4, 0), Qt.ItemDataRole.ToolTipRole),
            "<b>red</b><br><font style='font-family:monospace; white-space: nowrap;'>Integer</font>",
        )
        self.assertEqual(m.data(m.index(5, 0), Qt.ItemDataRole.ToolTipRole), None)
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(m.data(m.index(0, 0), Qt.ItemDataRole.ToolTipRole), None)
        self.assertEqual(
            m.data(m.index(1, 0), Qt.ItemDataRole.ToolTipRole),
            "<b>x</b><br><font style='font-family:monospace; white-space: nowrap;'>Float</font>",
        )
        self.assertEqual(
            m.data(m.index(2, 0), Qt.ItemDataRole.ToolTipRole),
            "<b>y</b><br><font style='font-family:monospace; white-space: nowrap;'>Float</font>",
        )
        self.assertEqual(
            m.data(m.index(5, 0), Qt.ItemDataRole.ToolTipRole),
            "<b>red</b><br><font style='font-family:monospace; white-space: nowrap;'>Integer</font>",
        )

    @unittest.skipIf(
        "ept" not in QgsProviderRegistry.instance().providerList(),
        "EPT provider not available",
    )
    def testSetLayer(self):
        m = QgsPointCloudAttributeModel()
        self.assertIsNone(m.layer())
        layer = QgsPointCloudLayer(
            unitTestDataPath() + "/point_clouds/ept/sunshine-coast/ept.json",
            "test",
            "ept",
        )
        self.assertTrue(layer.isValid())
        m.setLayer(layer)
        self.assertEqual(m.layer(), layer)
        self.assertEqual(
            [a.name() for a in m.attributes().attributes()],
            [
                "X",
                "Y",
                "Z",
                "Intensity",
                "ReturnNumber",
                "NumberOfReturns",
                "ScanDirectionFlag",
                "EdgeOfFlightLine",
                "Classification",
                "ScanAngleRank",
                "UserData",
                "PointSourceId",
                "GpsTime",
                "Red",
                "Green",
                "Blue",
            ],
        )

    def testProxyModel(self):
        m = QgsPointCloudAttributeModel()
        attributes = create_attributes()
        attributes.push_back(
            QgsPointCloudAttribute("green", QgsPointCloudAttribute.DataType.Short)
        )
        attributes.push_back(
            QgsPointCloudAttribute("intensity", QgsPointCloudAttribute.DataType.Double)
        )
        m.setAttributes(attributes)
        proxy = QgsPointCloudAttributeProxyModel(m)

        self.assertEqual(proxy.rowCount(), 7)
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "x",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "y",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(2, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "z",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(3, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "cat",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(4, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "red",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(5, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "green",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(6, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "intensity",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(7, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            None,
        )
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(proxy.rowCount(), 8)
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            None,
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "x",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(2, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "y",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(7, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "intensity",
        )

        m.setAllowEmptyAttributeName(False)
        proxy.setFilters(QgsPointCloudAttributeProxyModel.Filter.Char)
        self.assertEqual(proxy.rowCount(), 1)
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "cat",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            None,
        )
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(proxy.rowCount(), 2)
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            None,
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "cat",
        )

        m.setAllowEmptyAttributeName(False)
        proxy.setFilters(QgsPointCloudAttributeProxyModel.Filter.Short)
        self.assertEqual(proxy.rowCount(), 1)
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "green",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            None,
        )
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(proxy.rowCount(), 2)
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            None,
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "green",
        )

        m.setAllowEmptyAttributeName(False)
        proxy.setFilters(QgsPointCloudAttributeProxyModel.Filter.Int32)
        self.assertEqual(proxy.rowCount(), 1)
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "red",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            None,
        )
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(proxy.rowCount(), 2)
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            None,
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "red",
        )

        m.setAllowEmptyAttributeName(False)
        proxy.setFilters(QgsPointCloudAttributeProxyModel.Filter.Float)
        self.assertEqual(proxy.rowCount(), 3)
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "x",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "y",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(2, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "z",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(3, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            None,
        )
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(proxy.rowCount(), 4)
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            None,
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "x",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(2, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "y",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(3, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "z",
        )

        m.setAllowEmptyAttributeName(False)
        proxy.setFilters(QgsPointCloudAttributeProxyModel.Filter.Double)
        self.assertEqual(proxy.rowCount(), 1)
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "intensity",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            None,
        )
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(proxy.rowCount(), 2)
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            None,
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "intensity",
        )

        m.setAllowEmptyAttributeName(False)
        proxy.setFilters(
            QgsPointCloudAttributeProxyModel.Filter.Double
            | QgsPointCloudAttributeProxyModel.Filter.Int32
        )
        self.assertEqual(proxy.rowCount(), 2)
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "red",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "intensity",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(2, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            None,
        )
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(proxy.rowCount(), 3)
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            None,
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "red",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(2, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "intensity",
        )

        m.setAllowEmptyAttributeName(False)
        proxy.setFilters(QgsPointCloudAttributeProxyModel.Filter.Numeric)
        self.assertEqual(proxy.rowCount(), 6)
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "x",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "y",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(2, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "z",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(3, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "red",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(4, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "green",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(5, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "intensity",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(6, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            None,
        )
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(proxy.rowCount(), 7)
        self.assertEqual(
            proxy.data(
                proxy.index(0, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            None,
        )
        self.assertEqual(
            proxy.data(
                proxy.index(1, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "x",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(2, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "y",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(3, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "z",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(4, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "red",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(5, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "green",
        )
        self.assertEqual(
            proxy.data(
                proxy.index(6, 0),
                QgsPointCloudAttributeModel.FieldRoles.AttributeNameRole,
            ),
            "intensity",
        )


if __name__ == "__main__":
    unittest.main()
