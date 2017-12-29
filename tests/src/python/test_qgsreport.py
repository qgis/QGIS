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
        r = QgsReport(p)

        self.assertEqual(r.project(), p)

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
        r = QgsReport(p)
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
        self.assertIsNone(child1.project())
        r.appendChild(child1)
        self.assertEqual(r.childCount(), 1)
        self.assertEqual(r.children(), [child1])
        self.assertEqual(r.child(0), child1)
        self.assertEqual(child1.parent(), r)
        self.assertEqual(child1.project(), p)
        child2 = QgsReportSectionLayout()
        r.appendChild(child2)
        self.assertEqual(r.childCount(), 2)
        self.assertEqual(r.children(), [child1, child2])
        self.assertEqual(r.child(1), child2)
        self.assertEqual(child2.parent(), r)

    def testInsertChild(self):
        p = QgsProject()
        r = QgsReport(p)

        child1 = QgsReportSectionLayout()
        r.insertChild(11, child1)
        self.assertEqual(r.childCount(), 1)
        self.assertEqual(r.children(), [child1])
        self.assertEqual(child1.parent(), r)
        child2 = QgsReportSectionLayout()
        r.insertChild(-1, child2)
        self.assertEqual(r.childCount(), 2)
        self.assertEqual(r.children(), [child2, child1])
        self.assertEqual(child2.parent(), r)

    def testRemoveChild(self):
        p = QgsProject()
        r = QgsReport(p)

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
        r = QgsReport(p)

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
        self.assertEqual(cloned.child(0).parent(), cloned)
        self.assertFalse(cloned.child(1).headerEnabled())
        self.assertTrue(cloned.child(1).footerEnabled())
        self.assertEqual(cloned.child(1).parent(), cloned)

    def testReportSectionLayout(self):
        r = QgsReportSectionLayout()
        p = QgsProject()
        body = QgsLayout(p)
        r.setBody(body)
        self.assertEqual(r.body(), body)

    def testIteration(self):
        p = QgsProject()
        r = QgsReport(p)

        # empty report
        self.assertTrue(r.beginRender())
        self.assertFalse(r.next())

        # add a header
        r.setHeaderEnabled(True)
        report_header = QgsLayout(p)
        r.setHeader(report_header)

        self.assertTrue(r.beginRender())
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), report_header)
        self.assertFalse(r.next())

        # add a footer
        r.setFooterEnabled(True)
        report_footer = QgsLayout(p)
        r.setFooter(report_footer)

        self.assertTrue(r.beginRender())
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), report_header)
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), report_footer)
        self.assertFalse(r.next())

        # add a child
        child1 = QgsReportSectionLayout()
        child1_body = QgsLayout(p)
        child1.setBody(child1_body)
        r.appendChild(child1)
        self.assertTrue(r.beginRender())
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), report_header)
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_body)
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), report_footer)
        self.assertFalse(r.next())

        # header and footer on child
        child1_header = QgsLayout(p)
        child1.setHeader(child1_header)
        child1.setHeaderEnabled(True)
        child1_footer = QgsLayout(p)
        child1.setFooter(child1_footer)
        child1.setFooterEnabled(True)
        self.assertTrue(r.beginRender())
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), report_header)
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_header)
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_body)
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_footer)
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), report_footer)
        self.assertFalse(r.next())

        # add another child
        child2 = QgsReportSectionLayout()
        child2_header = QgsLayout(p)
        child2.setHeader(child2_header)
        child2.setHeaderEnabled(True)
        child2_footer = QgsLayout(p)
        child2.setFooter(child2_footer)
        child2.setFooterEnabled(True)
        r.appendChild(child2)
        self.assertTrue(r.beginRender())
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), report_header)
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_header)
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_body)
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_footer)
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_header)
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_footer)
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), report_footer)
        self.assertFalse(r.next())

        # add a child to child2
        child2a = QgsReportSectionLayout()
        child2a_header = QgsLayout(p)
        child2a.setHeader(child2a_header)
        child2a.setHeaderEnabled(True)
        child2a_footer = QgsLayout(p)
        child2a.setFooter(child2a_footer)
        child2a.setFooterEnabled(True)
        child2.appendChild(child2a)
        self.assertTrue(r.beginRender())
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), report_header)
        self.assertEqual(r.filePath('/tmp/myreport', 'png'), '/tmp/myreport_0001.png')
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_header)
        self.assertEqual(r.filePath('/tmp/myreport', 'png'), '/tmp/myreport_0002.png')
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_body)
        self.assertEqual(r.filePath('/tmp/myreport', '.png'), '/tmp/myreport_0003.png')
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_footer)
        self.assertEqual(r.filePath('/tmp/myreport', 'jpg'), '/tmp/myreport_0004.jpg')
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_header)
        self.assertEqual(r.filePath('/tmp/myreport', 'png'), '/tmp/myreport_0005.png')
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2a_header)
        self.assertEqual(r.filePath('/tmp/myreport', 'png'), '/tmp/myreport_0006.png')
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2a_footer)
        self.assertEqual(r.filePath('/tmp/myreport', 'png'), '/tmp/myreport_0007.png')
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_footer)
        self.assertEqual(r.filePath('/tmp/myreport', 'png'), '/tmp/myreport_0008.png')
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), report_footer)
        self.assertEqual(r.filePath('/tmp/myreport', 'png'), '/tmp/myreport_0009.png')
        self.assertFalse(r.next())


if __name__ == '__main__':
    unittest.main()
