# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsSubsetStringEditorProviderRegistry

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Even Rouault'
__date__ = '15/11/2020'
__copyright__ = 'Copyright 2018, The QGIS Project'

import qgis  # NOQA

from qgis.gui import (QgsGui,
                      QgsSubsetStringEditorProvider)
from qgis.testing import start_app, unittest

app = start_app()


class TestProvider(QgsSubsetStringEditorProvider):

    def __init__(self, name):
        super().__init__()
        self._name = name

    def providerKey(self):
        return self._name

    def canHandleLayer(self, layer):
        return False

    def createDialog(self, layer, parent, fl):
        return None


class TestQgsSubsetStringEditorProviderRegistry(unittest.TestCase):

    def testGuiRegistry(self):
        # ensure there is an application instance
        self.assertIsNotNone(QgsGui.subsetStringEditorProviderRegistry())

    def testRegistry(self):
        registry = QgsGui.subsetStringEditorProviderRegistry()
        initial_providers = registry.providers()
        self.assertTrue(initial_providers)  # we expect a bunch of default providers
        self.assertTrue([p.name() for p in initial_providers if p.name() == 'WFS'])

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

    def testProviderKey(self):
        """Tests finding provider by name and return providerKey"""

        registry = QgsGui.subsetStringEditorProviderRegistry()
        self.assertIsNotNone(registry.providerByName('WFS'))
        self.assertIsNone(registry.providerByName('i_do_not_exist'))
        self.assertEqual(registry.providerByName('WFS').providerKey(), 'WFS')


if __name__ == '__main__':
    unittest.main()
