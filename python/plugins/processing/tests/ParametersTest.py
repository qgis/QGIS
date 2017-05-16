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

import sys
from inspect import isclass
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
                                        ParameterTable,
                                        ParameterTableField,
                                        ParameterSelection,
                                        ParameterExpression,
                                        getParameterFromString)
from processing.tools import dataobjects
from processing.tests.TestData import points

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

    def testScriptCode(self):
        """Simple check that default constructed object export/import correctly"""
        paramClasses = [c for c in list(sys.modules[__name__].__dict__.values())
                        if isclass(c) and issubclass(c, Parameter) and c != Parameter]

        for paramClass in paramClasses:
            param = paramClass()
            if hasattr(param, 'getAsScriptCode'):
                code = param.getAsScriptCode()
                importedParam = paramClass.fromScriptCode(code)
                self.assertEqual(param.optional, importedParam.optional)
                self.assertEqual(param.default, importedParam.default, param)


class ParameterBooleanTest(unittest.TestCase):

    def testScriptCode(self):
        parameter = ParameterBoolean('myName', 'myDescription')
        code = parameter.getAsScriptCode()
        result = getParameterFromString(code)
        self.assertIsInstance(result, ParameterBoolean)
        self.assertFalse(result.optional)

        parameter.optional = True
        code = parameter.getAsScriptCode()
        result = getParameterFromString(code)
        self.assertIsInstance(result, ParameterBoolean)
        self.assertTrue(result.optional)


class ParameterCRSTest(unittest.TestCase):

    def testScriptCode(self):
        parameter = ParameterCrs('myName', 'myDescription')
        code = parameter.getAsScriptCode()
        result = getParameterFromString(code)
        self.assertIsInstance(result, ParameterCrs)

        parameter.optional = True
        code = parameter.getAsScriptCode()
        result = getParameterFromString(code)
        self.assertIsInstance(result, ParameterCrs)
        self.assertTrue(result.optional)


class ParameterDataObjectTest(unittest.TestCase):

    def testGetValueAsCommandLineParameter(self):
        parameter = ParameterDataObject('myName', 'myDesc')
        parameter.setValue(None)
        self.assertEqual(parameter.getValueAsCommandLineParameter(), "None")

        parameter = ParameterDataObject('myName', 'myDesc')
        parameter.setValue("someFile.dat")
        self.assertEqual(parameter.getValueAsCommandLineParameter(), '"someFile.dat"')


class ParameterExtentTest(unittest.TestCase):

    def testScriptCode(self):
        parameter = ParameterExtent('myName', 'myDescription')
        code = parameter.getAsScriptCode()
        result = getParameterFromString(code)
        self.assertIsInstance(result, ParameterExtent)

        parameter.optional = True
        code = parameter.getAsScriptCode()
        result = getParameterFromString(code)
        self.assertIsInstance(result, ParameterExtent)
        self.assertTrue(result.optional)


class ParameterPointTest(unittest.TestCase):

    def testScriptCode(self):
        parameter = ParameterPoint('myName', 'myDescription')
        code = parameter.getAsScriptCode()
        result = getParameterFromString(code)
        self.assertIsInstance(result, ParameterPoint)

        parameter.optional = True
        code = parameter.getAsScriptCode()
        result = getParameterFromString(code)
        self.assertIsInstance(result, ParameterPoint)
        self.assertTrue(result.optional)


class ParameterSelectionTest(unittest.TestCase):

    def testTupleOptions(self):
        options = (
            ('o1', 'option1'),
            ('o2', 'option2'),
            ('o3', 'option3'))

        optionalParameter = ParameterSelection('myName', 'myDesc', options, default='o1')
        self.assertEqual(optionalParameter.value, 'o1')
        optionalParameter.setValue('o2')
        self.assertEqual(optionalParameter.value, 'o2')

        optionalParameter = ParameterSelection('myName', 'myDesc', options, default=['o1', 'o2'], multiple=True)
        self.assertEqual(optionalParameter.value, ['o1', 'o2'])
        optionalParameter.setValue(['o2'])
        self.assertEqual(optionalParameter.value, ['o2'])


class ParameterFileTest(unittest.TestCase):

    def testGetValueAsCommandLineParameter(self):
        parameter = ParameterFile('myName', 'myDesc')
        parameter.setValue('myFile.png')
        self.assertEqual(parameter.getValueAsCommandLineParameter(), '"myFile.png"')

    def testScriptCode(self):
        parameter = ParameterFile('myName', 'myDescription')
        code = parameter.getAsScriptCode()
        result = getParameterFromString(code)
        self.assertIsInstance(result, ParameterFile)

        parameter.optional = True
        code = parameter.getAsScriptCode()
        result = getParameterFromString(code)
        self.assertIsInstance(result, ParameterFile)
        self.assertTrue(result.optional)


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


