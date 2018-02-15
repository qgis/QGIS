# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsEditFormConfig.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '11/04/2017'
__copyright__ = 'Copyright 2018, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsVectorLayer,
                       QgsReadWriteContext)
from qgis.gui import QgsGui

from qgis.testing import start_app, unittest
from qgis.PyQt.QtXml import QDomDocument, QDomElement

start_app()


class TestQgsEditFormConfig(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        QgsGui.editorWidgetRegistry().initEditors()

    def createLayer(self):
        self.layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer",
                                    "addfeat", "memory")
        return self.layer

    def testReadWriteXml(self):
        layer = self.createLayer()
        config = layer.editFormConfig()

        config.setReadOnly(0, True)
        config.setReadOnly(1, False)
        config.setLabelOnTop(0, False)
        config.setLabelOnTop(1, True)

        doc = QDomDocument("testdoc")
        elem = doc.createElement('edit')
        config.writeXml(elem, QgsReadWriteContext())

        layer2 = self.createLayer()
        config2 = layer2.editFormConfig()
        config2.readXml(elem, QgsReadWriteContext())

        self.assertTrue(config2.readOnly(0))
        self.assertFalse(config2.readOnly(1))
        self.assertFalse(config2.labelOnTop(0))
        self.assertTrue(config2.labelOnTop(1))

    def testReadOnly(self):
        layer = self.createLayer()
        config = layer.editFormConfig()

        # safety checks
        config.setReadOnly(-1, True)
        config.setReadOnly(100, True)

        # real checks
        config.setReadOnly(0, True)
        config.setReadOnly(1, True)
        self.assertTrue(config.readOnly(0))
        self.assertTrue(config.readOnly(1))

        config.setReadOnly(0, False)
        config.setReadOnly(1, False)
        self.assertFalse(config.readOnly(0))
        self.assertFalse(config.readOnly(1))

    def testLabelOnTop(self):
        layer = self.createLayer()
        config = layer.editFormConfig()

        # safety checks
        config.setLabelOnTop(-1, True)
        config.setLabelOnTop(100, True)

        # real checks
        config.setLabelOnTop(0, True)
        config.setLabelOnTop(1, True)
        self.assertTrue(config.labelOnTop(0))
        self.assertTrue(config.labelOnTop(1))

        config.setLabelOnTop(0, False)
        config.setLabelOnTop(1, False)
        self.assertFalse(config.labelOnTop(0))
        self.assertFalse(config.labelOnTop(1))


if __name__ == '__main__':
    unittest.main()
