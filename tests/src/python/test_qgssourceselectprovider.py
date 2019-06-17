# -*- coding: utf-8 -*-
"""
Test the QgsSourceSelectProvider 
and QgsSourceSelectProviderRegistry classes

Run with: ctest -V -R PyQgsSourceSelectProvider

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os
import tempfile
from qgis.gui import (QgsGui, QgsSourceSelectProvider, QgsSourceSelectProviderRegistry, QgsAbstractDataSourceWidget)
from qgis.testing import start_app, unittest
from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtWidgets import QWidget

__author__ = 'Alessandro Pasotti'
__date__ = '01/09/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'


start_app()


class ConcreteDataSourceWidget(QgsAbstractDataSourceWidget):
    pass


class ConcreteSourceSelectProvider(QgsSourceSelectProvider):

    def providerKey(self):
        return "MyTestProviderKey"

    def text(self):
        return "MyTestProviderText"

    def icon(self):
        return QIcon()

    def createDataSourceWidget(self):
        return ConcreteDataSourceWidget()

    def ordering(self):
        return 1


class ConcreteSourceSelectProvider2(QgsSourceSelectProvider):

    def providerKey(self):
        return "MyTestProviderKey2"

    def text(self):
        return "MyTestProviderText2"

    def name(self):
        return "MyName"

    def toolTip(self):
        return "MyToolTip"

    def icon(self):
        return QIcon()

    def createDataSourceWidget(self):
        return ConcreteDataSourceWidget()

    def ordering(self):
        return 2


class TestQgsSourceSelectProvider(unittest.TestCase):

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def testConcreteClass(self):

        provider = ConcreteSourceSelectProvider()
        self.assertTrue(isinstance(provider, ConcreteSourceSelectProvider))
        widget = provider.createDataSourceWidget()
        self.assertTrue(isinstance(widget, ConcreteDataSourceWidget))
        self.assertEqual(provider.providerKey(), "MyTestProviderKey")
        self.assertEqual(provider.name(), "MyTestProviderKey")
        self.assertEqual(provider.text(), "MyTestProviderText")
        self.assertEqual(provider.toolTip(), "")
        self.assertEqual(provider.ordering(), 1)
        self.assertTrue(isinstance(provider.icon(), QIcon))

        # test toolTip
        provider = ConcreteSourceSelectProvider2()
        self.assertEqual(provider.toolTip(), "MyToolTip")

    def _testRegistry(self, registry):

        registry.addProvider(ConcreteSourceSelectProvider())
        registry.addProvider(ConcreteSourceSelectProvider2())

        # Check order
        self.assertEqual(['MyTestProviderKey', 'MyName'], [p.name() for p in registry.providers() if p.providerKey().startswith('MyTestProviderKey')])

        registry = QgsSourceSelectProviderRegistry()
        registry.addProvider(ConcreteSourceSelectProvider())
        registry.addProvider(ConcreteSourceSelectProvider2())

        # Check order
        self.assertEqual(['MyTestProviderKey', 'MyName'], [p.name() for p in registry.providers() if p.providerKey().startswith('MyTestProviderKey')])

        # Get provider by name
        self.assertTrue(registry.providerByName('MyTestProviderKey'))
        self.assertTrue(registry.providerByName('MyName'))

        # Get not existent by name
        self.assertFalse(registry.providerByName('Oh This Is Missing!'))

        # Get providers by data provider key
        self.assertGreater(len(registry.providersByKey('MyTestProviderKey')), 0)
        self.assertGreater(len(registry.providersByKey('MyTestProviderKey2')), 0)

        # Get not existent by key
        self.assertEqual(len(registry.providersByKey('Oh This Is Missing!')), 0)

    def testRemoveProvider(self):
        registry = QgsSourceSelectProviderRegistry()
        registry.addProvider(ConcreteSourceSelectProvider())
        registry.addProvider(ConcreteSourceSelectProvider2())
        self.assertEqual(['MyTestProviderKey', 'MyName'], [p.name() for p in registry.providers() if p.providerKey().startswith('MyTestProviderKey')])

        self.assertTrue(registry.removeProvider(registry.providerByName('MyName')))
        self.assertEqual(['MyTestProviderKey'], [p.name() for p in registry.providers() if p.providerKey().startswith('MyTestProviderKey')])

        self.assertTrue(registry.removeProvider(registry.providerByName('MyTestProviderKey')))
        self.assertEqual([], [p.name() for p in registry.providers() if p.providerKey().startswith('MyTestProviderKey')])

    def testRegistry(self):
        registry = QgsSourceSelectProviderRegistry()
        self._testRegistry(registry)

    def testRegistrySingleton(self):
        registry = QgsGui.sourceSelectProviderRegistry()
        self._testRegistry(registry)
        # Check that at least OGR and GDAL are here
        self.assertTrue(registry.providersByKey('ogr'))
        self.assertTrue(registry.providersByKey('gdal'))


if __name__ == '__main__':
    unittest.main()
