"""QGIS Unit tests for QgsPointCloudAttributeModel

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '09/11/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA
from qgis.PyQt.QtCore import Qt
from qgis.core import (
    QgsPointCloudAttribute,
    QgsPointCloudAttributeCollection,
    QgsPointCloudAttributeModel,
    QgsPointCloudAttributeProxyModel,
    QgsPointCloudLayer,
    QgsProviderRegistry,
)
from qgis.testing import start_app, unittest

from utilities import unitTestDataPath

start_app()


def create_attributes():
    collection = QgsPointCloudAttributeCollection()
    collection.push_back(QgsPointCloudAttribute('x', QgsPointCloudAttribute.Float))
    collection.push_back(QgsPointCloudAttribute('y', QgsPointCloudAttribute.Float))
    collection.push_back(QgsPointCloudAttribute('z', QgsPointCloudAttribute.Float))
    collection.push_back(QgsPointCloudAttribute('cat', QgsPointCloudAttribute.Char))
    collection.push_back(QgsPointCloudAttribute('red', QgsPointCloudAttribute.Int32))
    return collection


class TestQgsFieldModel(unittest.TestCase):

    def testGettersSetters(self):
        """ test model getters/setters """
        m = QgsPointCloudAttributeModel()
        self.assertEqual(m.attributes().count(), 0)

        m.setAllowEmptyAttributeName(True)
        self.assertTrue(m.allowEmptyAttributeName())
        m.setAllowEmptyAttributeName(False)
        self.assertFalse(m.allowEmptyAttributeName())

        attributes = create_attributes()
        m.setAttributes(attributes)
        self.assertEqual([a.name() for a in m.attributes().attributes()], ['x', 'y', 'z', 'cat', 'red'])

    def testIndexFromName(self):
        m = QgsPointCloudAttributeModel()

        i = m.indexFromName('fldtxt')
        self.assertFalse(i.isValid())

        m.setAttributes(create_attributes())
        i = m.indexFromName('fldtxt')
        self.assertFalse(i.isValid())
        i = m.indexFromName('y')
        self.assertTrue(i.isValid())
        self.assertEqual(i.row(), 1)
        i = m.indexFromName('')
        self.assertFalse(i.isValid())

        m.setAllowEmptyAttributeName(True)
        i = m.indexFromName('fldtxt')
        self.assertFalse(i.isValid())
        i = m.indexFromName('y')
        self.assertTrue(i.isValid())
        self.assertEqual(i.row(), 2)
        i = m.indexFromName('')
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

        self.assertEqual(m.data(m.index(0, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'x')
        self.assertEqual(m.data(m.index(1, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'y')
        self.assertEqual(m.data(m.index(2, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'z')
        self.assertEqual(m.data(m.index(3, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'cat')
        self.assertEqual(m.data(m.index(4, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'red')
        self.assertEqual(m.data(m.index(5, 0), QgsPointCloudAttributeModel.AttributeNameRole), None)
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(m.data(m.index(0, 0), QgsPointCloudAttributeModel.AttributeNameRole), None)
        self.assertEqual(m.data(m.index(1, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'x')
        self.assertEqual(m.data(m.index(2, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'y')
        self.assertEqual(m.data(m.index(5, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'red')

    def testAttributeIndexRole(self):
        m = QgsPointCloudAttributeModel()
        m.setAttributes(create_attributes())

        self.assertEqual(m.data(m.index(0, 0), QgsPointCloudAttributeModel.AttributeIndexRole), 0)
        self.assertEqual(m.data(m.index(1, 0), QgsPointCloudAttributeModel.AttributeIndexRole), 1)
        self.assertEqual(m.data(m.index(2, 0), QgsPointCloudAttributeModel.AttributeIndexRole), 2)
        self.assertEqual(m.data(m.index(3, 0), QgsPointCloudAttributeModel.AttributeIndexRole), 3)
        self.assertEqual(m.data(m.index(4, 0), QgsPointCloudAttributeModel.AttributeIndexRole), 4)
        self.assertEqual(m.data(m.index(5, 0), QgsPointCloudAttributeModel.AttributeIndexRole), None)
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(m.data(m.index(0, 0), QgsPointCloudAttributeModel.AttributeIndexRole), None)
        self.assertEqual(m.data(m.index(1, 0), QgsPointCloudAttributeModel.AttributeIndexRole), 0)
        self.assertEqual(m.data(m.index(2, 0), QgsPointCloudAttributeModel.AttributeIndexRole), 1)
        self.assertEqual(m.data(m.index(5, 0), QgsPointCloudAttributeModel.AttributeIndexRole), 4)

    def testSizeRole(self):
        m = QgsPointCloudAttributeModel()
        m.setAttributes(create_attributes())

        self.assertEqual(m.data(m.index(0, 0), QgsPointCloudAttributeModel.AttributeSizeRole), 4)
        self.assertEqual(m.data(m.index(1, 0), QgsPointCloudAttributeModel.AttributeSizeRole), 4)
        self.assertEqual(m.data(m.index(2, 0), QgsPointCloudAttributeModel.AttributeSizeRole), 4)
        self.assertEqual(m.data(m.index(3, 0), QgsPointCloudAttributeModel.AttributeSizeRole), 1)
        self.assertEqual(m.data(m.index(4, 0), QgsPointCloudAttributeModel.AttributeSizeRole), 4)
        self.assertEqual(m.data(m.index(5, 0), QgsPointCloudAttributeModel.AttributeSizeRole), None)
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(m.data(m.index(0, 0), QgsPointCloudAttributeModel.AttributeSizeRole), None)
        self.assertEqual(m.data(m.index(1, 0), QgsPointCloudAttributeModel.AttributeSizeRole), 4)
        self.assertEqual(m.data(m.index(2, 0), QgsPointCloudAttributeModel.AttributeSizeRole), 4)
        self.assertEqual(m.data(m.index(5, 0), QgsPointCloudAttributeModel.AttributeSizeRole), 4)

    def testTypeRole(self):
        m = QgsPointCloudAttributeModel()
        m.setAttributes(create_attributes())

        self.assertEqual(m.data(m.index(0, 0), QgsPointCloudAttributeModel.AttributeTypeRole), QgsPointCloudAttribute.Float)
        self.assertEqual(m.data(m.index(1, 0), QgsPointCloudAttributeModel.AttributeTypeRole), QgsPointCloudAttribute.Float)
        self.assertEqual(m.data(m.index(2, 0), QgsPointCloudAttributeModel.AttributeTypeRole), QgsPointCloudAttribute.Float)
        self.assertEqual(m.data(m.index(3, 0), QgsPointCloudAttributeModel.AttributeTypeRole), QgsPointCloudAttribute.Char)
        self.assertEqual(m.data(m.index(4, 0), QgsPointCloudAttributeModel.AttributeTypeRole), QgsPointCloudAttribute.Int32)
        self.assertEqual(m.data(m.index(5, 0), QgsPointCloudAttributeModel.AttributeTypeRole), None)
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(m.data(m.index(0, 0), QgsPointCloudAttributeModel.AttributeTypeRole), None)
        self.assertEqual(m.data(m.index(1, 0), QgsPointCloudAttributeModel.AttributeTypeRole), QgsPointCloudAttribute.Float)
        self.assertEqual(m.data(m.index(2, 0), QgsPointCloudAttributeModel.AttributeTypeRole), QgsPointCloudAttribute.Float)
        self.assertEqual(m.data(m.index(5, 0), QgsPointCloudAttributeModel.AttributeTypeRole), QgsPointCloudAttribute.Int32)

    def testIsEmptyRole(self):
        m = QgsPointCloudAttributeModel()
        m.setAttributes(create_attributes())

        self.assertFalse(m.data(m.index(0, 0), QgsPointCloudAttributeModel.IsEmptyRole))
        self.assertFalse(m.data(m.index(1, 0), QgsPointCloudAttributeModel.IsEmptyRole))
        self.assertFalse(m.data(m.index(2, 0), QgsPointCloudAttributeModel.IsEmptyRole))
        self.assertFalse(m.data(m.index(3, 0), QgsPointCloudAttributeModel.IsEmptyRole))
        self.assertFalse(m.data(m.index(4, 0), QgsPointCloudAttributeModel.IsEmptyRole))
        self.assertFalse(m.data(m.index(5, 0), QgsPointCloudAttributeModel.IsEmptyRole))
        m.setAllowEmptyAttributeName(True)
        self.assertTrue(m.data(m.index(0, 0), QgsPointCloudAttributeModel.IsEmptyRole))
        self.assertFalse(m.data(m.index(1, 0), QgsPointCloudAttributeModel.IsEmptyRole))
        self.assertFalse(m.data(m.index(2, 0), QgsPointCloudAttributeModel.IsEmptyRole))
        self.assertFalse(m.data(m.index(5, 0), QgsPointCloudAttributeModel.IsEmptyRole))

    def testIsNumericRole(self):
        m = QgsPointCloudAttributeModel()
        m.setAttributes(create_attributes())

        self.assertTrue(m.data(m.index(0, 0), QgsPointCloudAttributeModel.IsNumericRole))
        self.assertTrue(m.data(m.index(1, 0), QgsPointCloudAttributeModel.IsNumericRole))
        self.assertTrue(m.data(m.index(2, 0), QgsPointCloudAttributeModel.IsNumericRole))
        self.assertFalse(m.data(m.index(3, 0), QgsPointCloudAttributeModel.IsNumericRole))
        self.assertTrue(m.data(m.index(4, 0), QgsPointCloudAttributeModel.IsNumericRole))
        self.assertFalse(m.data(m.index(5, 0), QgsPointCloudAttributeModel.IsNumericRole))
        m.setAllowEmptyAttributeName(True)
        self.assertFalse(m.data(m.index(0, 0), QgsPointCloudAttributeModel.IsNumericRole))
        self.assertTrue(m.data(m.index(1, 0), QgsPointCloudAttributeModel.IsNumericRole))
        self.assertTrue(m.data(m.index(2, 0), QgsPointCloudAttributeModel.IsNumericRole))
        self.assertTrue(m.data(m.index(5, 0), QgsPointCloudAttributeModel.IsNumericRole))

    def testDisplayRole(self):
        m = QgsPointCloudAttributeModel()
        m.setAttributes(create_attributes())

        self.assertEqual(m.data(m.index(0, 0), Qt.DisplayRole), 'x')
        self.assertEqual(m.data(m.index(1, 0), Qt.DisplayRole), 'y')
        self.assertEqual(m.data(m.index(2, 0), Qt.DisplayRole), 'z')
        self.assertEqual(m.data(m.index(3, 0), Qt.DisplayRole), 'cat')
        self.assertEqual(m.data(m.index(4, 0), Qt.DisplayRole), 'red')
        self.assertEqual(m.data(m.index(5, 0), Qt.DisplayRole), None)
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(m.data(m.index(0, 0), Qt.DisplayRole), None)
        self.assertEqual(m.data(m.index(1, 0), Qt.DisplayRole), 'x')
        self.assertEqual(m.data(m.index(2, 0), Qt.DisplayRole), 'y')
        self.assertEqual(m.data(m.index(5, 0), Qt.DisplayRole), 'red')

    def testTooltip(self):
        m = QgsPointCloudAttributeModel()
        m.setAttributes(create_attributes())

        self.assertEqual(m.data(m.index(0, 0), Qt.ToolTipRole), "<b>x</b><br><font style='font-family:monospace; white-space: nowrap;'>Float</font>")
        self.assertEqual(m.data(m.index(1, 0), Qt.ToolTipRole), "<b>y</b><br><font style='font-family:monospace; white-space: nowrap;'>Float</font>")
        self.assertEqual(m.data(m.index(2, 0), Qt.ToolTipRole), "<b>z</b><br><font style='font-family:monospace; white-space: nowrap;'>Float</font>")
        self.assertEqual(m.data(m.index(3, 0), Qt.ToolTipRole), "<b>cat</b><br><font style='font-family:monospace; white-space: nowrap;'>Character</font>")
        self.assertEqual(m.data(m.index(4, 0), Qt.ToolTipRole), "<b>red</b><br><font style='font-family:monospace; white-space: nowrap;'>Integer</font>")
        self.assertEqual(m.data(m.index(5, 0), Qt.ToolTipRole), None)
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(m.data(m.index(0, 0), Qt.ToolTipRole), None)
        self.assertEqual(m.data(m.index(1, 0), Qt.ToolTipRole), "<b>x</b><br><font style='font-family:monospace; white-space: nowrap;'>Float</font>")
        self.assertEqual(m.data(m.index(2, 0), Qt.ToolTipRole), "<b>y</b><br><font style='font-family:monospace; white-space: nowrap;'>Float</font>")
        self.assertEqual(m.data(m.index(5, 0), Qt.ToolTipRole), "<b>red</b><br><font style='font-family:monospace; white-space: nowrap;'>Integer</font>")

    @unittest.skipIf('ept' not in QgsProviderRegistry.instance().providerList(), 'EPT provider not available')
    def testSetLayer(self):
        m = QgsPointCloudAttributeModel()
        self.assertIsNone(m.layer())
        layer = QgsPointCloudLayer(unitTestDataPath() + '/point_clouds/ept/sunshine-coast/ept.json', 'test', 'ept')
        self.assertTrue(layer.isValid())
        m.setLayer(layer)
        self.assertEqual(m.layer(), layer)
        self.assertEqual([a.name() for a in m.attributes().attributes()], ['X', 'Y', 'Z', 'Intensity', 'ReturnNumber', 'NumberOfReturns', 'ScanDirectionFlag', 'EdgeOfFlightLine', 'Classification', 'ScanAngleRank', 'UserData', 'PointSourceId', 'GpsTime', 'Red', 'Green', 'Blue'])

    def testProxyModel(self):
        m = QgsPointCloudAttributeModel()
        attributes = create_attributes()
        attributes.push_back(QgsPointCloudAttribute('green', QgsPointCloudAttribute.Short))
        attributes.push_back(QgsPointCloudAttribute('intensity', QgsPointCloudAttribute.Double))
        m.setAttributes(attributes)
        proxy = QgsPointCloudAttributeProxyModel(m)

        self.assertEqual(proxy.rowCount(), 7)
        self.assertEqual(proxy.data(proxy.index(0, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'x')
        self.assertEqual(proxy.data(proxy.index(1, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'y')
        self.assertEqual(proxy.data(proxy.index(2, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'z')
        self.assertEqual(proxy.data(proxy.index(3, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'cat')
        self.assertEqual(proxy.data(proxy.index(4, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'red')
        self.assertEqual(proxy.data(proxy.index(5, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'green')
        self.assertEqual(proxy.data(proxy.index(6, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'intensity')
        self.assertEqual(proxy.data(proxy.index(7, 0), QgsPointCloudAttributeModel.AttributeNameRole), None)
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(proxy.rowCount(), 8)
        self.assertEqual(proxy.data(proxy.index(0, 0), QgsPointCloudAttributeModel.AttributeNameRole), None)
        self.assertEqual(proxy.data(proxy.index(1, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'x')
        self.assertEqual(proxy.data(proxy.index(2, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'y')
        self.assertEqual(proxy.data(proxy.index(7, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'intensity')

        m.setAllowEmptyAttributeName(False)
        proxy.setFilters(QgsPointCloudAttributeProxyModel.Char)
        self.assertEqual(proxy.rowCount(), 1)
        self.assertEqual(proxy.data(proxy.index(0, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'cat')
        self.assertEqual(proxy.data(proxy.index(1, 0), QgsPointCloudAttributeModel.AttributeNameRole), None)
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(proxy.rowCount(), 2)
        self.assertEqual(proxy.data(proxy.index(0, 0), QgsPointCloudAttributeModel.AttributeNameRole), None)
        self.assertEqual(proxy.data(proxy.index(1, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'cat')

        m.setAllowEmptyAttributeName(False)
        proxy.setFilters(QgsPointCloudAttributeProxyModel.Short)
        self.assertEqual(proxy.rowCount(), 1)
        self.assertEqual(proxy.data(proxy.index(0, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'green')
        self.assertEqual(proxy.data(proxy.index(1, 0), QgsPointCloudAttributeModel.AttributeNameRole), None)
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(proxy.rowCount(), 2)
        self.assertEqual(proxy.data(proxy.index(0, 0), QgsPointCloudAttributeModel.AttributeNameRole), None)
        self.assertEqual(proxy.data(proxy.index(1, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'green')

        m.setAllowEmptyAttributeName(False)
        proxy.setFilters(QgsPointCloudAttributeProxyModel.Int32)
        self.assertEqual(proxy.rowCount(), 1)
        self.assertEqual(proxy.data(proxy.index(0, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'red')
        self.assertEqual(proxy.data(proxy.index(1, 0), QgsPointCloudAttributeModel.AttributeNameRole), None)
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(proxy.rowCount(), 2)
        self.assertEqual(proxy.data(proxy.index(0, 0), QgsPointCloudAttributeModel.AttributeNameRole), None)
        self.assertEqual(proxy.data(proxy.index(1, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'red')

        m.setAllowEmptyAttributeName(False)
        proxy.setFilters(QgsPointCloudAttributeProxyModel.Float)
        self.assertEqual(proxy.rowCount(), 3)
        self.assertEqual(proxy.data(proxy.index(0, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'x')
        self.assertEqual(proxy.data(proxy.index(1, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'y')
        self.assertEqual(proxy.data(proxy.index(2, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'z')
        self.assertEqual(proxy.data(proxy.index(3, 0), QgsPointCloudAttributeModel.AttributeNameRole), None)
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(proxy.rowCount(), 4)
        self.assertEqual(proxy.data(proxy.index(0, 0), QgsPointCloudAttributeModel.AttributeNameRole), None)
        self.assertEqual(proxy.data(proxy.index(1, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'x')
        self.assertEqual(proxy.data(proxy.index(2, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'y')
        self.assertEqual(proxy.data(proxy.index(3, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'z')

        m.setAllowEmptyAttributeName(False)
        proxy.setFilters(QgsPointCloudAttributeProxyModel.Double)
        self.assertEqual(proxy.rowCount(), 1)
        self.assertEqual(proxy.data(proxy.index(0, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'intensity')
        self.assertEqual(proxy.data(proxy.index(1, 0), QgsPointCloudAttributeModel.AttributeNameRole), None)
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(proxy.rowCount(), 2)
        self.assertEqual(proxy.data(proxy.index(0, 0), QgsPointCloudAttributeModel.AttributeNameRole), None)
        self.assertEqual(proxy.data(proxy.index(1, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'intensity')

        m.setAllowEmptyAttributeName(False)
        proxy.setFilters(QgsPointCloudAttributeProxyModel.Double | QgsPointCloudAttributeProxyModel.Int32)
        self.assertEqual(proxy.rowCount(), 2)
        self.assertEqual(proxy.data(proxy.index(0, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'red')
        self.assertEqual(proxy.data(proxy.index(1, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'intensity')
        self.assertEqual(proxy.data(proxy.index(2, 0), QgsPointCloudAttributeModel.AttributeNameRole), None)
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(proxy.rowCount(), 3)
        self.assertEqual(proxy.data(proxy.index(0, 0), QgsPointCloudAttributeModel.AttributeNameRole), None)
        self.assertEqual(proxy.data(proxy.index(1, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'red')
        self.assertEqual(proxy.data(proxy.index(2, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'intensity')

        m.setAllowEmptyAttributeName(False)
        proxy.setFilters(QgsPointCloudAttributeProxyModel.Numeric)
        self.assertEqual(proxy.rowCount(), 6)
        self.assertEqual(proxy.data(proxy.index(0, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'x')
        self.assertEqual(proxy.data(proxy.index(1, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'y')
        self.assertEqual(proxy.data(proxy.index(2, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'z')
        self.assertEqual(proxy.data(proxy.index(3, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'red')
        self.assertEqual(proxy.data(proxy.index(4, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'green')
        self.assertEqual(proxy.data(proxy.index(5, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'intensity')
        self.assertEqual(proxy.data(proxy.index(6, 0), QgsPointCloudAttributeModel.AttributeNameRole), None)
        m.setAllowEmptyAttributeName(True)
        self.assertEqual(proxy.rowCount(), 7)
        self.assertEqual(proxy.data(proxy.index(0, 0), QgsPointCloudAttributeModel.AttributeNameRole), None)
        self.assertEqual(proxy.data(proxy.index(1, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'x')
        self.assertEqual(proxy.data(proxy.index(2, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'y')
        self.assertEqual(proxy.data(proxy.index(3, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'z')
        self.assertEqual(proxy.data(proxy.index(4, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'red')
        self.assertEqual(proxy.data(proxy.index(5, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'green')
        self.assertEqual(proxy.data(proxy.index(6, 0), QgsPointCloudAttributeModel.AttributeNameRole), 'intensity')


if __name__ == '__main__':
    unittest.main()
