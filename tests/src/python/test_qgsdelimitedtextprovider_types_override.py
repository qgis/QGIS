# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsDelimitedTextProvider type overrides.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Alessandro Pasotti'
__date__ = '22/11/2021'
__copyright__ = 'Copyright 2021, The QGIS Project'

import qgis  # NOQA

import os

from qgis.PyQt.QtCore import QCoreApplication, QVariant, QUrl, QObject, QTemporaryDir

from qgis.core import (
    QgsVectorLayer,
)

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath, compareWkt, compareUrl

start_app()


class TestQgsDelimitedTextProviderTypesOverride(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        cls.tmp_dir = QTemporaryDir()
        cls.tmp_path = cls.tmp_dir.path()

    def _run_test(self, csv_content, csvt_content='', uri_options=''):

        try:
            self.__text_index += 1
        except:
            self.__text_index = 1

        basename = 'test_type_detection_{}'.format(self.__text_index)

        csv_file = os.path.join(self.tmp_path, basename + '.csv')
        with open(csv_file, 'w+') as f:
            f.write(csv_content)

        if csvt_content:
            csvt_file = os.path.join(self.tmp_path, basename + '.csvt')
            with open(csvt_file, 'w+') as f:
                f.write(csvt_content)

        uri = 'file:///{}'.format(csv_file)
        if uri_options:
            uri += '?{}'.format(uri_options)

        vl = QgsVectorLayer(uri, 'test_{}'.format(basename), 'delimitedtext')
        return vl

    def test_type_override(self):
        """Test type overrides"""

        vl = self._run_test('\n'.join((
            "integer,bool,long,real,text",
            "1,true,9189304972279762602,1.234,text",
            "2,false,,5.678,another text",
        )))
        self.assertTrue(vl.isValid())
        fields = {f.name(): (f.type(), f.typeName()) for f in vl.fields()}
        self.assertEqual(fields, {
            'integer': (QVariant.Int, 'integer'),
            'bool': (QVariant.Bool, 'bool'),
            'long': (QVariant.LongLong, 'longlong'),
            'real': (QVariant.Double, 'double'),
            'text': (QVariant.String, 'text')})

        vl = self._run_test('\n'.join((
            "integer,bool,long,real,text",
            "1,true,9189304972279762602,1.234,text",
            "2,false,,5.678,another text",
        )), uri_options='field=bool:integer&field=integer:double&field=long:double&field=real:text')
        self.assertTrue(vl.isValid())
        fields = {f.name(): (f.type(), f.typeName()) for f in vl.fields()}
        self.assertEqual(fields, {
            'integer': (QVariant.Double, 'double'),
            'bool': (QVariant.Int, 'integer'),
            'long': (QVariant.Double, 'double'),
            'real': (QVariant.String, 'text'),
            'text': (QVariant.String, 'text')})


if __name__ == '__main__':
    unittest.main()
