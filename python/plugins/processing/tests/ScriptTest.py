# -*- coding: utf-8 -*-

"""
***************************************************************************
    ScriptTest.py
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

import processing
import unittest
from processing.tests.TestData import points, points2, polygons, polygons2, lines, union,\
    table
from processing.tools import dataobjects

class ScriptTest(unittest.TestCase):
    '''tests that use scripts'''

    def test_scriptcreatetilingfromvectorlayer(self):
        outputs=processing.runalg("script:createtilingfromvectorlayer",union(),10,None)
        output=outputs['polygons']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['longitude','latitude']
        expectedtypes=['Real','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(10, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["270761.415396242","4458948.29588823"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POLYGON((270755.54427424 4458901.23378639,270755.54427424 4458995.35799007,270767.28651824 4458995.35799007,270767.28651824 4458901.23378639,270755.54427424 4458901.23378639))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_scripthexgridfromlayerbounds(self):
        outputs=processing.runalg("script:hexgridfromlayerbounds",polygons(),10,None)
        output=outputs['grid']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['longitude','latitude']
        expectedtypes=['Real','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(117, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["270765.621834001","4458907.27146471"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POLYGON((270771.39533669 4458907.27146471,270768.50858535 4458902.27146471,270762.73508265 4458902.27146471,270759.84833131 4458907.27146471,270762.73508265 4458912.27146471,270768.50858535 4458912.27146471,270771.39533669 4458907.27146471))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_scriptascriptthatreturnsanumber(self):
        outputs=processing.runalg("script:ascriptthatreturnsanumber")
        output=outputs['number']
        self.assertTrue(10, output)



def suite():
    suite = unittest.makeSuite(ScriptTest, 'test')
    return suite

def runtests():
    result = unittest.TestResult()
    testsuite = suite()
    testsuite.run(result)
    return result