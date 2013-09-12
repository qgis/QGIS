# -*- coding: utf-8 -*-

"""
***************************************************************************
    RunAlgTest.py
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
from processing.tools.system import *

class ParametrizedTestCase(unittest.TestCase):

    def __init__(self, methodName='runTest', useTempFiles=False):
        super(ParametrizedTestCase, self).__init__(methodName)
        self.useTempFiles = useTempFiles

    @staticmethod
    def parametrize(testcase_klass, useTempFiles):
        testloader = unittest.TestLoader()
        testnames = testloader.getTestCaseNames(testcase_klass)
        suite = unittest.TestSuite()
        for name in testnames:
            suite.addTest(testcase_klass(name, useTempFiles=useTempFiles))
        return suite

class RunAlgTest(ParametrizedTestCase):
    '''This test takes a reduced set of algorithms and executes them in different ways, changing
    parameters such as whether to use temp outputs, the output file format, etc.
    Basically, it uses some algorithms to test other parts of the processign framework, not the algorithms themselves'''

    def getOutputFile(self):
        if self.useTempFiles:
            return None
        else:
            return getTempFilename('shp')

    def test_qgiscountpointsinpolygon(self):
        outputs=processing.runalg("qgis:countpointsinpolygon",polygons(),points(),"NUMPOINTS", self.getOutputFile())
        output=outputs['OUTPUT']
        layer=dataobjects.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','POLY_NUM_A','POLY_ST_A','NUMPOINTS']
        expectedtypes=['Integer','Real','String','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=processing.features(layer)
        self.assertEqual(2, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","string a","6"]
        values=[str(attr) for attr in attrs]
        self.assertEqual(expectedvalues, values)

def suite():
    suite = unittest.TestSuite()
    suite.addTest(ParametrizedTestCase.parametrize(RunAlgTest, False))
    suite.addTest(ParametrizedTestCase.parametrize(RunAlgTest, True))
    return suite

def runtests():
    result = unittest.TestResult()
    testsuite = suite()
    testsuite.run(result)
    return result
