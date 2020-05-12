# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsTextDocument.

Run with: ctest -V -R QgsTextDocument

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '12/05/2020'
__copyright__ = 'Copyright 2020, The QGIS Project'

import qgis  # NOQA

from qgis.core import QgsTextDocument
from qgis.testing import start_app, unittest

start_app()


class TestQgsTextDocument(unittest.TestCase):

    def testSplitLines(self):
        doc = QgsTextDocument.fromHtml(['abc def'])
        self.assertEqual(len(doc), 1)
        self.assertEqual(len(doc[0]), 1)
        self.assertEqual(doc[0][0].text(), 'abc def')
        doc.splitLines(' ')
        self.assertEqual(len(doc), 2)
        self.assertEqual(len(doc[0]), 1)
        self.assertEqual(doc[0][0].text(), 'abc')
        self.assertEqual(len(doc[1]), 1)
        self.assertEqual(doc[1][0].text(), 'def')

        doc = QgsTextDocument.fromHtml(['<span style="color: red">R_ED</span> not <div>red</div>'])
        self.assertEqual(len(doc), 2)
        self.assertEqual(len(doc[0]), 2)
        self.assertEqual(doc[0][0].text(), 'R_ED')
        self.assertEqual(doc[0][1].text(), ' not ')
        self.assertEqual(len(doc[1]), 1)
        self.assertEqual(doc[1][0].text(), 'red')
        doc.splitLines(' ')
        self.assertEqual(len(doc), 4)
        self.assertEqual(len(doc[0]), 1)
        self.assertEqual(doc[0][0].text(), 'R_ED')
        self.assertEqual(len(doc[1]), 1)
        self.assertEqual(doc[1][0].text(), 'not')
        self.assertEqual(len(doc[2]), 1)
        self.assertEqual(doc[2][0].text(), '')
        self.assertEqual(len(doc[3]), 1)
        self.assertEqual(doc[3][0].text(), 'red')

        doc = QgsTextDocument.fromHtml(['<span style="color: red">R_ED</span> not <div>red</div>'])
        doc.splitLines('_')
        self.assertEqual(len(doc), 3)
        self.assertEqual(len(doc[0]), 1)
        self.assertEqual(doc[0][0].text(), 'R')
        self.assertEqual(len(doc[1]), 2)
        self.assertEqual(doc[1][0].text(), 'ED')
        self.assertEqual(doc[1][1].text(), ' not ')
        self.assertEqual(len(doc[2]), 1)
        self.assertEqual(doc[2][0].text(), 'red')


if __name__ == '__main__':
    unittest.main()
