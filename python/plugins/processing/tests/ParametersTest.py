# -*- coding: utf-8 -*-

"""
***************************************************************************
    ParametersTest
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

from qgis.testing import start_app, unittest

from processing.core.parameters import (Parameter,
                                        ParameterBoolean,
                                        ParameterCrs,
                                        ParameterDataObject,
                                        ParameterExtent,
                                        ParameterFile,
                                        ParameterFixedTable,
                                        ParameterMultipleInput,
                                        ParameterNumber,
                                        ParameterPoint,
                                        ParameterString,
                                        ParameterVector,
                                        ParameterTableField,
                                        ParameterTableMultipleField)
from processing.tests.TestData import points2

from qgis.core import (QgsRasterLayer,
                       QgsVectorLayer)

start_app()


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

    def testSetValue(self):
        parameter = ParameterBoolean('myName', 'myDescription')
        self.assertEqual(parameter.value, None)
        parameter.setValue(False)
        self.assertEqual(parameter.value, False)
        parameter.setValue(True)
        self.assertEqual(parameter.value, True)

    def testDefault(self):
        parameter = ParameterBoolean('myName', 'myDescription', default=False, optional=True)
        self.assertEqual(parameter.value, False)
        parameter.setValue(None)
        self.assertEqual(parameter.value, None)

    def testOptional(self):
        optionalParameter = ParameterBoolean('myName', 'myDescription', default=False, optional=True)
        self.assertEqual(optionalParameter.value, False)
        optionalParameter.setValue(True)
        self.assertEqual(optionalParameter.value, True)
        self.assertTrue(optionalParameter.setValue(None))
        self.assertEqual(optionalParameter.value, None)

        requiredParameter = ParameterBoolean('myName', 'myDescription', default=False, optional=False)
        self.assertEqual(requiredParameter.value, False)
        requiredParameter.setValue(True)
        self.assertEqual(requiredParameter.value, True)
        self.assertFalse(requiredParameter.setValue(None))
        self.assertEqual(requiredParameter.value, True)


class ParameterCRSTest(unittest.TestCase):

    def testSetValue(self):
        parameter = ParameterCrs('myName', 'myDesc')
        self.assertTrue(parameter.setValue('EPSG:12003'))
        self.assertEqual(parameter.value, 'EPSG:12003')

    def testOptional(self):
        optionalParameter = ParameterCrs('myName', 'myDesc', default='EPSG:4326', optional=True)
        self.assertEqual(optionalParameter.value, 'EPSG:4326')
        optionalParameter.setValue('EPSG:12003')
        self.assertEqual(optionalParameter.value, 'EPSG:12003')
        self.assertTrue(optionalParameter.setValue(None))
        self.assertEqual(optionalParameter.value, None)

        requiredParameter = ParameterCrs('myName', 'myDesc', default='EPSG:4326', optional=False)
        self.assertEqual(requiredParameter.value, 'EPSG:4326')
        requiredParameter.setValue('EPSG:12003')
        self.assertEqual(requiredParameter.value, 'EPSG:12003')
        self.assertFalse(requiredParameter.setValue(None))
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

    def testSetValue(self):
        parameter = ParameterExtent('myName', 'myDesc')
        self.assertTrue(parameter.setValue('0,2,2,4'))
        self.assertEqual(parameter.value, '0,2,2,4')

    def testSetInvalidValue(self):
        parameter = ParameterExtent('myName', 'myDesc')
        self.assertFalse(parameter.setValue('0,2,0'))
        self.assertFalse(parameter.setValue('0,2,0,a'))

    def testOptional(self):
        optionalParameter = ParameterExtent('myName', 'myDesc', default='0,1,0,1', optional=True)
        self.assertEqual(optionalParameter.value, '0,1,0,1')
        optionalParameter.setValue('1,2,3,4')
        self.assertEqual(optionalParameter.value, '1,2,3,4')
        self.assertTrue(optionalParameter.setValue(None))
        # Extent is unique in that it will let you set `None`, whereas other
        # optional parameters become "default" when assigning None.
        self.assertEqual(optionalParameter.value, None)

        requiredParameter = ParameterExtent('myName', 'myDesc', default='0,1,0,1', optional=False)
        self.assertEqual(requiredParameter.value, '0,1,0,1')
        requiredParameter.setValue('1,2,3,4')
        self.assertEqual(requiredParameter.value, '1,2,3,4')
        self.assertFalse(requiredParameter.setValue(None))
        self.assertEqual(requiredParameter.value, '1,2,3,4')


class ParameterPointTest(unittest.TestCase):

    def testSetValue(self):
        parameter = ParameterPoint('myName', 'myDesc')
        self.assertTrue(parameter.setValue('0,2'))
        self.assertEqual(parameter.value, '0,2')

    def testSetInvalidValue(self):
        parameter = ParameterPoint('myName', 'myDesc')
        self.assertFalse(parameter.setValue('0'))
        self.assertFalse(parameter.setValue('0,a'))

    def testOptional(self):
        optionalParameter = ParameterPoint('myName', 'myDesc', default='0,1', optional=True)
        self.assertEqual(optionalParameter.value, '0,1')
        optionalParameter.setValue('1,2')
        self.assertEqual(optionalParameter.value, '1,2')
        self.assertTrue(optionalParameter.setValue(None))
        # Extent is unique in that it will let you set `None`, whereas other
        # optional parameters become "default" when assigning None.
        self.assertEqual(optionalParameter.value, None)

        requiredParameter = ParameterPoint('myName', 'myDesc', default='0,1', optional=False)
        self.assertEqual(requiredParameter.value, '0,1')
        requiredParameter.setValue('1,2')
        self.assertEqual(requiredParameter.value, '1,2')
        self.assertFalse(requiredParameter.setValue(None))
        self.assertEqual(requiredParameter.value, '1,2')


class ParameterFileTest(unittest.TestCase):

    def testSetValue(self):
        parameter = ParameterFile('myName', 'myDesc')
        self.assertTrue(parameter.setValue('myFile.png'))
        self.assertEqual(parameter.value, 'myFile.png')

    def testOptional(self):
        optionalParameter = ParameterFile('myName', 'myDesc', optional=True)
        self.assertTrue(optionalParameter.setValue('myFile.png'))
        self.assertEqual(optionalParameter.value, 'myFile.png')

        self.assertTrue(optionalParameter.setValue(""))
        self.assertEqual(optionalParameter.value, '')

        self.assertTrue(optionalParameter.setValue(None))
        self.assertEqual(optionalParameter.value, None)

        requiredParameter = ParameterFile('myName', 'myDesc', optional=False)
        self.assertTrue(requiredParameter.setValue('myFile.png'))
        self.assertEqual(requiredParameter.value, 'myFile.png')

        self.assertFalse(requiredParameter.setValue(''))
        self.assertEqual(requiredParameter.value, 'myFile.png')

        self.assertFalse(requiredParameter.setValue('  '))
        self.assertEqual(requiredParameter.value, 'myFile.png')

        self.assertFalse(requiredParameter.setValue(None))
        self.assertEqual(requiredParameter.value, 'myFile.png')

    def testSetValueWithExtension(self):
        parameter = ParameterFile('myName', 'myDesc', ext="png")
        self.assertTrue(parameter.setValue('myFile.png'))
        self.assertEqual(parameter.value, 'myFile.png')

        self.assertFalse(parameter.setValue('myFile.bmp'))
        self.assertEqual(parameter.value, 'myFile.png')

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
        self.assertEqual(ParameterFixedTable.tableToString(table), 'a0,a1,a2,b0,b1,b2')

        table = [['a0']]
        self.assertEqual(ParameterFixedTable.tableToString(table), 'a0')

        table = [[]]
        self.assertEqual(ParameterFixedTable.tableToString(table), '')

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
        parameter = ParameterFixedTable('myName', 'myDesc', optional=True)
        self.assertTrue(parameter.setValue('1,2,3'))
        self.assertEqual(parameter.value, '1,2,3')

        self.assertTrue(parameter.setValue(None))
        self.assertEqual(parameter.value, None)

        parameter = ParameterFixedTable('myName', 'myDesc', optional=False)
        self.assertFalse(parameter.setValue(None))
        self.assertEqual(parameter.value, None)

        self.assertTrue(parameter.setValue('1,2,3'))
        self.assertEqual(parameter.value, '1,2,3')

        self.assertFalse(parameter.setValue(None))
        self.assertEqual(parameter.value, '1,2,3')


class ParameterMultipleInputTest(unittest.TestCase):

    def testOptional(self):
        parameter = ParameterMultipleInput('myName', 'myDesc', optional=True)
        self.assertTrue(parameter.setValue('myLayerFile.shp'))
        self.assertEqual(parameter.value, 'myLayerFile.shp')

        self.assertTrue(parameter.setValue(None))
        self.assertEqual(parameter.value, None)

        parameter = ParameterMultipleInput('myName', 'myDesc', optional=False)
        self.assertFalse(parameter.setValue(None))
        self.assertEqual(parameter.value, None)

        self.assertTrue(parameter.setValue("myLayerFile.shp"))
        self.assertEqual(parameter.value, "myLayerFile.shp")

        self.assertFalse(parameter.setValue(None))
        self.assertEqual(parameter.value, "myLayerFile.shp")

    def testMultipleInput(self):
        parameter = ParameterMultipleInput('myName', 'myDesc', optional=True)
        self.assertTrue(parameter.setMinNumInputs(1))

        parameter = ParameterMultipleInput('myName', 'myDesc', optional=False)
        self.assertFalse(parameter.setMinNumInputs(0))

        parameter.setMinNumInputs(2)
        self.assertTrue(parameter.setValue(['myLayerFile.shp', 'myLayerFile2.shp']))

        parameter.setMinNumInputs(3)
        self.assertFalse(parameter.setValue(['myLayerFile.shp', 'myLayerFile2.shp']))

    def testGetAsStringWhenRaster(self):
        parameter = ParameterMultipleInput('myName', 'myDesc', datatype=ParameterMultipleInput.TYPE_RASTER)

        # With Path
        self.assertEqual(parameter.getAsString('/some/path'), '/some/path')

        # With Layer
        layer = QgsRasterLayer('/path/to/myRaster.tif', 'myRaster')
        self.assertEqual(parameter.getAsString(layer), '/path/to/myRaster.tif')

        # TODO With Layer Name, instead of Layer object

    def testGetAsStringWhenFile(self):
        parameter = ParameterMultipleInput('myName', 'myDesc', datatype=ParameterMultipleInput.TYPE_FILE)
        self.assertEqual(parameter.getAsString('/some/path'), '/some/path')

    def testGetAsStringWhenVector(self):
        parameter = ParameterMultipleInput('myName', 'myDesc', datatype=ParameterMultipleInput.TYPE_VECTOR_ANY)

        # With Path
        self.assertEqual(parameter.getAsString('/some/path'), '/some/path')

        # With Layer
        layer = QgsVectorLayer('/path/to/myVector.shp', 'myVector', 'memory')
        self.assertEqual(parameter.getAsString(layer), '/path/to/myVector.shp')

        # TODO With Layer Name, instead of Layer object


class ParameterNumberTest(unittest.TestCase):

    def testSetValue(self):
        parameter = ParameterNumber('myName', 'myDescription')
        self.assertTrue(parameter.setValue(5))
        self.assertEqual(parameter.value, 5)

    def testSetValueOnlyValidNumbers(self):
        parameter = ParameterNumber('myName', 'myDescription')
        self.assertFalse(parameter.setValue('not a number'))
        self.assertEqual(parameter.value, None)

    def testIsInteger(self):
        floatParameter = ParameterNumber('myname', 'myDescription', default=1.0)
        self.assertFalse(floatParameter.isInteger)
        intParameter = ParameterNumber('myname', 'myDescription', default=10)
        self.assertTrue(intParameter.isInteger)
        strFloatParameter = ParameterNumber('myname', 'myDescription', default="1.0")
        self.assertFalse(strFloatParameter.isInteger)
        strIntParameter = ParameterNumber('myname', 'myDescription', default="10")
        self.assertTrue(strIntParameter.isInteger)

    def testMaxValue(self):
        parameter = ParameterNumber('myName', 'myDescription', maxValue=10)
        self.assertFalse(parameter.setValue(11))
        self.assertEqual(parameter.value, None)
        self.assertTrue(parameter.setValue(10))
        self.assertEqual(parameter.value, 10)

    def testMinValue(self):
        parameter = ParameterNumber('myName', 'myDescription', minValue=3)
        self.assertFalse(parameter.setValue(1))
        self.assertEqual(parameter.value, None)
        self.assertFalse(parameter.setValue(-2))
        self.assertEqual(parameter.value, None)
        self.assertTrue(parameter.setValue(3))
        self.assertEqual(parameter.value, 3)

    def testOptional(self):
        optionalParameter = ParameterNumber('myName', 'myDescription', default=1.0, optional=True)
        self.assertEqual(optionalParameter.value, 1.0)
        optionalParameter.setValue(5)
        self.assertEqual(optionalParameter.value, 5)
        self.assertTrue(optionalParameter.setValue(None))
        self.assertEqual(optionalParameter.value, None)

        requiredParameter = ParameterNumber('myName', 'myDescription', default=1.0, optional=False)
        self.assertEqual(requiredParameter.value, 1.0)
        requiredParameter.setValue(5)
        self.assertEqual(requiredParameter.value, 5)
        self.assertFalse(requiredParameter.setValue(None))
        self.assertEqual(requiredParameter.value, 5)


class ParameterStringTest(unittest.TestCase):

    def testSetValue(self):
        parameter = ParameterString('myName', 'myDescription')
        self.assertTrue(parameter.setValue('test'))
        self.assertEqual(parameter.value, 'test')

    def testOptional(self):
        optionalParameter = ParameterString('myName', 'myDesc', default='test', optional=True)
        self.assertEqual(optionalParameter.value, 'test')
        optionalParameter.setValue('check')
        self.assertEqual(optionalParameter.value, 'check')
        self.assertTrue(optionalParameter.setValue(None))
        self.assertEqual(optionalParameter.value, None)

        requiredParameter = ParameterCrs('myName', 'myDesc', default='test', optional=False)
        self.assertEqual(requiredParameter.value, 'test')
        requiredParameter.setValue('check')
        self.assertEqual(requiredParameter.value, 'check')
        self.assertFalse(requiredParameter.setValue(None))
        self.assertEqual(requiredParameter.value, 'check')


class ParameterTableFieldTest(unittest.TestCase):

    def testOptional(self):
        parent_name = 'test_parent_layer'
        test_data = points2()
        test_layer = QgsVectorLayer(test_data, parent_name, 'ogr')
        parent = ParameterVector(parent_name, parent_name)
        parent.setValue(test_layer)
        parameter = ParameterTableField(
            'myName', 'myDesc', parent_name, optional=True)


class ParameterTableMultipleFieldTest(unittest.TestCase):

    def setUp(self):
        self.parent_name = 'test_parent_layer'
        test_data = points2()
        test_layer = QgsVectorLayer(test_data, self.parent_name, 'ogr')
        self.parent = ParameterVector(self.parent_name,
                                      self.parent_name)
        self.parent.setValue(test_layer)

    def testGetAsScriptCode(self):
        parameter = ParameterTableMultipleField(
            'myName', 'myDesc', self.parent_name, optional=False)

        self.assertEqual(
            parameter.getAsScriptCode(),
            '##myName=multiple field test_parent_layer')

        # test optional
        parameter.optional = True
        self.assertEqual(
            parameter.getAsScriptCode(),
            '##myName=optional multiple field test_parent_layer')

    def testOptional(self):
        parameter = ParameterTableMultipleField(
            'myName', 'myDesc', self.parent_name, optional=True)

        parameter.setValue(['my', 'super', 'widget', '77'])
        self.assertEqual(parameter.value, 'my;super;widget;77')

        parameter.setValue([])
        self.assertEqual(parameter.value, None)

        parameter.setValue(None)
        self.assertEqual(parameter.value, None)

        parameter.setValue(['my', 'widget', '77'])
        self.assertEqual(parameter.value, 'my;widget;77')

    def testSetValue(self):
        parameter = ParameterTableMultipleField(
            'myName', 'myDesc', self.parent_name, optional=False)

        parameter.setValue(['my', 'super', 'widget', '77'])
        self.assertEqual(parameter.value, 'my;super;widget;77')

        parameter.setValue([])
        self.assertEqual(parameter.value, 'my;super;widget;77')

        parameter.setValue(None)
        self.assertEqual(parameter.value, 'my;super;widget;77')

        parameter.setValue(['my', 'widget', '77'])
        self.assertEqual(parameter.value, 'my;widget;77')

if __name__ == '__main__':
    unittest.main()
