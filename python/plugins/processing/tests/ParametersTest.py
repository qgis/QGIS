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


if __name__ == '__main__':
    unittest.main()
