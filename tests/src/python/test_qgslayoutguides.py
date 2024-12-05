"""QGIS Unit tests for QgsLayoutGuide/QgsLayoutGuideCollection.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "05/07/2017"
__copyright__ = "Copyright 2017, The QGIS Project"

from qgis.PyQt import sip
from qgis.PyQt.QtCore import QModelIndex, Qt
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import (
    QgsLayout,
    QgsLayoutGuide,
    QgsLayoutGuideCollection,
    QgsLayoutGuideProxyModel,
    QgsLayoutItemPage,
    QgsLayoutMeasurement,
    QgsProject,
    QgsReadWriteContext,
    QgsUnitTypes,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestQgsLayoutGuide(QgisTestCase):

    def testGuideGettersSetters(self):
        p = QgsProject()
        l = QgsLayout(p)
        l.initializeDefaults()  # add a page
        g = QgsLayoutGuide(
            Qt.Orientation.Horizontal,
            QgsLayoutMeasurement(5, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
            None,
        )
        self.assertEqual(g.orientation(), Qt.Orientation.Horizontal)
        self.assertEqual(g.position().length(), 5.0)
        self.assertEqual(
            g.position().units(), QgsUnitTypes.LayoutUnit.LayoutCentimeters
        )

        g.setLayout(l)
        self.assertEqual(g.layout(), l)

        position_changed_spy = QSignalSpy(g.positionChanged)
        g.setPosition(QgsLayoutMeasurement(15, QgsUnitTypes.LayoutUnit.LayoutInches))
        self.assertEqual(g.position().length(), 15.0)
        self.assertEqual(g.position().units(), QgsUnitTypes.LayoutUnit.LayoutInches)
        self.assertEqual(len(position_changed_spy), 1)

        page = l.pageCollection().page(0)
        g.setPage(page)
        self.assertEqual(g.page(), page)

    def testUpdateGuide(self):
        p = QgsProject()
        l = QgsLayout(p)
        l.initializeDefaults()  # add a page
        # add a second page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize("A5")
        l.pageCollection().addPage(page2)

        g = QgsLayoutGuide(
            Qt.Orientation.Horizontal,
            QgsLayoutMeasurement(5, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
            l.pageCollection().page(0),
        )
        g.setLayout(l)
        g.update()

        self.assertTrue(g.item().isVisible())
        self.assertEqual(g.item().line().x1(), 0)
        self.assertEqual(g.item().line().y1(), 50)
        self.assertEqual(g.item().line().x2(), 297)
        self.assertEqual(g.item().line().y2(), 50)
        self.assertEqual(g.layoutPosition(), 50)

        g.setPosition(
            QgsLayoutMeasurement(15, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        g.update()
        self.assertTrue(g.item().isVisible())
        self.assertEqual(g.item().line().x1(), 0)
        self.assertEqual(g.item().line().y1(), 15)
        self.assertEqual(g.item().line().x2(), 297)
        self.assertEqual(g.item().line().y2(), 15)
        self.assertEqual(g.layoutPosition(), 15)

        # guide on page2
        g1 = QgsLayoutGuide(
            Qt.Orientation.Horizontal,
            QgsLayoutMeasurement(5, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
            l.pageCollection().page(1),
        )
        g1.setLayout(l)
        g1.update()
        g1.setPosition(
            QgsLayoutMeasurement(15, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        g1.update()
        self.assertTrue(g1.item().isVisible())
        self.assertEqual(g1.item().line().x1(), 0)
        self.assertEqual(g1.item().line().y1(), 235)
        self.assertEqual(g1.item().line().x2(), 148)
        self.assertEqual(g1.item().line().y2(), 235)
        self.assertEqual(g1.layoutPosition(), 235)

        # vertical guide
        g2 = QgsLayoutGuide(
            Qt.Orientation.Vertical,
            QgsLayoutMeasurement(5, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
            l.pageCollection().page(0),
        )
        g2.setLayout(l)
        g2.update()
        self.assertTrue(g2.item().isVisible())
        self.assertEqual(g2.item().line().x1(), 50)
        self.assertEqual(g2.item().line().y1(), 0)
        self.assertEqual(g2.item().line().x2(), 50)
        self.assertEqual(g2.item().line().y2(), 210)
        self.assertEqual(g2.layoutPosition(), 50)

        g.setPage(None)
        g.update()
        self.assertFalse(g.item().isVisible())

        g.setPage(l.pageCollection().page(0))
        g.update()
        self.assertTrue(g.item().isVisible())

        # throw it off the bottom of the page
        g.setPosition(
            QgsLayoutMeasurement(1115, QgsUnitTypes.LayoutUnit.LayoutMillimeters)
        )
        g.update()
        self.assertFalse(g.item().isVisible())

        # guide on page2
        g3 = QgsLayoutGuide(
            Qt.Orientation.Vertical,
            QgsLayoutMeasurement(5, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
            l.pageCollection().page(1),
        )
        g3.setLayout(l)
        g3.update()
        self.assertTrue(g3.item().isVisible())
        self.assertEqual(g3.item().line().x1(), 50)
        self.assertEqual(g3.item().line().y1(), 220)
        self.assertEqual(g3.item().line().x2(), 50)
        self.assertEqual(g3.item().line().y2(), 430)
        self.assertEqual(g3.layoutPosition(), 50)

    def testCollection(self):
        p = QgsProject()
        l = QgsLayout(p)
        l.initializeDefaults()  # add a page
        guides = l.guides()

        # no guides initially
        self.assertEqual(guides.rowCount(QModelIndex()), 0)
        self.assertFalse(
            guides.data(QModelIndex(), QgsLayoutGuideCollection.Roles.OrientationRole)
        )
        self.assertFalse(guides.guides(Qt.Orientation.Horizontal))
        self.assertFalse(guides.guides(Qt.Orientation.Vertical))

        # add a guide
        g1 = QgsLayoutGuide(
            Qt.Orientation.Horizontal,
            QgsLayoutMeasurement(5, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
            l.pageCollection().page(0),
        )
        guides.addGuide(g1)
        self.assertEqual(guides.rowCount(QModelIndex()), 1)
        self.assertEqual(
            guides.data(
                guides.index(0, 0), QgsLayoutGuideCollection.Roles.OrientationRole
            ),
            Qt.Orientation.Horizontal,
        )
        self.assertEqual(
            guides.data(
                guides.index(0, 0), QgsLayoutGuideCollection.Roles.PositionRole
            ),
            5,
        )
        self.assertEqual(
            guides.data(guides.index(0, 0), QgsLayoutGuideCollection.Roles.UnitsRole),
            QgsUnitTypes.LayoutUnit.LayoutCentimeters,
        )
        self.assertEqual(
            guides.data(guides.index(0, 0), QgsLayoutGuideCollection.Roles.PageRole), 0
        )
        self.assertEqual(guides.guides(Qt.Orientation.Horizontal), [g1])
        self.assertFalse(guides.guides(Qt.Orientation.Vertical))
        self.assertEqual(guides.guidesOnPage(0), [g1])
        self.assertEqual(guides.guidesOnPage(1), [])

        g2 = QgsLayoutGuide(
            Qt.Orientation.Horizontal,
            QgsLayoutMeasurement(15),
            l.pageCollection().page(0),
        )
        guides.addGuide(g2)
        self.assertEqual(guides.rowCount(QModelIndex()), 2)
        self.assertEqual(
            guides.data(
                guides.index(1, 0), QgsLayoutGuideCollection.Roles.OrientationRole
            ),
            Qt.Orientation.Horizontal,
        )
        self.assertEqual(
            guides.data(
                guides.index(1, 0), QgsLayoutGuideCollection.Roles.PositionRole
            ),
            15,
        )
        self.assertEqual(
            guides.data(guides.index(1, 0), QgsLayoutGuideCollection.Roles.UnitsRole),
            QgsUnitTypes.LayoutUnit.LayoutMillimeters,
        )
        self.assertEqual(
            guides.data(guides.index(1, 0), QgsLayoutGuideCollection.Roles.PageRole), 0
        )
        self.assertEqual(guides.guides(Qt.Orientation.Horizontal), [g1, g2])
        self.assertFalse(guides.guides(Qt.Orientation.Vertical))
        self.assertEqual(guides.guidesOnPage(0), [g1, g2])

        page2 = QgsLayoutItemPage(l)
        page2.setPageSize("A3")
        l.pageCollection().addPage(page2)
        g3 = QgsLayoutGuide(
            Qt.Orientation.Vertical,
            QgsLayoutMeasurement(35),
            l.pageCollection().page(1),
        )
        guides.addGuide(g3)
        self.assertEqual(guides.rowCount(QModelIndex()), 3)
        self.assertEqual(
            guides.data(
                guides.index(2, 0), QgsLayoutGuideCollection.Roles.OrientationRole
            ),
            Qt.Orientation.Vertical,
        )
        self.assertEqual(
            guides.data(
                guides.index(2, 0), QgsLayoutGuideCollection.Roles.PositionRole
            ),
            35,
        )
        self.assertEqual(
            guides.data(guides.index(2, 0), QgsLayoutGuideCollection.Roles.UnitsRole),
            QgsUnitTypes.LayoutUnit.LayoutMillimeters,
        )
        self.assertEqual(
            guides.data(guides.index(2, 0), QgsLayoutGuideCollection.Roles.PageRole), 1
        )
        self.assertEqual(guides.guides(Qt.Orientation.Horizontal), [g1, g2])
        self.assertEqual(guides.guides(Qt.Orientation.Horizontal, 0), [g1, g2])
        self.assertEqual(guides.guides(Qt.Orientation.Horizontal, 1), [])
        self.assertEqual(guides.guides(Qt.Orientation.Vertical), [g3])
        self.assertEqual(guides.guides(Qt.Orientation.Vertical, 0), [])
        self.assertEqual(guides.guides(Qt.Orientation.Vertical, 1), [g3])
        self.assertEqual(guides.guides(Qt.Orientation.Vertical, 2), [])
        self.assertEqual(guides.guidesOnPage(0), [g1, g2])
        self.assertEqual(guides.guidesOnPage(1), [g3])

    def testDeleteRows(self):
        p = QgsProject()
        l = QgsLayout(p)
        l.initializeDefaults()
        guides = l.guides()

        g1 = QgsLayoutGuide(
            Qt.Orientation.Horizontal,
            QgsLayoutMeasurement(5, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
            l.pageCollection().page(0),
        )
        guides.addGuide(g1)
        g2 = QgsLayoutGuide(
            Qt.Orientation.Horizontal,
            QgsLayoutMeasurement(15),
            l.pageCollection().page(0),
        )
        guides.addGuide(g2)
        g3 = QgsLayoutGuide(
            Qt.Orientation.Vertical,
            QgsLayoutMeasurement(35),
            l.pageCollection().page(0),
        )
        guides.addGuide(g3)

        self.assertTrue(guides.removeRows(1, 1))
        self.assertEqual(guides.guides(Qt.Orientation.Horizontal), [g1])
        self.assertEqual(guides.guides(Qt.Orientation.Vertical), [g3])

        self.assertTrue(guides.removeRows(0, 2))
        self.assertEqual(guides.guides(Qt.Orientation.Horizontal), [])
        self.assertEqual(guides.guides(Qt.Orientation.Vertical), [])

    def testQgsLayoutGuideProxyModel(self):
        p = QgsProject()
        l = QgsLayout(p)
        l.initializeDefaults()  # add a page
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize("A3")
        l.pageCollection().addPage(page2)
        guides = l.guides()

        hoz_filter = QgsLayoutGuideProxyModel(None, Qt.Orientation.Horizontal, 0)
        hoz_filter.setSourceModel(guides)
        hoz_page_1_filter = QgsLayoutGuideProxyModel(None, Qt.Orientation.Horizontal, 1)
        hoz_page_1_filter.setSourceModel(guides)
        vert_filter = QgsLayoutGuideProxyModel(None, Qt.Orientation.Vertical, 0)
        vert_filter.setSourceModel(guides)

        # no guides initially
        self.assertEqual(hoz_filter.rowCount(QModelIndex()), 0)
        self.assertEqual(hoz_page_1_filter.rowCount(QModelIndex()), 0)
        self.assertEqual(vert_filter.rowCount(QModelIndex()), 0)

        # add some guides
        g1 = QgsLayoutGuide(
            Qt.Orientation.Horizontal,
            QgsLayoutMeasurement(5, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
            l.pageCollection().page(0),
        )
        guides.addGuide(g1)
        g2 = QgsLayoutGuide(
            Qt.Orientation.Horizontal,
            QgsLayoutMeasurement(15),
            l.pageCollection().page(1),
        )
        guides.addGuide(g2)
        g3 = QgsLayoutGuide(
            Qt.Orientation.Vertical,
            QgsLayoutMeasurement(35),
            l.pageCollection().page(0),
        )
        guides.addGuide(g3)

        self.assertEqual(hoz_filter.rowCount(QModelIndex()), 1)
        self.assertEqual(
            hoz_filter.data(
                hoz_filter.index(0, 0), QgsLayoutGuideCollection.Roles.PositionRole
            ),
            5,
        )
        self.assertEqual(hoz_page_1_filter.rowCount(QModelIndex()), 1)
        self.assertEqual(
            hoz_page_1_filter.data(
                hoz_page_1_filter.index(0, 0),
                QgsLayoutGuideCollection.Roles.PositionRole,
            ),
            15,
        )
        self.assertEqual(vert_filter.rowCount(QModelIndex()), 1)
        self.assertEqual(
            vert_filter.data(
                vert_filter.index(0, 0), QgsLayoutGuideCollection.Roles.PositionRole
            ),
            35,
        )

        # change page
        hoz_page_1_filter.setPage(0)
        self.assertEqual(hoz_page_1_filter.rowCount(QModelIndex()), 1)
        self.assertEqual(
            hoz_page_1_filter.data(
                hoz_page_1_filter.index(0, 0),
                QgsLayoutGuideCollection.Roles.PositionRole,
            ),
            5,
        )

    def testRemoveGuide(self):
        p = QgsProject()
        l = QgsLayout(p)
        l.initializeDefaults()  # add a page
        guides = l.guides()

        # add a guide
        g1 = QgsLayoutGuide(
            Qt.Orientation.Horizontal,
            QgsLayoutMeasurement(5, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
            l.pageCollection().page(0),
        )
        guides.addGuide(g1)
        self.assertEqual(guides.guides(Qt.Orientation.Horizontal), [g1])
        guides.removeGuide(None)
        self.assertEqual(guides.guides(Qt.Orientation.Horizontal), [g1])
        guides.removeGuide(g1)
        self.assertEqual(guides.guides(Qt.Orientation.Horizontal), [])

    def testClear(self):
        p = QgsProject()
        l = QgsLayout(p)
        l.initializeDefaults()  # add a page
        guides = l.guides()

        # add a guide
        g1 = QgsLayoutGuide(
            Qt.Orientation.Horizontal,
            QgsLayoutMeasurement(5, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
            l.pageCollection().page(0),
        )
        guides.addGuide(g1)
        g2 = QgsLayoutGuide(
            Qt.Orientation.Horizontal,
            QgsLayoutMeasurement(5, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
            l.pageCollection().page(0),
        )
        guides.addGuide(g2)
        self.assertEqual(guides.guides(Qt.Orientation.Horizontal), [g1, g2])
        guides.clear()
        self.assertEqual(guides.guides(Qt.Orientation.Horizontal), [])

    def testApplyToOtherPages(self):
        p = QgsProject()
        l = QgsLayout(p)
        l.initializeDefaults()
        page2 = QgsLayoutItemPage(l)
        page2.setPageSize("A6")
        l.pageCollection().addPage(page2)
        guides = l.guides()

        # add some guides
        g1 = QgsLayoutGuide(
            Qt.Orientation.Horizontal,
            QgsLayoutMeasurement(5),
            l.pageCollection().page(0),
        )
        guides.addGuide(g1)
        g2 = QgsLayoutGuide(
            Qt.Orientation.Vertical, QgsLayoutMeasurement(6), l.pageCollection().page(0)
        )
        guides.addGuide(g2)
        g3 = QgsLayoutGuide(
            Qt.Orientation.Horizontal,
            QgsLayoutMeasurement(190),
            l.pageCollection().page(0),
        )
        guides.addGuide(g3)
        g4 = QgsLayoutGuide(
            Qt.Orientation.Horizontal,
            QgsLayoutMeasurement(1),
            l.pageCollection().page(1),
        )
        guides.addGuide(g4)

        # apply guides from page 0 - should delete g4
        guides.applyGuidesToAllOtherPages(0)
        self.assertEqual(guides.guides(Qt.Orientation.Horizontal, 0), [g1, g3])
        self.assertEqual(guides.guides(Qt.Orientation.Vertical, 0), [g2])
        self.assertTrue(sip.isdeleted(g4))

        # g3 is outside of page 2 bounds - should not be copied
        self.assertEqual(len(guides.guides(Qt.Orientation.Horizontal, 1)), 1)
        self.assertEqual(
            guides.guides(Qt.Orientation.Horizontal, 1)[0].position().length(), 5
        )
        self.assertEqual(len(guides.guides(Qt.Orientation.Vertical, 1)), 1)
        self.assertEqual(
            guides.guides(Qt.Orientation.Vertical, 1)[0].position().length(), 6
        )

        # apply guides from page 1 to 0
        guides.applyGuidesToAllOtherPages(1)
        self.assertTrue(sip.isdeleted(g1))
        self.assertTrue(sip.isdeleted(g2))
        self.assertTrue(sip.isdeleted(g3))
        self.assertEqual(len(guides.guides(Qt.Orientation.Horizontal, 0)), 1)
        self.assertEqual(
            guides.guides(Qt.Orientation.Horizontal, 0)[0].position().length(), 5
        )
        self.assertEqual(len(guides.guides(Qt.Orientation.Vertical, 0)), 1)
        self.assertEqual(
            guides.guides(Qt.Orientation.Vertical, 0)[0].position().length(), 6
        )

    def testSetVisible(self):
        p = QgsProject()
        l = QgsLayout(p)
        l.initializeDefaults()
        guides = l.guides()

        # add some guides
        g1 = QgsLayoutGuide(
            Qt.Orientation.Horizontal,
            QgsLayoutMeasurement(5),
            l.pageCollection().page(0),
        )
        guides.addGuide(g1)
        g2 = QgsLayoutGuide(
            Qt.Orientation.Vertical, QgsLayoutMeasurement(6), l.pageCollection().page(0)
        )
        guides.addGuide(g2)

        guides.setVisible(False)
        self.assertFalse(g1.item().isVisible())
        self.assertFalse(g2.item().isVisible())
        guides.setVisible(True)
        self.assertTrue(g1.item().isVisible())
        self.assertTrue(g2.item().isVisible())

    def testReadWriteXml(self):
        p = QgsProject()
        l = QgsLayout(p)
        l.initializeDefaults()
        guides = l.guides()

        # add some guides
        g1 = QgsLayoutGuide(
            Qt.Orientation.Horizontal,
            QgsLayoutMeasurement(5, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
            l.pageCollection().page(0),
        )
        guides.addGuide(g1)
        g2 = QgsLayoutGuide(
            Qt.Orientation.Vertical,
            QgsLayoutMeasurement(6, QgsUnitTypes.LayoutUnit.LayoutInches),
            l.pageCollection().page(0),
        )
        guides.addGuide(g2)

        guides.setVisible(False)

        doc = QDomDocument("testdoc")
        elem = doc.createElement("test")
        self.assertTrue(guides.writeXml(elem, doc, QgsReadWriteContext()))

        l2 = QgsLayout(p)
        l2.initializeDefaults()
        guides2 = l2.guides()

        self.assertTrue(
            guides2.readXml(elem.firstChildElement(), doc, QgsReadWriteContext())
        )
        guide_list = guides2.guidesOnPage(0)
        self.assertEqual(len(guide_list), 2)

        self.assertEqual(guide_list[0].orientation(), Qt.Orientation.Horizontal)
        self.assertEqual(guide_list[0].position().length(), 5.0)
        self.assertEqual(
            guide_list[0].position().units(), QgsUnitTypes.LayoutUnit.LayoutCentimeters
        )
        self.assertEqual(guide_list[1].orientation(), Qt.Orientation.Vertical)
        self.assertEqual(guide_list[1].position().length(), 6.0)
        self.assertEqual(
            guide_list[1].position().units(), QgsUnitTypes.LayoutUnit.LayoutInches
        )

    def testGuideLayoutPosition(self):
        p = QgsProject()
        l = QgsLayout(p)
        l.initializeDefaults()
        guides = l.guides()

        # add some guides
        g1 = QgsLayoutGuide(
            Qt.Orientation.Horizontal,
            QgsLayoutMeasurement(1, QgsUnitTypes.LayoutUnit.LayoutCentimeters),
            l.pageCollection().page(0),
        )
        guides.addGuide(g1)

        # set position in layout units (mm)
        guides.setGuideLayoutPosition(g1, 50)

        self.assertEqual(g1.position().length(), 5.0)
        self.assertEqual(
            g1.position().units(), QgsUnitTypes.LayoutUnit.LayoutCentimeters
        )


if __name__ == "__main__":
    unittest.main()
