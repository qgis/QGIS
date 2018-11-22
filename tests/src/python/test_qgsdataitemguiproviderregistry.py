# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsDataItemGuiProviderRegistry

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '27/10/2018'
__copyright__ = 'Copyright 2018, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.gui import (QgsGui,
                      QgsDataItemGuiContext,
                      QgsDataItemGuiProvider,
                      QgsDataItemGuiProviderRegistry,
                      QgsMessageBar)
from qgis.testing import start_app, unittest

app = start_app()


class TestProvider(QgsDataItemGuiProvider):

    def __init__(self, name):
        super().__init__()
        self._name = name

    def name(self):
        return self._name


class TestQgsDataItemGuiContext(unittest.TestCase):

    def testContext(self):
        context = QgsDataItemGuiContext()
        self.assertIsNone(context.messageBar())

        mb = QgsMessageBar()
        context.setMessageBar(mb)
        self.assertEqual(context.messageBar(), mb)


class TestQgsDataItemGuiProviderRegistry(unittest.TestCase):

    def testAppRegistry(self):
        # ensure there is an application instance
        self.assertIsNotNone(QgsGui.dataItemGuiProviderRegistry())

    def testRegistry(self):
        registry = QgsDataItemGuiProviderRegistry()
        initial_providers = registry.providers()

        # add a new provider
        p1 = TestProvider('p1')
        registry.addProvider(p1)
        self.assertIn(p1, registry.providers())

        p2 = TestProvider('p2')
        registry.addProvider(p2)
        self.assertIn(p1, registry.providers())
        self.assertIn(p2, registry.providers())

        registry.removeProvider(None)
        p3 = TestProvider('p3')
        # not in registry yet
        registry.removeProvider(p3)

        registry.removeProvider(p1)
        self.assertNotIn('p1', [p.name() for p in registry.providers()])
        self.assertIn(p2, registry.providers())

        registry.removeProvider(p2)
        self.assertNotIn('p2', [p.name() for p in registry.providers()])
        self.assertEqual(registry.providers(), initial_providers)


if __name__ == '__main__':
    unittest.main()
