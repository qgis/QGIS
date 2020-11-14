# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsDataItem

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Even Rouault'
__date__ = '14/11/2020 late in the night'
__copyright__ = 'Copyright 2020, The QGIS Project'

from qgis.PyQt.QtCore import QEventLoop
from qgis.core import QgsDataCollectionItem, QgsLayerItem

from qgis.testing import start_app, unittest

app = start_app()


class PyQgsLayerItem(QgsLayerItem):

    def __del__(self):
        self.tabSetDestroyedFlag[0] = True


class PyQgsDataConnectionItem(QgsDataCollectionItem):

    def createChildren(self):
        children = []

        # Add a Python object as child
        pyQgsLayerItem = PyQgsLayerItem(None, "name", "", "uri", QgsLayerItem.Vector, "my_provider")
        pyQgsLayerItem.tabSetDestroyedFlag = self.tabSetDestroyedFlag
        children.append(pyQgsLayerItem)

        # Add a C++ object as child
        children.append(QgsLayerItem(None, "name2", "", "uri", QgsLayerItem.Vector, "my_provider"))

        return children


class TestQgsDataItem(unittest.TestCase):

    def testPythonCreateChildrenCalledFromCplusplus(self):
        """ test createChildren() method implemented in Python, called from C++ """

        loop = QEventLoop()
        NUM_ITERS = 10  # set more to detect memory leaks
        for i in range(NUM_ITERS):
            tabSetDestroyedFlag = [False]

            item = PyQgsDataConnectionItem(None, "name", "", "my_provider")
            item.tabSetDestroyedFlag = tabSetDestroyedFlag

            # Causes PyQgsDataConnectionItem.createChildren() to be called
            item.populate()

            # wait for populate() to have done its job
            item.stateChanged.connect(loop.quit)
            loop.exec_()

            # Python object PyQgsLayerItem should still be alive
            self.assertFalse(tabSetDestroyedFlag[0])

            self.assertEqual(len(item.children()), 2)
            self.assertEqual(item.children()[0].name(), "name")
            self.assertEqual(item.children()[1].name(), "name2")

            # Delete the object and make sure all deferred deletions are processed
            item.destroyed.connect(loop.quit)
            item.deleteLater()
            loop.exec_()

            # Check that the PyQgsLayerItem Python object is now destroyed
            self.assertTrue(tabSetDestroyedFlag[0])
            tabSetDestroyedFlag[0] = False


if __name__ == '__main__':
    unittest.main()
