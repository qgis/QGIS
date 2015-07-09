# -*- coding: utf-8 -*-

"""
***************************************************************************
    ProcessingToolsTest.py
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

import unittest

import processing
from processing.tools.vector import values
from processing.tools.dataobjects import getObjectFromName

from processing.tests.TestData import points, polygons


class ProcessingToolsTest(unittest.TestCase):
    '''Tests the method imported when doing an "import processing", and
    also in processing.tools. They are mostly convenience tools.
    '''

    def test_getobject(self):
        layer = processing.getObject(points())
        self.assertIsNotNone(layer)
        layer = processing.getObject('points')
        self.assertIsNotNone(layer)

    def test_runandload(self):
        processing.runandload('qgis:countpointsinpolygon', polygons(),
                              points(), 'NUMPOINTS', None)
        layer = getObjectFromName('Result')
        self.assertIsNotNone(layer)

    def test_featuresWithoutSelection(self):
        layer = processing.getObject(points())
        features = processing.features(layer)
        self.assertEqual(12, len(features))

    def test_featuresWithSelection(self):
        layer = processing.getObject(points())
        feature = layer.getFeatures().next()
        selected = [feature.id()]
        layer.setSelectedFeatures(selected)
        features = processing.features(layer)
        self.assertEqual(1, len(features))
        layer.setSelectedFeatures([])

    def test_attributeValues(self):
        layer = processing.getObject(points())
        attributeValues = values(layer, 'ID')
        i = 1
        for value in attributeValues['ID']:
            self.assertEqual(int(i), int(value))
            i += 1
        self.assertEquals(13, i)

    def test_extent(self):
        pass


def suite():
    suite = unittest.makeSuite(ProcessingToolsTest, 'test')
    return suite


def runtests():
    result = unittest.TestResult()
    testsuite = suite()
    testsuite.run(result)
    return result
