"""
test_qgsoptional.py
                     --------------------------------------
               Date                 : September 2016
               Copyright            : (C) 2016 Matthias Kuhn
               email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""

from qgis.core import QgsExpression, QgsOptionalExpression
from qgis.testing import unittest


class TestQgsOptional(unittest.TestCase):

    def setUp(self):
        """Run before each test."""
        pass

    def tearDown(self):
        """Run after each test."""
        pass

    def testQgsOptionalExpression(self):
        opt = QgsOptionalExpression()
        self.assertFalse(opt.enabled())
        self.assertFalse(opt)

        opt = QgsOptionalExpression(QgsExpression("true"))
        self.assertTrue(opt.enabled())
        self.assertEqual(opt.data().expression(), "true")
        self.assertTrue(opt)
        opt.setEnabled(False)
        self.assertFalse(opt.enabled())
        self.assertFalse(opt)
        # boolean operator not yet working in python
        # self.assertFalse(opt)
        self.assertEqual(opt.data().expression(), "true")
        opt.setEnabled(True)
        self.assertTrue(opt.enabled())
        # self.assertTrue(opt)
        self.assertEqual(opt.data().expression(), "true")
        opt.setData(QgsExpression("xyz"))
        self.assertTrue(opt.enabled())
        self.assertEqual(opt.data().expression(), "xyz")

        opt = QgsOptionalExpression(QgsExpression("true"), False)
        self.assertFalse(opt.enabled())


if __name__ == "__main__":
    unittest.main()
