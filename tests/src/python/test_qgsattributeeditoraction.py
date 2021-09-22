# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsAttributeEditorAction

From build dir, run: ctest -R PyQgsAttributeEditorAction -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Alessandro Pasotti'
__date__ = '16/08/2021'
__copyright__ = 'Copyright 2021, The QGIS Project'

import qgis  # NOQA switch sip api

from qgis.core import (QgsVectorLayer,
                       QgsProject,
                       QgsFeature,
                       QgsAction,
                       QgsAttributeEditorAction,
                       QgsAttributeEditorContainer,
                       )
from qgis.PyQt.QtCore import QDir, QTemporaryFile, QUuid
from qgis.PyQt.QtWidgets import QPushButton, QWidget
from qgis.PyQt.QtGui import QIcon

from qgis.testing import start_app, unittest

import os
import time
import platform

start_app()

from qgis.testing import QGISAPP


class TestQgsActionWidgetWrapper(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.layer = QgsVectorLayer("Point?field=fldtxt:string&field=fldint:integer&field=flddate:datetime",
                                   "test_layer", "memory")

        QgsProject.instance().addMapLayers([cls.layer])

        cls.action_id1 = QUuid.createUuid()
        cls.action_id2 = QUuid.createUuid()
        cls.action_id3 = QUuid.createUuid()
        cls.action1 = QgsAction(cls.action_id1, QgsAction.GenericPython, 'Test Action 1 Desc', 'i=1', '', False, 'Test Action 1 Short Title')
        cls.action2 = QgsAction(cls.action_id2, QgsAction.GenericPython, 'Test Action 2 Desc', 'i=2', QGISAPP.appIconPath(), False, 'Test Action 2 Short Title')
        cls.action3 = QgsAction(cls.action_id3, QgsAction.GenericPython, 'Test Action 3 Desc', 'i=3', '', False)
        cls.layer.actions().addAction(cls.action1)
        cls.layer.actions().addAction(cls.action2)
        cls.layer.actions().addAction(cls.action3)

    @classmethod
    def tearDownClass(cls):
        cls.layer = None
        cls.manager = None

    def testEditorActionCtor(self):

        parent = QgsAttributeEditorContainer('container', None)
        editor_action = QgsAttributeEditorAction(self.action1, parent)
        self.assertEqual(QUuid(editor_action.action(self.layer).id()), self.action_id1)
        self.assertEqual(editor_action.action(self.layer).id(), self.action1.id())

        # Lazy load
        editor_action = QgsAttributeEditorAction(self.action1.id(), parent)
        self.assertEqual(QUuid(editor_action.action(self.layer).id()), self.action_id1)
        self.assertEqual(editor_action.action(self.layer).id(), self.action1.id())

    def testSetAction(self):

        parent = QgsAttributeEditorContainer('container', None)
        editor_action = QgsAttributeEditorAction(self.action1, parent)

        editor_action.setAction(self.action2)
        self.assertEqual(QUuid(editor_action.action(self.layer).id()), self.action_id2)


if __name__ == '__main__':
    unittest.main()
