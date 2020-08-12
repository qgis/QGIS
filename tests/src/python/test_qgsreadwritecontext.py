# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsReadWriteContext.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = 'Denis Rouzaud'
__date__ = '28.02.2018'
__copyright__ = 'Copyright 2017, The QGIS Project'

import qgis  # NOQA


from qgis.core import Qgis, QgsReadWriteContext
from qgis.testing import unittest


class TestQgsReadWriteContext(unittest.TestCase):

    def testEnterCategory(self):
        context = QgsReadWriteContext()
        context.pushMessage('msg0', Qgis.Critical)
        with QgsReadWriteContext.enterCategory(context, 'cat1'):
            context.pushMessage('msg1', Qgis.Warning)
            with QgsReadWriteContext.enterCategory(context, 'cat2', "detail2"):
                context.pushMessage('msg2')
            context.pushMessage('msg3')
        context.pushMessage('msg4')

        messages = context.takeMessages()

        self.assertEqual(messages[0].message(), 'msg0')
        self.assertEqual(messages[0].level(), Qgis.Critical)
        self.assertEqual(messages[0].categories(), [])

        self.assertEqual(messages[1].message(), 'msg1')
        self.assertEqual(messages[1].level(), Qgis.Warning)
        self.assertEqual(messages[1].categories(), ['cat1'])

        self.assertEqual(messages[2].message(), 'msg2')
        self.assertEqual(messages[2].categories(), ['cat1', 'cat2 :: detail2'])

        self.assertEqual(messages[3].message(), 'msg3')
        self.assertEqual(messages[3].categories(), ['cat1'])

        self.assertEqual(messages[4].message(), 'msg4')
        self.assertEqual(messages[4].categories(), [])


if __name__ == '__main__':
    unittest.main()
