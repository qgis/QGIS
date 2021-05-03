# -*- coding: utf-8 -*-

"""
***************************************************************************
    ParametersTest
    ---------------------
    Date                 : May 2021
    Copyright            : (C) 2021 by René-Luc DHONT
    Email                : rldhont at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'René-Luc DHONT'
__date__ = 'May 2021'
__copyright__ = '(C) 2021, René-Luc DHONT'

import os
import shutil

from qgis.core import QgsProcessingParameterDefinition
from qgis.testing import start_app, unittest

from processing.core.parameters import getParameterFromString

testDataPath = os.path.join(os.path.dirname(__file__), 'testdata')

start_app()


class ParametersTest(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.cleanup_paths = []

    @classmethod
    def tearDownClass(cls):
        for path in cls.cleanup_paths:
            shutil.rmtree(path)

    def testParameterStringDesc(self):
        desc = 'QgsProcessingParameterString|in_string|Input String'
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), 'string')
        self.assertEqual(param.name(), 'in_string')
        self.assertEqual(param.description(), 'Input String')
        self.assertFalse(param.flags() & QgsProcessingParameterDefinition.FlagOptional)

        desc = 'QgsProcessingParameterString|in_string|Input String|default value'
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), 'string')
        self.assertEqual(param.name(), 'in_string')
        self.assertEqual(param.description(), 'Input String')
        self.assertEqual(param.defaultValue(), 'default value')
        self.assertFalse(param.flags() & QgsProcessingParameterDefinition.FlagOptional)

        desc = 'QgsProcessingParameterString|in_string|Input String|default value|True'
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), 'string')
        self.assertEqual(param.name(), 'in_string')
        self.assertEqual(param.description(), 'Input String')
        self.assertEqual(param.defaultValue(), 'default value')
        self.assertTrue(param.multiLine())
        self.assertFalse(param.flags() & QgsProcessingParameterDefinition.FlagOptional)

        desc = 'QgsProcessingParameterString|in_string|Input String||False|True'
        param = getParameterFromString(desc)
        self.assertIsNotNone(param)
        self.assertEqual(param.type(), 'string')
        self.assertEqual(param.name(), 'in_string')
        self.assertEqual(param.description(), 'Input String')
        self.assertEqual(param.defaultValue(), '')
        self.assertFalse(param.multiLine())
        self.assertTrue(param.flags() & QgsProcessingParameterDefinition.FlagOptional)


if __name__ == '__main__':
    unittest.main()
