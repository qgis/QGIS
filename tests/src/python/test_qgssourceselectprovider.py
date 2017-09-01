# -*- coding: utf-8 -*-
"""
Test the QgsSourceSelectProvider class

Run with: ctest -V -R PyQgsSourceSelectProvider

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import os
import tempfile
from qgis.gui import (QgsSourceSelectProvider, QgsAbstractDataSourceWidget)
from qgis.testing import start_app, unittest
from qgis.PyQt.QtGui import QIcon

__author__ = 'Alessandro Pasotti'
__date__ = '01/09/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'


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
        self.assertEqual(provider.text(), "MyTestProviderText")
        self.assertTrue(isinstance(provider.icon(), QIcon))


if __name__ == '__main__':
    unittest.main()
