"""QGIS Unit tests for QgsApplication.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Tim Sutton (tim@linfiniti.com)"
__date__ = "20/01/2011"
__copyright__ = "Copyright 2012, The QGIS Project"


import unittest

from qgis.PyQt.QtCore import QCoreApplication, QEvent
from qgis.PyQt.QtWidgets import QAction, QMenu
from qgis.testing import QgisTestCase, start_app
from qgis.testing.mocked import get_iface

QGISAPP = start_app()


class TestPyQgsApplication(QgisTestCase):
    def setUp(self):
        super().setUp()
        self.iface = get_iface()

    def testInvalidThemeName(self):
        """Check using an invalid theme will fallback to  'default'"""
        QGISAPP.setUITheme("fooobar")
        myExpectedResult = "default"
        myResult = QGISAPP.themeName()
        myMessage = f"Expected:\n{myExpectedResult}\nGot:\n{myResult}\n"
        assert myExpectedResult == myResult, myMessage

    def testMenusNotCreatedByRemove(self):
        menu_name = "&Test Menu"
        action = QAction("Temporary Action", self.iface.mainWindow())

        self.iface.removePluginDatabaseMenu(menu_name, action)
        self.iface.removePluginMenu(menu_name, action)
        self.iface.removePluginRasterMenu(menu_name, action)
        self.iface.removePluginVectorMenu(menu_name, action)
        self.iface.removePluginWebMenu(menu_name, action)
        self.iface.removePluginMeshMenu(menu_name, action)

        QCoreApplication.sendPostedEvents(None, QEvent.Type.DeferredDelete)
        QCoreApplication.processEvents()

        self.assertTrue(
            not any(
                menu
                for menu in self.iface.mainWindow().findChildren(QMenu)
                if menu.title() == menu_name
            ),
            "Menu should have been deleted",
        )


if __name__ == "__main__":
    unittest.main()
