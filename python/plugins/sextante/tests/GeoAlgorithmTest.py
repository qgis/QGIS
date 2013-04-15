# -*- coding: utf-8 -*-

"""
***************************************************************************
    GeoAlgorithmTest.py
    ---------------------
    Date                 : March 2013
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
__date__ = 'March 2013'
__copyright__ = '(C) 2013, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import sextante
import unittest
from sextante.tests.TestData import points, points2, polygons, polygons2, lines, union,\
    table, polygonsGeoJson, raster
from sextante.core.QGisLayers import QGisLayers
from sextante.core.SextanteUtils import SextanteUtils

class GeoAlgorithmTest(unittest.TestCase):

    def testWrongformat(self):
        outputs=sextante.runalg("qgis:countpointsinpolygon",polygons(),points(),"NUMPOINTS",SextanteUtils.getTempFilename("wrongext"))
        output=outputs['OUTPUT']
        self.assertTrue(output.endswith('shp'))
        layer=QGisLayers.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','POLY_NUM_A','POLY_ST_A','NUMPOINTS']
        expectedtypes=['Integer','Real','String','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=sextante.getfeatures(layer)
        self.assertEqual(2, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","string a","6"]
        values=[str(attr.toString()) for attr in attrs]
        self.assertEqual(expectedvalues, values)


def suite():
    suite = unittest.makeSuite(GeoAlgorithmTest, 'test')
    return suite

def runtests():
    result = unittest.TestResult()
    testsuite = suite()
    testsuite.run(result)
    return result
