# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsReport

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '29/12/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsProject,
                       QgsLayout,
                       QgsReport,
                       QgsReportSectionLayout)
from qgis.testing import start_app, unittest

start_app()


class TestQgsReport(unittest.TestCase):

    def testGettersSetters(self):
        p = QgsProject()
        r = QgsReport()

        r.setHeaderEnabled(True)
        self.assertTrue(r.headerEnabled())

        header = QgsLayout(p)
        r.setHeader(header)
        self.assertEqual(r.header(), header)

        r.setFooterEnabled(True)
        self.assertTrue(r.footerEnabled())

        footer = QgsLayout(p)
        r.setFooter(footer)
        self.assertEqual(r.footer(), footer)

    def testChildren(self):
        p = QgsProject()
        r = QgsReport()
        self.assertEqual(r.childCount(), 0)
        self.assertEqual(r.children(), [])
        self.assertIsNone(r.child(-1))
        self.assertIsNone(r.child(1))
        self.assertIsNone(r.child(0))

        # try deleting non-existant children
        r.removeChildAt(-1)
        r.removeChildAt(0)
        r.removeChildAt(100)
        r.removeChild(None)

        # append child
        child1 = QgsReportSectionLayout()
        r.appendChild(child1)
        self.assertEqual(r.childCount(), 1)
        self.assertEqual(r.children(), [child1])
        self.assertEqual(r.child(0), child1)
        child2 = QgsReportSectionLayout()
        r.appendChild(child2)
        self.assertEqual(r.childCount(), 2)
        self.assertEqual(r.children(), [child1, child2])
        self.assertEqual(r.child(1), child2)

    def testInsertChild(self):
        p = QgsProject()
        r = QgsReport()

        child1 = QgsReportSectionLayout()
        r.insertChild(11, child1)
        self.assertEqual(r.childCount(), 1)
        self.assertEqual(r.children(), [child1])
        child2 = QgsReportSectionLayout()
        r.insertChild(-1, child2)
        self.assertEqual(r.childCount(), 2)
        self.assertEqual(r.children(), [child2, child1])

    def testRemoveChild(self):
        p = QgsProject()
        r = QgsReport()

        child1 = QgsReportSectionLayout()
        r.appendChild(child1)
        child2 = QgsReportSectionLayout()
        r.appendChild(child2)

        r.removeChildAt(-1)
        r.removeChildAt(100)
        r.removeChild(None)
        self.assertEqual(r.childCount(), 2)
        self.assertEqual(r.children(), [child1, child2])

        r.removeChildAt(1)
        self.assertEqual(r.childCount(), 1)
        self.assertEqual(r.children(), [child1])

        r.removeChild(child1)
        self.assertEqual(r.childCount(), 0)
        self.assertEqual(r.children(), [])

    def testClone(self):
        p = QgsProject()
        r = QgsReport()

        child1 = QgsReportSectionLayout()
        child1.setHeaderEnabled(True)
        r.appendChild(child1)
        child2 = QgsReportSectionLayout()
        child2.setFooterEnabled(True)
        r.appendChild(child2)

        cloned = r.clone()
        self.assertEqual(cloned.childCount(), 2)
        self.assertTrue(cloned.child(0).headerEnabled())
        self.assertFalse(cloned.child(0).footerEnabled())
        self.assertFalse(cloned.child(1).headerEnabled())
        self.assertTrue(cloned.child(1).footerEnabled())

    def testReportSectionLayout(self):
        r = QgsReportSectionLayout()
        p = QgsProject()
        body = QgsLayout(p)
        r.setBody(body)
        self.assertEqual(r.body(), body)


if __name__ == '__main__':
    unittest.main()
