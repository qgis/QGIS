# -*- coding: utf-8 -*-
"""Contains tests which reveal broken behavior in QGIS.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '10/08/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

from qgis.PyQt.QtCore import QEventLoop, QT_VERSION
from qgis.core import QgsDataCollectionItem, QgsLayerItem
from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

app = start_app()
TEST_DATA_DIR = unitTestDataPath()


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


"""
This file contains tests which reveal actual broken behavior in QGIS, where the fix for the
underlying issue is unknown or non-trivial.

(It is not designed for broken *tests*, only for working tests which show broken behavior and
accordingly can't be run on the CI)

DO NOT ADD TESTS TO THIS FILE WITHOUT A DETAILED EXPLANATION ON WHY!!!!
"""


class TestQgsDisabledTests(unittest.TestCase):

    @unittest.skipIf(QT_VERSION >= 0x050d00, 'Crashes on newer Qt/PyQt versions')
    def testPythonCreateChildrenCalledFromCplusplus(self):
        """
        test createChildren() method implemented in Python, called from C++

        This test was originally working under Qt 5.12, but is broken on newer Qt or sip
        versions. The test currently segfaults, as the children created by the python QgsDataCollectionItem
        subclass PyQgsDataConnectionItem are immediately garbage collected.

        The SIP SIP_VIRTUAL_CATCHER_CODE in qgsdataitem.h is supposed to fix this situation by
        adding an extra reference to the returned python objects, but the lines

          // pyItem is given an extra reference which is removed when the C++ instance’s destructor is called.
          sipTransferTo( pyItem, Py_None );

        no longer have any effect and the object is still immediately deleted.

        Attempted solutions include:
        - all combinations of the existing VirtualCatcherCode with the different Factory/TransferBack annotations
        - removing the VirtualCatcherCode and replacing with raw Factory/TransferBack annotations
        - disabling the python garbage collection of the object entirely with sipTransferTo( pyItem, NULL )

        When fixed this test should be moved back to test_qgsdataitem.py
        """

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

            children = item.children()
            self.assertEqual(len(children), 2)
            self.assertEqual(children[0].name(), "name")
            self.assertEqual(children[1].name(), "name2")

            del children

            # Delete the object and make sure all deferred deletions are processed
            item.destroyed.connect(loop.quit)
            item.deleteLater()
            loop.exec_()

            # Check that the PyQgsLayerItem Python object is now destroyed
            self.assertTrue(tabSetDestroyedFlag[0])
            tabSetDestroyedFlag[0] = False


if __name__ == '__main__':
    unittest.main()
