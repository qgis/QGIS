# -*- coding: utf-8 -*-
"""QGIS Unit tests for MetaSearch Catalogue Client.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Tom Kralidis'
__date__ = '03/12/2014'
__copyright__ = 'Copyright 2014, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis import utils

from utilities import (unitTestDataPath,
                       getQgisTestApp,
                       TestCase,
                       unittest,
                       expectedFailure
                       )

QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

class TestQgsMetaSearch(TestCase):

    def __init__(self, methodName):
        """Run once on class initialisation."""
        unittest.TestCase.__init__(self, methodName)

    def setUp(self):
        self.plugin = utils.plugins['MetaSearch']

    def test_plugin_metadata(self):
        myExpectedValue = '&MetaSearch'
        myActualValue = self.plugin.web_menu
        myMessage = 'Expected: %s Got: %s' % (myExpectedValue, myActualValue)
        assert myExpectedValue == myActualValue, myMessage

        myExpectedValue = 'MetaSearch Catalogue Client'
        myActualValue = self.plugin.context.metadata.get('general', 'name')
        myMessage = 'Expected: %s Got: %s' % (myExpectedValue, myActualValue)
        assert myExpectedValue == myActualValue, myMessage



if __name__ == '__main__':
    unittest.main()

