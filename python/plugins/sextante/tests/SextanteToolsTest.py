# -*- coding: utf-8 -*-

"""
***************************************************************************
    SextanteToolsTest.py
    ---------------------
    Date                 : April 2013
    Copyright            : (C) 2013 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'April 2013'
__copyright__ = '(C) 2013, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import sextante
import unittest
from sextante.tests.TestData import points, points2, polygons, polygons2, lines, union,\
    table, polygonsGeoJson, raster
from sextante.core import Sextante
from sextante.tools.vector import values
from sextante.tools.general import getfromname

class SextanteToolsTest(unittest.TestCase):
    '''tests the method imported when doing an "import sextante", and also in sextante.tools.
    They are mostly convenience tools'''

    def test_getobject(self):
        layer = sextante.getobject(points());
        self.assertIsNotNone(layer)
        layer = sextante.getobject("points");
        self.assertIsNotNone(layer)

    def test_runandload(self):
        sextante.runandload("qgis:countpointsinpolygon",polygons(),points(),"NUMPOINTS", None)
        layer = getfromname("Result")
        self.assertIsNotNone(layer)

    def test_featuresWithoutSelection(self):
        layer = sextante.getobject(points())
        features = sextante.getfeatures(layer)
        self.assertEqual(12, len(features))

    def test_featuresWithSelection(self):
        layer = sextante.getobject(points())
        feature = layer.getFeatures().next()
        selected = [feature.id()]
        layer.setSelectedFeatures(selected)
        features = sextante.getfeatures(layer)
        self.assertEqual(1, len(features))
        layer.setSelectedFeatures([])

    def test_attributeValues(self):
        layer = sextante.getobject(points())
        attributeValues = values(layer, "ID")
        i = 1
        for value in attributeValues['ID']:
            self.assertEqual(int(i), int(value))
            i+=1
        self.assertEquals(13,i)

    def test_extent(self):
        pass



def suite():
    suite = unittest.makeSuite(SextanteToolsTest, 'test')
    return suite

def runtests():
    result = unittest.TestResult()
    testsuite = suite()
    testsuite.run(result)
    return result
