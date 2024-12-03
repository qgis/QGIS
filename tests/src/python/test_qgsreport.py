"""QGIS Unit tests for QgsReport

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "29/12/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsFeature,
    QgsLayout,
    QgsProject,
    QgsReadWriteContext,
    QgsReport,
    QgsReportSectionFieldGroup,
    QgsReportSectionLayout,
    QgsUnitTypes,
    QgsVectorLayer,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsReport(QgisTestCase):

    def testGettersSetters(self):
        p = QgsProject()
        r = QgsReport(p)

        self.assertEqual(r.layoutProject(), p)
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

    def testchildSections(self):
        p = QgsProject()
        r = QgsReport(p)
        self.assertEqual(r.childCount(), 0)
        self.assertEqual(r.childSections(), [])
        self.assertIsNone(r.childSection(-1))
        self.assertIsNone(r.childSection(1))
        self.assertIsNone(r.childSection(0))

        # try deleting non-existent childSections
        r.removeChildAt(-1)
        r.removeChildAt(0)
        r.removeChildAt(100)
        r.removeChild(None)

        # append child
        child1 = QgsReportSectionLayout()
        self.assertIsNone(child1.project())
        r.appendChild(child1)
        self.assertEqual(r.childCount(), 1)
        self.assertEqual(r.childSections(), [child1])
        self.assertEqual(r.childSection(0), child1)
        self.assertEqual(child1.parentSection(), r)
        self.assertEqual(child1.row(), 0)
        self.assertEqual(child1.project(), p)
        child2 = QgsReportSectionLayout()
        r.appendChild(child2)
        self.assertEqual(r.childCount(), 2)
        self.assertEqual(r.childSections(), [child1, child2])
        self.assertEqual(r.childSection(1), child2)
        self.assertEqual(child2.parentSection(), r)
        self.assertEqual(child2.row(), 1)

    def testInsertChild(self):
        p = QgsProject()
        r = QgsReport(p)

        child1 = QgsReportSectionLayout()
        r.insertChild(11, child1)
        self.assertEqual(r.childCount(), 1)
        self.assertEqual(r.childSections(), [child1])
        self.assertEqual(child1.parentSection(), r)
        self.assertEqual(child1.row(), 0)
        child2 = QgsReportSectionLayout()
        r.insertChild(-1, child2)
        self.assertEqual(r.childCount(), 2)
        self.assertEqual(r.childSections(), [child2, child1])
        self.assertEqual(child2.parentSection(), r)
        self.assertEqual(child2.row(), 0)
        self.assertEqual(child1.row(), 1)

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
        self.assertEqual(r.childSections(), [child1, child2])

        r.removeChildAt(1)
        self.assertEqual(r.childCount(), 1)
        self.assertEqual(r.childSections(), [child1])

        r.removeChild(child1)
        self.assertEqual(r.childCount(), 0)
        self.assertEqual(r.childSections(), [])

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
        self.assertTrue(cloned.childSection(0).headerEnabled())
        self.assertFalse(cloned.childSection(0).footerEnabled())
        self.assertEqual(cloned.childSection(0).parentSection(), cloned)
        self.assertFalse(cloned.childSection(1).headerEnabled())
        self.assertTrue(cloned.childSection(1).footerEnabled())
        self.assertEqual(cloned.childSection(1).parentSection(), cloned)

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
        self.assertEqual(r.filePath("/tmp/myreport", "png"), "/tmp/myreport_0001.png")
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_header)
        self.assertEqual(r.filePath("/tmp/myreport", "png"), "/tmp/myreport_0002.png")
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_body)
        self.assertEqual(r.filePath("/tmp/myreport", ".png"), "/tmp/myreport_0003.png")
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_footer)
        self.assertEqual(r.filePath("/tmp/myreport", "jpg"), "/tmp/myreport_0004.jpg")
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_header)
        self.assertEqual(r.filePath("/tmp/myreport", "png"), "/tmp/myreport_0005.png")
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2a_header)
        self.assertEqual(r.filePath("/tmp/myreport", "png"), "/tmp/myreport_0006.png")
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2a_footer)
        self.assertEqual(r.filePath("/tmp/myreport", "png"), "/tmp/myreport_0007.png")
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_footer)
        self.assertEqual(r.filePath("/tmp/myreport", "png"), "/tmp/myreport_0008.png")
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), report_footer)
        self.assertEqual(r.filePath("/tmp/myreport", "png"), "/tmp/myreport_0009.png")
        self.assertFalse(r.next())

    def testFieldGroup(self):
        # create a layer
        ptLayer = QgsVectorLayer(
            "Point?crs=epsg:4326&field=country:string(20)&field=state:string(20)&field=town:string(20)",
            "points",
            "memory",
        )

        attributes = [
            ["Australia", "QLD", "Brisbane"],
            ["Australia", "QLD", "Emerald"],
            ["NZ", "state1", "town1"],
            ["Australia", "VIC", "Melbourne"],
            ["NZ", "state1", "town2"],
            ["Australia", "QLD", "Beerburrum"],
            ["Australia", "VIC", "Geelong"],
            ["NZ", "state2", "town2"],
            ["PNG", "state1", "town1"],
            ["Australia", "NSW", "Sydney"],
        ]

        pr = ptLayer.dataProvider()
        for a in attributes:
            f = QgsFeature()
            f.initAttributes(3)
            f.setAttribute(0, a[0])
            f.setAttribute(1, a[1])
            f.setAttribute(2, a[2])
            self.assertTrue(pr.addFeature(f))

        p = QgsProject()
        r = QgsReport(p)

        # add a child
        child1 = QgsReportSectionFieldGroup()
        child1_body = QgsLayout(p)
        child1.setLayer(ptLayer)
        child1.setBody(child1_body)
        child1.setBodyEnabled(True)
        child1.setField("country")
        r.appendChild(child1)
        self.assertTrue(r.beginRender())
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "QLD", "Brisbane"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "QLD", "Emerald"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "VIC", "Melbourne"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "QLD", "Beerburrum"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "VIC", "Geelong"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "NSW", "Sydney"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state1", "town1"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state1", "town2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state2", "town2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["PNG", "state1", "town1"],
        )
        self.assertFalse(r.next())

        # another group
        # remove body from child1
        child1.setBodyEnabled(False)

        child2 = QgsReportSectionFieldGroup()
        child2_body = QgsLayout(p)
        child2.setLayer(ptLayer)
        child2.setBody(child2_body)
        child2.setBodyEnabled(True)
        child2.setField("state")
        child1.appendChild(child2)
        self.assertTrue(r.beginRender())
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "NSW", "Sydney"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "QLD", "Brisbane"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "QLD", "Emerald"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "QLD", "Beerburrum"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "VIC", "Melbourne"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "VIC", "Geelong"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state1", "town1"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state1", "town2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state2", "town2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["PNG", "state1", "town1"],
        )
        self.assertFalse(r.next())

        # another group
        # remove body from child1
        child2.setBodyEnabled(False)

        child3 = QgsReportSectionFieldGroup()
        child3_body = QgsLayout(p)
        child3.setLayer(ptLayer)
        child3.setBody(child3_body)
        child3.setBodyEnabled(True)
        child3.setField("town")
        child3.setSortAscending(False)
        child2.appendChild(child3)
        self.assertTrue(r.beginRender())
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "NSW", "Sydney"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "QLD", "Emerald"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "QLD", "Brisbane"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "QLD", "Beerburrum"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "VIC", "Melbourne"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "VIC", "Geelong"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state1", "town2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state1", "town1"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state2", "town2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["PNG", "state1", "town1"],
        )
        self.assertFalse(r.next())

        # add headers/footers
        child3_header = QgsLayout(p)
        child3.setHeader(child3_header)
        child3.setHeaderEnabled(True)
        child3_footer = QgsLayout(p)
        child3.setFooter(child3_footer)
        child3.setFooterEnabled(True)

        self.assertTrue(r.beginRender())
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "NSW", "Sydney"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "NSW", "Sydney"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "NSW", "Sydney"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "QLD", "Emerald"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "QLD", "Emerald"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "QLD", "Brisbane"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "QLD", "Beerburrum"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "QLD", "Beerburrum"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "VIC", "Melbourne"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "VIC", "Melbourne"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "VIC", "Geelong"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "VIC", "Geelong"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state1", "town2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state1", "town2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state1", "town1"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state1", "town1"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state2", "town2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state2", "town2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state2", "town2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["PNG", "state1", "town1"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["PNG", "state1", "town1"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["PNG", "state1", "town1"],
        )
        self.assertFalse(r.next())

        # header/footer for section2
        child2_header = QgsLayout(p)
        child2.setHeader(child2_header)
        child2.setHeaderEnabled(True)
        child2_footer = QgsLayout(p)
        child2.setFooter(child2_footer)
        child2.setFooterEnabled(True)

        self.assertTrue(r.beginRender())
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes()[:2], ["Australia", "NSW"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "NSW", "Sydney"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "NSW", "Sydney"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "NSW", "Sydney"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "QLD", "Emerald"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "QLD", "Emerald"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "QLD", "Brisbane"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "QLD", "Beerburrum"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "QLD", "Beerburrum"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "VIC", "Melbourne"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "VIC", "Melbourne"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "VIC", "Geelong"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes()[:2], ["Australia", "VIC"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes()[:2], ["Australia", "VIC"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes()[:2], ["NZ", "state1"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state1", "town2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state1", "town2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state1", "town1"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state1", "town1"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state2", "town2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state2", "town2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state2", "town2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes()[:2], ["NZ", "state2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes()[:2], ["PNG", "state1"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["PNG", "state1", "town1"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["PNG", "state1", "town1"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["PNG", "state1", "town1"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes()[:2], ["PNG", "state1"]
        )
        self.assertFalse(r.next())

        # child 1 and report header/footer
        child1_header = QgsLayout(p)
        child1.setHeader(child1_header)
        child1.setHeaderEnabled(True)
        child1_footer = QgsLayout(p)
        child1.setFooter(child1_footer)
        child1.setFooterEnabled(True)
        report_header = QgsLayout(p)
        r.setHeader(report_header)
        r.setHeaderEnabled(True)
        report_footer = QgsLayout(p)
        r.setFooter(report_footer)
        r.setFooterEnabled(True)

        self.assertTrue(r.beginRender())
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), report_header)
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes()[:1], ["Australia"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes()[:2], ["Australia", "NSW"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "NSW", "Sydney"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "NSW", "Sydney"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "NSW", "Sydney"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "QLD", "Emerald"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "QLD", "Emerald"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "QLD", "Brisbane"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "QLD", "Beerburrum"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "QLD", "Beerburrum"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "VIC", "Melbourne"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "VIC", "Melbourne"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "VIC", "Geelong"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["Australia", "VIC", "Geelong"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes()[:2], ["Australia", "VIC"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes()[:2], ["NZ", "state1"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state1", "town2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state1", "town2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state1", "town1"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state1", "town1"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state2", "town2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state2", "town2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state2", "town2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes()[:2], ["NZ", "state2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes()[:2], ["PNG", "state1"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["PNG", "state1", "town1"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["PNG", "state1", "town1"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(),
            ["PNG", "state1", "town1"],
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes()[:2], ["PNG", "state1"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_footer)
        self.assertEqual(r.layout().reportContext().feature().attributes()[:1], ["PNG"])
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), report_footer)
        self.assertFalse(r.next())

    def testFieldGroupSectionVisibility(self):
        states = QgsVectorLayer(
            "Point?crs=epsg:4326&field=country:string(20)&field=state:string(20)",
            "points",
            "memory",
        )

        p = QgsProject()
        r = QgsReport(p)

        # add a child
        child1 = QgsReportSectionFieldGroup()
        child1.setLayer(states)
        child1.setField("country")
        child1_header = QgsLayout(p)
        child1.setHeader(child1_header)
        child1.setHeaderEnabled(True)
        child1_footer = QgsLayout(p)
        child1.setFooter(child1_footer)
        child1.setFooterEnabled(True)
        r.appendChild(child1)

        # check that no header was rendered when no features are found
        self.assertTrue(r.beginRender())
        self.assertFalse(r.next())

        child1.setHeaderVisibility(
            QgsReportSectionFieldGroup.SectionVisibility.AlwaysInclude
        )
        child1.setFooterVisibility(
            QgsReportSectionFieldGroup.SectionVisibility.AlwaysInclude
        )

        # check that the header is included when no features are found
        self.assertTrue(r.beginRender())
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_header)
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_footer)

    def testFieldGroupMultiLayer(self):
        # create a layer
        states = QgsVectorLayer(
            "Point?crs=epsg:4326&field=country:string(20)&field=state:string(20)",
            "points",
            "memory",
        )

        attributes = [
            ["Australia", "QLD"],
            ["NZ", "state1"],
            ["Australia", "VIC"],
            ["NZ", "state2"],
            ["PNG", "state3"],
            ["Australia", "NSW"],
        ]

        pr = states.dataProvider()
        for a in attributes:
            f = QgsFeature()
            f.initAttributes(2)
            f.setAttribute(0, a[0])
            f.setAttribute(1, a[1])
            self.assertTrue(pr.addFeature(f))

        places = QgsVectorLayer(
            "Point?crs=epsg:4326&field=state:string(20)&field=town:string(20)",
            "points",
            "memory",
        )

        attributes = [
            ["QLD", "Brisbane"],
            ["QLD", "Emerald"],
            ["state1", "town1"],
            ["VIC", "Melbourne"],
            ["state1", "town2"],
            ["QLD", "Beerburrum"],
            ["VIC", "Geelong"],
            ["state3", "town1"],
        ]

        pr = places.dataProvider()
        for a in attributes:
            f = QgsFeature()
            f.initAttributes(2)
            f.setAttribute(0, a[0])
            f.setAttribute(1, a[1])
            self.assertTrue(pr.addFeature(f))

        p = QgsProject()
        r = QgsReport(p)

        # add a child
        child1 = QgsReportSectionFieldGroup()
        child1_body = QgsLayout(p)
        child1.setLayer(states)
        child1.setBody(child1_body)
        child1.setBodyEnabled(True)
        child1.setField("country")
        r.appendChild(child1)
        self.assertTrue(r.beginRender())
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["Australia", "QLD"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["Australia", "VIC"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["Australia", "NSW"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state1"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child1_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["PNG", "state3"]
        )
        self.assertFalse(r.next())

        # another group
        # remove body from child1
        child1.setBodyEnabled(False)

        child2 = QgsReportSectionFieldGroup()
        child2_body = QgsLayout(p)
        child2.setLayer(states)
        child2.setBody(child2_body)
        child2.setBodyEnabled(True)
        child2.setField("state")
        child1.appendChild(child2)
        self.assertTrue(r.beginRender())
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["Australia", "NSW"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["Australia", "QLD"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["Australia", "VIC"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state1"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["PNG", "state3"]
        )
        self.assertFalse(r.next())

        # another group

        child3 = QgsReportSectionFieldGroup()
        child3_body = QgsLayout(p)
        child3.setLayer(places)
        child3.setBody(child3_body)
        child3.setBodyEnabled(True)
        child3.setField("town")
        child3.setSortAscending(False)
        child2.appendChild(child3)
        self.assertTrue(r.beginRender())
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["Australia", "NSW"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["Australia", "QLD"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["QLD", "Emerald"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["QLD", "Brisbane"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["QLD", "Beerburrum"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["Australia", "VIC"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["VIC", "Melbourne"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["VIC", "Geelong"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state1"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["state1", "town2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["state1", "town1"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["PNG", "state3"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["state3", "town1"]
        )
        self.assertFalse(r.next())

        # add headers/footers
        child3_header = QgsLayout(p)
        child3.setHeader(child3_header)
        child3.setHeaderEnabled(True)
        child3_footer = QgsLayout(p)
        child3.setFooter(child3_footer)
        child3.setFooterEnabled(True)

        self.assertTrue(r.beginRender())
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["Australia", "NSW"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["Australia", "QLD"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["QLD", "Emerald"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["QLD", "Emerald"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["QLD", "Brisbane"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["QLD", "Beerburrum"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["QLD", "Beerburrum"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["Australia", "VIC"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["VIC", "Melbourne"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["VIC", "Melbourne"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["VIC", "Geelong"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["VIC", "Geelong"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state1"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["state1", "town2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["state1", "town2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["state1", "town1"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["state1", "town1"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["NZ", "state2"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child2_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["PNG", "state3"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_header)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["state3", "town1"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_body)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["state3", "town1"]
        )
        self.assertTrue(r.next())
        self.assertEqual(r.layout(), child3_footer)
        self.assertEqual(
            r.layout().reportContext().feature().attributes(), ["state3", "town1"]
        )
        self.assertFalse(r.next())

    def testReadWriteXml(self):
        p = QgsProject()
        ptLayer = QgsVectorLayer(
            "Point?crs=epsg:4326&field=country:string(20)&field=state:string(20)&field=town:string(20)",
            "points",
            "memory",
        )
        p.addMapLayer(ptLayer)

        r = QgsReport(p)
        r.setName("my report")
        # add a header
        r.setHeaderEnabled(True)
        report_header = QgsLayout(p)
        report_header.setUnits(QgsUnitTypes.LayoutUnit.LayoutInches)
        r.setHeader(report_header)
        # add a footer
        r.setFooterEnabled(True)
        report_footer = QgsLayout(p)
        report_footer.setUnits(QgsUnitTypes.LayoutUnit.LayoutMeters)
        r.setFooter(report_footer)

        # add some subsections
        child1 = QgsReportSectionLayout()
        child1_body = QgsLayout(p)
        child1_body.setUnits(QgsUnitTypes.LayoutUnit.LayoutPoints)
        child1.setBody(child1_body)

        child2 = QgsReportSectionLayout()
        child2_body = QgsLayout(p)
        child2_body.setUnits(QgsUnitTypes.LayoutUnit.LayoutPixels)
        child2.setBody(child2_body)
        child1.appendChild(child2)

        child2a = QgsReportSectionFieldGroup()
        child2a_body = QgsLayout(p)
        child2a_body.setUnits(QgsUnitTypes.LayoutUnit.LayoutInches)
        child2a.setBody(child2a_body)
        child2a.setField("my field")
        child2a.setLayer(ptLayer)
        child1.appendChild(child2a)

        r.appendChild(child1)

        doc = QDomDocument("testdoc")
        elem = r.writeLayoutXml(doc, QgsReadWriteContext())

        r2 = QgsReport(p)
        self.assertTrue(r2.readLayoutXml(elem, doc, QgsReadWriteContext()))
        self.assertEqual(r2.name(), "my report")
        self.assertTrue(r2.headerEnabled())
        self.assertEqual(r2.header().units(), QgsUnitTypes.LayoutUnit.LayoutInches)
        self.assertTrue(r2.footerEnabled())
        self.assertEqual(r2.footer().units(), QgsUnitTypes.LayoutUnit.LayoutMeters)

        self.assertEqual(r2.childCount(), 1)
        self.assertEqual(
            r2.childSection(0).body().units(), QgsUnitTypes.LayoutUnit.LayoutPoints
        )
        self.assertEqual(r2.childSection(0).childCount(), 2)
        self.assertEqual(
            r2.childSection(0).childSection(0).body().units(),
            QgsUnitTypes.LayoutUnit.LayoutPixels,
        )
        self.assertEqual(
            r2.childSection(0).childSection(1).body().units(),
            QgsUnitTypes.LayoutUnit.LayoutInches,
        )
        self.assertEqual(r2.childSection(0).childSection(1).field(), "my field")
        self.assertEqual(r2.childSection(0).childSection(1).layer(), ptLayer)


if __name__ == "__main__":
    unittest.main()
