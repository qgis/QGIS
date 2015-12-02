# -*- coding: utf-8 -*-

"""
***************************************************************************
    ParametersTest.py
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

import unittest
from utilities import getQgisTestApp, unittest
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

from processing.core.parameters import (Parameter,
                                        ParameterBoolean,
                                        ParameterCrs,
                                        ParameterDataObject,
                                        ParameterExtent,
                                        ParameterFile,
                                        ParameterFixedTable,
                                        ParameterNumber)

class ParameterTest(unittest.TestCase):
    def testGetValueAsCommandLineParameter(self):
        parameter = Parameter('myName', 'myDesc')
        parameter.setValue(None)
        self.assertEqual(parameter.getValueAsCommandLineParameter(), "None")

        parameter.setValue("someValue")
        self.assertEqual(parameter.getValueAsCommandLineParameter(), 'someValue')

        parameter.setValue(123)
        self.assertEqual(parameter.getValueAsCommandLineParameter(), '123')

class ParameterBooleanTest(unittest.TestCase):
    def testInitDefaults(self):
        parameter = ParameterBoolean()
        self.assertEqual(parameter.name, '')
        self.assertEqual(parameter.description, '')
        self.assertEqual(parameter.default, True)
        self.assertEqual(parameter.optional, False)

    def testInit(self):
        parameter = ParameterBoolean('myName', 'myDescription', False, True)
        self.assertEqual(parameter.name, 'myName')
        self.assertEqual(parameter.description, 'myDescription')
        self.assertEqual(parameter.default, False)
        self.assertEqual(parameter.optional, True)

    def testSetValue(self):
        parameter = ParameterBoolean('myName', 'myDescription', False, True)
        self.assertEqual(parameter.value, None)
        parameter.setValue(True)
        self.assertEqual(parameter.value, True)
        parameter.setValue(False)
        self.assertEqual(parameter.value, False)

    def testDefault(self):
        default = False
        parameter = ParameterBoolean('myName', 'myDescription', default, True)
        parameter.setValue(None)
        self.assertEqual(parameter.value, default)

    def testOptional(self):
        optionalParameter = ParameterBoolean('myName', 'myDescription', default = False, optional = True)
        optionalParameter.setValue(True)
        optionalParameter.setValue(None)
        self.assertEqual(optionalParameter.value, False)

        requiredParameter = ParameterBoolean('myName', 'myDescription', default = False, optional = False)
        requiredParameter.setValue(True)
        requiredParameter.setValue(None)
        self.assertEqual(requiredParameter.value, True)

class ParameterCRSTest(unittest.TestCase):

    def testSetValue(self):
        parameter = ParameterCrs('myName', 'myDesc', default='EPSG:4326', optional=False)
        self.assertTrue(parameter.setValue('EPSG:12003'))
        self.assertEqual(parameter.value, 'EPSG:12003')

    def testOptional(self):
        optionalParameter = ParameterCrs('myName', 'myDesc', default='EPSG:4326', optional=True)
        optionalParameter.setValue('EPSG:12003')
        optionalParameter.setValue(None)
        self.assertEqual(optionalParameter.value, 'EPSG:4326')

        requiredParameter = ParameterCrs('myName', 'myDesc', default='EPSG:4326', optional=False)
        requiredParameter.setValue('EPSG:12003')
        requiredParameter.setValue(None)
        self.assertEqual(requiredParameter.value, 'EPSG:12003')

class ParameterDataObjectTest(unittest.TestCase):
    def testGetValueAsCommandLineParameter(self):
        parameter = ParameterDataObject('myName', 'myDesc')
        parameter.setValue(None)
        self.assertEqual(parameter.getValueAsCommandLineParameter(), "None")

        parameter = ParameterDataObject('myName', 'myDesc')
        parameter.setValue("someFile.dat")
        self.assertEqual(parameter.getValueAsCommandLineParameter(), '"someFile.dat"')

class ParameterExtentTest(unittest.TestCase):
    def testSetInvalidValue(self):
        parameter = ParameterExtent('myName', 'myDesc')
        self.assertFalse(parameter.setValue('0,2,0'))
        self.assertFalse(parameter.setValue('0,2,0,a'))

    def testSetValue(self):
        parameter = ParameterExtent('myName', 'myDesc')
        self.assertTrue(parameter.setValue('0,2,2,4'))
        self.assertEqual(parameter.value, '0,2,2,4')

    def testOptional(self):
        optionalParameter = ParameterExtent('myName', 'myDesc', default='0,1,0,1', optional=True)
        optionalParameter.setValue('1,2,3,4')
        optionalParameter.setValue(None)
        # Extent is unique in that it will let you set "None", whereas other
        # optional params become "default" when assigning None.
        self.assertEqual(optionalParameter.value, None)

        requiredParameter = ParameterExtent('myName', 'myDesc', default='0,1,0,1', optional=False)
        requiredParameter.setValue('1,2,3,4')
        requiredParameter.setValue(None)
        self.assertEqual(requiredParameter.value, '1,2,3,4')

class ParameterFileTest(unittest.TestCase):
    def testSetValueWhenOptional(self):
        parameter = ParameterFile('myName', 'myDesc', isFolder=False, optional=True)
        self.assertTrue(parameter.setValue('myFile.png'))
        self.assertEquals(parameter.value, 'myFile.png')

        self.assertTrue(parameter.setValue(""))
        self.assertEquals(parameter.value, '')

        self.assertTrue(parameter.setValue(None))
        self.assertEquals(parameter.value, '')

    def testSetValidValueWhenRequired(self):
        parameter = ParameterFile('myName', 'myDesc', isFolder=False, optional=False)
        self.assertTrue(parameter.setValue('myFile.png'))
        self.assertEquals(parameter.value, 'myFile.png')

        self.assertFalse(parameter.setValue(""))
        self.assertEquals(parameter.value, '')

        self.assertFalse(parameter.setValue(None))
        self.assertEquals(parameter.value, '')

    def testSetValueWithExtension(self):
        parameter = ParameterFile('myName', 'myDesc', isFolder=False, optional=True, ext="png")
        self.assertTrue(parameter.setValue('myFile.png'))
        self.assertEquals(parameter.value, 'myFile.png')

        self.assertFalse(parameter.setValue('myFile.bmp'))
        self.assertEquals(parameter.value, 'myFile.bmp')

    def testGetValueAsCommandLineParameter(self):
        parameter = ParameterFile('myName', 'myDesc')
        parameter.setValue('myFile.png')
        self.assertEqual(parameter.getValueAsCommandLineParameter(), '"myFile.png"')

class TestParameterFixedTable(unittest.TestCase):

    def testTableToString(self):
        table = [
                    ['a0', 'a1', 'a2'],
                    ['b0', 'b1', 'b2']
                ]
        self.assertEquals(ParameterFixedTable.tableToString(table), 'a0,a1,a2,b0,b1,b2')

        table = [['a0']]
        self.assertEquals(ParameterFixedTable.tableToString(table), 'a0')

        table = [[]]
        self.assertEquals(ParameterFixedTable.tableToString(table), '')

    def testSetStringValue(self):
        parameter = ParameterFixedTable('myName', 'myDesc')
        self.assertTrue(parameter.setValue('1,2,3'))
        self.assertEqual(parameter.value, '1,2,3')

    def testSet2DListValue(self):
        table = [
                    ['a0', 'a1', 'a2'],
                    ['b0', 'b1', 'b2']
                ]
        parameter = ParameterFixedTable('myName', 'myDesc')
        self.assertTrue(parameter.setValue(table))
        self.assertEqual(parameter.value, 'a0,a1,a2,b0,b1,b2')

    def testOptional(self):
        parameter = ParameterFixedTable('myName', 'myDesc', numRows=3, cols=['values'], fixedNumOfRows=False, optional=True)
        self.assertTrue(parameter.setValue('1,2,3'))
        self.assertEqual(parameter.value, '1,2,3')

        self.assertTrue(parameter.setValue(None))
        self.assertEqual(parameter.value, None)

        parameter = ParameterFixedTable('myName', 'myDesc', numRows=3, cols=['values'], fixedNumOfRows=False, optional=False)
        self.assertFalse(parameter.setValue(None))
        self.assertEqual(parameter.value, None)

        self.assertTrue(parameter.setValue('1,2,3'))
        self.assertEqual(parameter.value, '1,2,3')

        self.assertFalse(parameter.setValue(None))
        self.assertEqual(parameter.value, '1,2,3')

class ParameterNumberTest(unittest.TestCase):
    def testSetOnlyValidNumbers(self):
        parameter = ParameterNumber('myName', 'myDescription', minValue = 0, maxValue = 10, default = 1.0, optional = True)
        self.assertFalse(parameter.setValue('wrongvalue'))
        self.assertEqual(parameter.value, None)

    def testMaxValue(self):
        parameter = ParameterNumber('myName', 'myDescription', minValue = 0, maxValue = 10, default = 1.0, optional = True)
        self.assertFalse(parameter.setValue(11))
        self.assertEqual(parameter.value, None)
        self.assertTrue(parameter.setValue(10))
        self.assertEqual(parameter.value, 10)

    def testMinValue(self):
        parameter = ParameterNumber('myName', 'myDescription', minValue = 3, maxValue = 10, default = 1.0, optional = True)
        self.assertFalse(parameter.setValue(1))
        self.assertFalse(parameter.setValue(-2))
        self.assertEqual(parameter.value, None)
        self.assertTrue(parameter.setValue(3))
        self.assertEqual(parameter.value, 3)

    def testSetValue(self):
        parameter = ParameterNumber('myName', 'myDescription', minValue = 0, maxValue = 10, default = 1.0, optional = True)
        self.assertTrue(parameter.setValue(5))
        self.assertEquals(parameter.value, 5)

    def testOptional(self):
        optionalParameter = ParameterNumber('myName', 'myDescription', minValue = 0, maxValue = 10, default = 1.0, optional = True)
        optionalParameter.setValue(5)
        optionalParameter.setValue(None)
        self.assertEqual(optionalParameter.value, 1.0)

        requiredParameter = ParameterNumber('myName', 'myDescription', minValue = 0, maxValue = 10, default = 1.0, optional = False)
        requiredParameter.setValue(5)
        requiredParameter.setValue(None)
        self.assertEqual(requiredParameter.value, 5)

def suite():
    suite = unittest.makeSuite(ParametersTest, 'test')
    return suite

def runtests():
    result = unittest.TestResult()
    testsuite = suite()
    testsuite.run(result)
    return result

if __name__ == '__main__':
    unittest.main()
