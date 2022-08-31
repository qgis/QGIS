# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsUnsetAttributeValue.

From build dir, run: ctest -R PyQgsUnsetAttributeValue -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2020 by Nyall Dawson'
__date__ = '29/07/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA
from qgis.PyQt.QtCore import (QSize,
                              QDir,
                              QTemporaryDir)
from qgis.PyQt.QtGui import (QImage,
                             QPainter,
                             QColor)
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsUnsetAttributeValue,
    QgsFeature
)
from qgis.testing import start_app, unittest

from utilities import unitTestDataPath, compareWkt

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsUnsetAttributeValue(unittest.TestCase):

    def testClause(self):
        value = QgsUnsetAttributeValue()
        self.assertFalse(value.defaultValueClause())
        value = QgsUnsetAttributeValue('Autonumber')
        self.assertEqual(value.defaultValueClause(), 'Autonumber')

    def testRepr(self):
        value = QgsUnsetAttributeValue()
        self.assertEqual(str(value), '<QgsUnsetAttributeValue>')
        value = QgsUnsetAttributeValue('Autonumber')
        self.assertEqual(str(value), '<QgsUnsetAttributeValue: Autonumber>')

    def testEquality(self):
        # we don't care about the default clause when comparing equality!
        self.assertEqual(QgsUnsetAttributeValue('Autonumber'), QgsUnsetAttributeValue('Autonumber2'))
        self.assertEqual(QgsUnsetAttributeValue('Autonumber'), QgsUnsetAttributeValue())
        self.assertEqual(QgsUnsetAttributeValue(), QgsUnsetAttributeValue('Autonumber2'))

        # some deeper tests...
        self.assertEqual([1, 2, QgsUnsetAttributeValue(), 3], [1, 2, QgsUnsetAttributeValue(), 3])
        self.assertEqual([1, 2, QgsUnsetAttributeValue(), 3], [1, 2, QgsUnsetAttributeValue('Autogenerate'), 3])

        # requires fixes in QgsFeature!
        # feature = QgsFeature()
        # feature.setAttributes([1, 2, QgsUnsetAttributeValue(), 3])
        # feature2 = QgsFeature()
        # feature2.setAttributes([1, 2, QgsUnsetAttributeValue('Autogenerate'), 3])
        # self.assertEqual(feature.attributes(), feature2.attributes())
        # self.assertEqual(feature, feature2)

    def testEqualityToString(self):
        self.assertEqual(QgsUnsetAttributeValue('Autonumber'), 'Autonumber')
        self.assertNotEqual(QgsUnsetAttributeValue('Autonumberx'), 'Autonumber')
        self.assertNotEqual(QgsUnsetAttributeValue('Autonumber'), '')
        self.assertNotEqual(QgsUnsetAttributeValue(''), 'Autonumber')

        self.assertEqual('Autonumber', QgsUnsetAttributeValue('Autonumber'))
        self.assertNotEqual('Autonumber', QgsUnsetAttributeValue('Autonumberx'))
        self.assertNotEqual('', QgsUnsetAttributeValue('Autonumber'))
        self.assertNotEqual('Autonumber', QgsUnsetAttributeValue(''))


if __name__ == '__main__':
    unittest.main()
