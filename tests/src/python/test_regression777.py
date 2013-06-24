# -*- coding: utf-8 -*-
"""QGIS Unit test for issue 777
@see: http://hub.qgis.org/issues/777

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Matthias Kuhn'
__date__ = '24.2.2013'
__copyright__ = 'Copyright 2013, The Quantum GIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os
from PyQt4.QtCore import QVariant

from qgis.core import (QgsVectorLayer
                       , QgsFeature
                       , QgsGeometry
                       )
from utilities import (unitTestDataPath,
                       getQgisTestApp,
                       TestCase,
                       unittest,
                       #expectedFailure
                       )
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

class TestRegression777(TestCase):

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass
    
    def test_Regression(self):
        myPath = os.path.join( unitTestDataPath(), 'lines.shp' )
        myLayer = QgsVectorLayer( myPath, 'Lines', 'ogr' )
        myLayer.setSelectedFeatures( [1] )
        ft = myLayer.selectedFeatures()
        a = ft[0].geometry().exportToWkt()
        b = myLayer.selectedFeatures()[0].geometry().exportToWkt()
        assert a == b

if __name__ == '__main__':
    unittest.main()