class ParameterMultipleInputTest(unittest.TestCase):

    def testGetAsStringWhenRaster(self):
        parameter = ParameterMultipleInput('myName', 'myDesc', datatype=dataobjects.TYPE_RASTER)

        # With Path
        self.assertEqual(parameter.getAsString('/some/path'), '/some/path')

        # With Layer
        layer = QgsRasterLayer('/path/to/myRaster.tif', 'myRaster')
        self.assertEqual(parameter.getAsString(layer), '/path/to/myRaster.tif')

        # TODO With Layer Name, instead of Layer object

    def testGetAsStringWhenFile(self):
        parameter = ParameterMultipleInput('myName', 'myDesc', datatype=dataobjects.TYPE_FILE)
        self.assertEqual(parameter.getAsString('/some/path'), '/some/path')

    def testGetAsStringWhenVector(self):
        parameter = ParameterMultipleInput('myName', 'myDesc', datatype=dataobjects.TYPE_VECTOR_ANY)

        # With Path
        self.assertEqual(parameter.getAsString('/some/path'), '/some/path')

        # With Layer
        layer = QgsVectorLayer('/path/to/myVector.shp', 'myVector', 'memory')
        self.assertEqual(parameter.getAsString(layer), '/path/to/myVector.shp')

        # TODO With Layer Name, instead of Layer object

    def testScriptCode(self):
        parameter = ParameterMultipleInput('myName', 'myDescription')
        code = parameter.getAsScriptCode()
        result = getParameterFromString(code)
        self.assertIsInstance(result, ParameterMultipleInput)

        parameter.optional = True
        code = parameter.getAsScriptCode()
        result = getParameterFromString(code)
        self.assertIsInstance(result, ParameterMultipleInput)
        self.assertTrue(result.optional)


class ParameterNumberTest(unittest.TestCase):

    def testScriptCode(self):
        parameter = ParameterNumber('myName', 'myDescription')
        code = parameter.getAsScriptCode()
        result = getParameterFromString(code)
        self.assertIsInstance(result, ParameterNumber)

        parameter.optional = True
        code = parameter.getAsScriptCode()
        result = getParameterFromString(code)
        self.assertIsInstance(result, ParameterNumber)
        self.assertTrue(result.optional)


class ParameterStringTest(unittest.TestCase):

    def testScriptCode(self):
        parameter = ParameterString('myName', 'myDescription', default='test')
        code = parameter.getAsScriptCode()
        result = getParameterFromString(code)
        self.assertIsInstance(result, ParameterString)
        self.assertEqual(result.default, parameter.default)

        parameter.default = None
        parameter.optional = True
        code = parameter.getAsScriptCode()
        result = getParameterFromString(code)
        self.assertIsInstance(result, ParameterString)
        self.assertTrue(result.optional)
        self.assertEqual(result.default, parameter.default)

        parameter.default = 'None'
        code = parameter.getAsScriptCode()
        result = getParameterFromString(code)
        self.assertIsInstance(result, ParameterString)
        self.assertTrue(result.optional)
        self.assertEqual(result.default, parameter.default)

        parameter.default = 'It\'s Mario'
        code = parameter.getAsScriptCode()
        result = getParameterFromString(code)
        self.assertIsInstance(result, ParameterString)
        self.assertTrue(result.optional)
        self.assertEqual(result.default, parameter.default)


class ParameterExpressionTest(unittest.TestCase):

    def testScriptCode(self):
        parameter = ParameterExpression('myName', 'myDescription', default='test')
        code = parameter.getAsScriptCode()
        result = getParameterFromString(code)
        self.assertIsInstance(result, ParameterExpression)

        parameter.optional = True
        code = parameter.getAsScriptCode()
        result = getParameterFromString(code)
        self.assertIsInstance(result, ParameterExpression)
        self.assertTrue(result.optional)
        self.assertEqual(result.default, parameter.default)


class ParameterTableFieldTest(unittest.TestCase):

    def testOptional(self):
        parent_name = 'test_parent_layer'
        test_data = points()
        test_layer = QgsVectorLayer(test_data, parent_name, 'ogr')
        parent = ParameterVector(parent_name, parent_name)
        parent.setValue(test_layer)
        ParameterTableField('myName', 'myDesc', parent_name, optional=True)

    def testScriptCode(self):
        parent_name = 'test_parent_layer'
        test_data = points()
        test_layer = QgsVectorLayer(test_data, parent_name, 'ogr')  # NOQA
        parameter = ParameterTableField('myName', 'myDesc', parent_name)
        code = parameter.getAsScriptCode()
        result = getParameterFromString(code)
        self.assertIsInstance(result, ParameterTableField)

        parameter.optional = True
        code = parameter.getAsScriptCode()
        result = getParameterFromString(code)
        self.assertIsInstance(result, ParameterTableField)
        self.assertTrue(result.optional)


class ParameterTableTest(unittest.TestCase):

    def testScriptCode(self):
        parameter = ParameterTable(
            'myName', 'myDesc')
        code = parameter.getAsScriptCode()
        result = getParameterFromString(code)
        self.assertIsInstance(result, ParameterTable)

        parameter.optional = True
        code = parameter.getAsScriptCode()
        result = getParameterFromString(code)
        self.assertIsInstance(result, ParameterTable)
        self.assertTrue(result.optional)


if __name__ == '__main__':
    unittest.main()
