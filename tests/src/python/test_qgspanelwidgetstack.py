# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsPanelWidgetStack.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '05/10/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'

import qgis  # NOQA

from qgis.gui import QgsPanelWidget, QgsPanelWidgetStack
from qgis.testing import start_app, unittest
from qgis.PyQt.QtTest import QSignalSpy

start_app()


class TestQgsPanelWidgetStack(unittest.TestCase):

    def testMainPanel(self):
        """ test mainPanel methods """

        s = QgsPanelWidgetStack()

        # no main panel
        self.assertFalse(s.mainPanel())
        self.assertFalse(s.takeMainPanel())

        # set main panel
        p1 = QgsPanelWidget()
        s.setMainPanel(p1)
        self.assertEqual(s.mainPanel(), p1)

        # takeMainPanel()
        self.assertEqual(s.takeMainPanel(), p1)
        self.assertFalse(s.mainPanel())
        self.assertFalse(s.takeMainPanel())

    def testAddingPanels(self):
        """ test adding panels to stack """

        s = QgsPanelWidgetStack()
        mp = QgsPanelWidget()
        s.setMainPanel(mp)

        p1 = QgsPanelWidget()
        s.showPanel(p1)
        self.assertEqual(s.currentPanel(), p1)

        p2 = QgsPanelWidget()
        s.showPanel(p2)
        self.assertEqual(s.currentPanel(), p2)

    def testAcceptCurrentPanel(self):
        """ test accepting current panel """

        s = QgsPanelWidgetStack()
        # call on empty stack
        s.acceptCurrentPanel()

        mp = QgsPanelWidget()
        s.setMainPanel(mp)
        # call on main panel - should be no effect
        s.acceptCurrentPanel()
        self.assertEqual(s.mainPanel(), mp)
        self.assertEqual(s.currentPanel(), mp)

        # add panels
        p1 = QgsPanelWidget()
        s.showPanel(p1)
        p2 = QgsPanelWidget()
        s.showPanel(p2)

        # accept them
        self.assertEqual(s.currentPanel(), p2)
        p2_accept_spy = QSignalSpy(p2.panelAccepted)
        s.acceptCurrentPanel()
        self.assertEqual(s.currentPanel(), p1)
        self.assertEqual(len(p2_accept_spy), 1)
        p1_accept_spy = QSignalSpy(p1.panelAccepted)
        s.acceptCurrentPanel()
        self.assertEqual(s.currentPanel(), mp)
        self.assertEqual(len(p1_accept_spy), 1)

    def testAcceptAllPanel(self):
        """ test accepting all panels """
        s = QgsPanelWidgetStack()
        # call on empty stack
        s.acceptAllPanels()

        mp = QgsPanelWidget()
        s.setMainPanel(mp)
        # call on main panel - should be no effect
        s.acceptAllPanels()
        self.assertEqual(s.mainPanel(), mp)
        self.assertEqual(s.currentPanel(), mp)

        # add panels
        p1 = QgsPanelWidget()
        s.showPanel(p1)
        p1_accept_spy = QSignalSpy(p1.panelAccepted)
        p2 = QgsPanelWidget()
        s.showPanel(p2)
        p2_accept_spy = QSignalSpy(p2.panelAccepted)
        p3 = QgsPanelWidget()
        s.showPanel(p3)
        p3_accept_spy = QSignalSpy(p3.panelAccepted)

        # accept all
        s.acceptAllPanels()
        self.assertEqual(s.currentPanel(), mp)
        self.assertEqual(len(p1_accept_spy), 1)
        self.assertEqual(len(p2_accept_spy), 1)
        self.assertEqual(len(p3_accept_spy), 1)

    def testClear(self):
        """ test clearing stack """
        s = QgsPanelWidgetStack()
        # call on empty stack
        s.clear()

        # add panels
        mp = QgsPanelWidget()
        s.setMainPanel(mp)
        p1 = QgsPanelWidget()
        s.showPanel(p1)
        p2 = QgsPanelWidget()
        s.showPanel(p2)
        p3 = QgsPanelWidget()
        s.showPanel(p3)

        # clear
        s.clear()
        self.assertFalse(s.currentPanel())
        self.assertFalse(s.mainPanel())

    def testTakeMainAcceptsAll(self):
        """ test that taking the main panel accepts all open child panels"""
        s = QgsPanelWidgetStack()
        mp = QgsPanelWidget()
        s.setMainPanel(mp)
        p1 = QgsPanelWidget()
        s.showPanel(p1)
        p1_accept_spy = QSignalSpy(p1.panelAccepted)
        p2 = QgsPanelWidget()
        s.showPanel(p2)
        p2_accept_spy = QSignalSpy(p2.panelAccepted)
        p3 = QgsPanelWidget()
        s.showPanel(p3)
        p3_accept_spy = QSignalSpy(p3.panelAccepted)

        # take main
        s.takeMainPanel()
        self.assertEqual(len(p1_accept_spy), 1)
        self.assertEqual(len(p2_accept_spy), 1)
        self.assertEqual(len(p3_accept_spy), 1)


if __name__ == '__main__':
    unittest.main()
