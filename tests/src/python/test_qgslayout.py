# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayout

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '18/07/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA
from qgis.PyQt import sip
import tempfile
import shutil
import os

from qgis.core import (QgsUnitTypes,
                       QgsLayout,
                       QgsLayoutItemPage,
                       QgsLayoutGuide,
                       QgsLayoutObject,
                       QgsProject,
                       QgsPrintLayout,
                       QgsLayoutItemGroup,
                       QgsLayoutItem,
                       QgsLayoutItemHtml,
                       QgsProperty,
                       QgsLayoutPageCollection,
                       QgsLayoutMeasurement,
                       QgsLayoutFrame,
                       QgsFillSymbol,
                       QgsReadWriteContext,
                       QgsLayoutItemMap,
                       QgsLayoutItemLabel,
                       QgsLayoutSize,
                       QgsLayoutPoint)
from qgis.PyQt.QtCore import Qt, QCoreApplication, QEvent, QPointF, QRectF
from qgis.PyQt.QtTest import QSignalSpy
from qgis.PyQt.QtXml import QDomDocument

from qgis.testing import start_app, unittest

start_app()


class TestQgsLayout(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        cls.basetestpath = tempfile.mkdtemp()

    @classmethod
    def tearDownClass(cls):
        """Run after all tests"""
        shutil.rmtree(cls.basetestpath, True)

    def testReadWriteXml(self):
        p = QgsProject()
        l = QgsPrintLayout(p)
        l.setName('my layout')
        l.setUnits(QgsUnitTypes.LayoutInches)
        collection = l.pageCollection()

        # add a page
        page = QgsLayoutItemPage(l)
        page.setPageSize('A6')
        collection.addPage(page)

        grid = l.gridSettings()
        grid.setResolution(QgsLayoutMeasurement(5, QgsUnitTypes.LayoutPoints))

        g1 = QgsLayoutGuide(Qt.Horizontal, QgsLayoutMeasurement(5, QgsUnitTypes.LayoutCentimeters),
                            l.pageCollection().page(0))
        l.guides().addGuide(g1)

        snapper = l.snapper()
        snapper.setSnapTolerance(7)

        # add some items
        item1 = QgsLayoutItemMap(l)
        item1.setId('xxyyxx')
        l.addItem(item1)
        item2 = QgsLayoutItemMap(l)
        item2.setId('zzyyzz')
        l.addItem(item2)

        l.setReferenceMap(item2)

        doc = QDomDocument("testdoc")
        elem = l.writeXml(doc, QgsReadWriteContext())

        l2 = QgsPrintLayout(p)
        self.assertTrue(l2.readXml(elem, doc, QgsReadWriteContext()))
        self.assertEqual(l2.name(), 'my layout')
        self.assertEqual(l2.units(), QgsUnitTypes.LayoutInches)

        collection2 = l2.pageCollection()
        self.assertEqual(collection2.pageCount(), 1)
        self.assertAlmostEqual(collection2.page(0).pageSize().width(), 105, 4)
        self.assertEqual(collection2.page(0).pageSize().height(), 148)
        self.assertEqual(l2.gridSettings().resolution().length(), 5.0)
        self.assertEqual(l2.gridSettings().resolution().units(), QgsUnitTypes.LayoutPoints)
        self.assertEqual(l2.guides().guidesOnPage(0)[0].orientation(), Qt.Horizontal)
        self.assertEqual(l2.guides().guidesOnPage(0)[0].position().length(), 5.0)
        self.assertEqual(l2.guides().guidesOnPage(0)[0].position().units(), QgsUnitTypes.LayoutCentimeters)
        self.assertEqual(l2.snapper().snapTolerance(), 7)

        # check restored items
        new_item1 = l2.itemByUuid(item1.uuid())
        self.assertTrue(new_item1)
        self.assertEqual(new_item1.id(), 'xxyyxx')
        new_item2 = l2.itemByUuid(item2.uuid())
        self.assertTrue(new_item2)
        self.assertEqual(new_item2.id(), 'zzyyzz')
        self.assertEqual(l2.referenceMap().id(), 'zzyyzz')

    def testAddItemsFromXml(self):
        p = QgsProject()
        l = QgsLayout(p)

        # add some items
        item1 = QgsLayoutItemLabel(l)
        item1.setId('xxyyxx')
        item1.attemptMove(QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        item1.attemptResize(QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        l.addItem(item1)
        item2 = QgsLayoutItemLabel(l)
        item2.setId('zzyyzz')
        item2.attemptMove(QgsLayoutPoint(1.4, 1.8, QgsUnitTypes.LayoutCentimeters))
        item2.attemptResize(QgsLayoutSize(2.8, 2.2, QgsUnitTypes.LayoutCentimeters))
        l.addItem(item2)

        doc = QDomDocument("testdoc")
        # store in xml
        elem = l.writeXml(doc, QgsReadWriteContext())

        l2 = QgsLayout(p)
        new_items = l2.addItemsFromXml(elem, doc, QgsReadWriteContext())
        self.assertEqual(len(new_items), 2)
        items = l2.items()
        self.assertTrue([i for i in items if i.id() == 'xxyyxx'])
        self.assertTrue([i for i in items if i.id() == 'zzyyzz'])
        self.assertTrue(new_items[0] in l2.items())
        self.assertTrue(new_items[1] in l2.items())
        new_item1 = [i for i in items if i.id() == 'xxyyxx'][0]
        new_item2 = [i for i in items if i.id() == 'zzyyzz'][0]
        self.assertEqual(new_item1.positionWithUnits(), QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(new_item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(new_item2.positionWithUnits(), QgsLayoutPoint(1.4, 1.8, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(new_item2.sizeWithUnits(), QgsLayoutSize(2.8, 2.2, QgsUnitTypes.LayoutCentimeters))

        # test with a group
        group = QgsLayoutItemGroup(l)
        group.addItem(item1)
        group.addItem(item2)
        l.addLayoutItem(group)
        elem = l.writeXml(doc, QgsReadWriteContext())

        l3 = QgsLayout(p)
        new_items = l3.addItemsFromXml(elem, doc, QgsReadWriteContext())
        self.assertEqual(len(new_items), 3)
        items = l3.items()
        self.assertTrue([i for i in items if i.id() == 'xxyyxx'])
        self.assertTrue([i for i in items if i.id() == 'zzyyzz'])
        self.assertTrue(new_items[0] in l3.items())
        self.assertTrue(new_items[1] in l3.items())
        self.assertTrue(new_items[2] in l3.items())

        # f*** you sip, I'll just manually cast
        new_group = sip.cast(l3.itemByUuid(group.uuid()), QgsLayoutItemGroup)
        self.assertIsNotNone(new_group)
        other_items = [i for i in new_items if i.type() != new_group.type()]
        self.assertCountEqual(new_group.items(), other_items)

        # test restoring at set position
        l3 = QgsLayout(p)
        new_items = l3.addItemsFromXml(elem, doc, QgsReadWriteContext(), QPointF(10, 30))
        self.assertEqual(len(new_items), 3)
        items = l3.items()
        new_item1 = [i for i in items if i.id() == 'xxyyxx'][0]
        new_item2 = [i for i in items if i.id() == 'zzyyzz'][0]
        self.assertEqual(new_item1.positionWithUnits(), QgsLayoutPoint(10, 30, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(new_item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(new_item2.positionWithUnits(), QgsLayoutPoint(2.0, 4.0, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(new_item2.sizeWithUnits(), QgsLayoutSize(2.8, 2.2, QgsUnitTypes.LayoutCentimeters))

        # paste in place
        l4 = QgsLayout(p)
        page = QgsLayoutItemPage(l)
        page.setPageSize('A3')
        l4.pageCollection().addPage(page)
        page = QgsLayoutItemPage(l)
        page.setPageSize('A6')
        l4.pageCollection().addPage(page)

        new_items = l4.addItemsFromXml(elem, doc, QgsReadWriteContext(), QPointF(10, 30), True)
        self.assertEqual(len(new_items), 3)
        new_item1 = [i for i in new_items if i.id() == 'xxyyxx'][0]
        new_item2 = [i for i in new_items if i.id() == 'zzyyzz'][0]
        self.assertEqual(new_item1.pagePositionWithUnits(), QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(new_item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(new_item1.page(), 0)
        self.assertEqual(new_item2.pagePositionWithUnits(), QgsLayoutPoint(1.4, 1.8, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(new_item2.sizeWithUnits(), QgsLayoutSize(2.8, 2.2, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(new_item2.page(), 0)

        # paste in place, page 2
        new_items = l4.addItemsFromXml(elem, doc, QgsReadWriteContext(), QPointF(10, 550), True)
        self.assertEqual(len(new_items), 3)
        new_item1 = [i for i in new_items if i.id() == 'xxyyxx'][0]
        new_item2 = [i for i in new_items if i.id() == 'zzyyzz'][0]
        self.assertEqual(new_item1.pagePositionWithUnits(), QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(new_item1.page(), 1)
        self.assertEqual(new_item1.sizeWithUnits(), QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        self.assertEqual(new_item2.pagePositionWithUnits(), QgsLayoutPoint(1.4, 1.8, QgsUnitTypes.LayoutCentimeters))
        self.assertEqual(new_item2.page(), 1)
        self.assertEqual(new_item2.sizeWithUnits(), QgsLayoutSize(2.8, 2.2, QgsUnitTypes.LayoutCentimeters))

        #TODO - test restoring multiframe

    def testSaveLoadTemplate(self):
        tmpfile = os.path.join(self.basetestpath, 'testTemplate.qpt')

        p = QgsProject()
        l = QgsLayout(p)
        l.initializeDefaults()

        # add some items
        item1 = QgsLayoutItemLabel(l)
        item1.setId('xxyyxx')
        item1.attemptMove(QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        item1.attemptResize(QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        l.addItem(item1)
        item2 = QgsLayoutItemLabel(l)
        item2.setId('zzyyzz')
        item2.attemptMove(QgsLayoutPoint(1.4, 1.8, QgsUnitTypes.LayoutCentimeters))
        item2.attemptResize(QgsLayoutSize(2.8, 2.2, QgsUnitTypes.LayoutCentimeters))
        l.addItem(item2)

        # multiframe
        multiframe1 = QgsLayoutItemHtml(l)
        multiframe1.setHtml('mf1')
        l.addMultiFrame(multiframe1)
        frame1 = QgsLayoutFrame(l, multiframe1)
        frame1.setId('frame1')
        frame1.attemptMove(QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        frame1.attemptResize(QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        multiframe1.addFrame(frame1)

        multiframe2 = QgsLayoutItemHtml(l)
        multiframe2.setHtml('mf2')
        l.addMultiFrame(multiframe2)
        frame2 = QgsLayoutFrame(l, multiframe2)
        frame2.setId('frame2')
        frame2.attemptMove(QgsLayoutPoint(1.4, 1.8, QgsUnitTypes.LayoutCentimeters))
        frame2.attemptResize(QgsLayoutSize(2.8, 2.2, QgsUnitTypes.LayoutCentimeters))
        multiframe2.addFrame(frame2)

        uuids = {item1.uuid(), item2.uuid(), frame1.uuid(), frame2.uuid(), multiframe1.uuid(), multiframe2.uuid()}
        original_uuids = {item1.uuid(), item2.uuid(), frame1.uuid(), frame2.uuid()}

        self.assertTrue(l.saveAsTemplate(tmpfile, QgsReadWriteContext()))

        l2 = QgsLayout(p)
        with open(tmpfile) as f:
            template_content = f.read()
        doc = QDomDocument()
        doc.setContent(template_content)

        # adding to existing items
        new_items, ok = l2.loadFromTemplate(doc, QgsReadWriteContext(), False)
        self.assertTrue(ok)
        self.assertEqual(len(new_items), 4)
        items = l2.items()
        multiframes = l2.multiFrames()
        self.assertEqual(len(multiframes), 2)
        self.assertTrue([i for i in items if i.id() == 'xxyyxx'])
        self.assertTrue([i for i in items if i.id() == 'zzyyzz'])
        self.assertTrue([i for i in items if i.id() == 'frame1'])
        self.assertTrue([i for i in items if i.id() == 'frame2'])
        self.assertTrue([i for i in multiframes if i.html() == 'mf1'])
        self.assertTrue([i for i in multiframes if i.html() == 'mf2'])
        self.assertTrue(new_items[0] in l2.items())
        self.assertTrue(new_items[1] in l2.items())
        self.assertTrue(new_items[2] in l2.items())
        self.assertTrue(new_items[3] in l2.items())

        # double check that new items have a unique uid
        self.assertNotIn(new_items[0].uuid(), uuids)
        uuids.add(new_items[0].uuid())
        self.assertNotIn(new_items[1].uuid(), uuids)
        uuids.add(new_items[1].uuid())
        self.assertNotIn(new_items[2].uuid(), uuids)
        uuids.add(new_items[2].uuid())
        self.assertNotIn(new_items[3].uuid(), uuids)
        uuids.add(new_items[3].uuid())

        self.assertNotIn(multiframes[0].uuid(), [multiframe1.uuid(), multiframe2.uuid()])
        self.assertNotIn(multiframes[1].uuid(), [multiframe1.uuid(), multiframe2.uuid()])
        new_multiframe1 = [i for i in multiframes if i.html() == 'mf1'][0]
        self.assertEqual(new_multiframe1.layout(), l2)
        new_multiframe2 = [i for i in multiframes if i.html() == 'mf2'][0]
        self.assertEqual(new_multiframe2.layout(), l2)
        new_frame1 = sip.cast([i for i in items if i.id() == 'frame1'][0], QgsLayoutFrame)
        new_frame2 = sip.cast([i for i in items if i.id() == 'frame2'][0], QgsLayoutFrame)
        self.assertEqual(new_frame1.multiFrame(), new_multiframe1)
        self.assertEqual(new_multiframe1.frames()[0].uuid(), new_frame1.uuid())
        self.assertEqual(new_frame2.multiFrame(), new_multiframe2)
        self.assertEqual(new_multiframe2.frames()[0].uuid(), new_frame2.uuid())

        # adding to existing items
        new_items2, ok = l2.loadFromTemplate(doc, QgsReadWriteContext(), False)
        self.assertTrue(ok)
        self.assertEqual(len(new_items2), 4)
        items = l2.items()
        self.assertEqual(len(items), 8)
        multiframes2 = l2.multiFrames()
        self.assertEqual(len(multiframes2), 4)
        multiframes2 = [m for m in l2.multiFrames() if not m.uuid() in [new_multiframe1.uuid(), new_multiframe2.uuid()]]
        self.assertEqual(len(multiframes2), 2)
        self.assertTrue([i for i in items if i.id() == 'xxyyxx'])
        self.assertTrue([i for i in items if i.id() == 'zzyyzz'])
        self.assertTrue([i for i in items if i.id() == 'frame1'])
        self.assertTrue([i for i in items if i.id() == 'frame2'])
        self.assertTrue([i for i in multiframes2 if i.html() == 'mf1'])
        self.assertTrue([i for i in multiframes2 if i.html() == 'mf2'])
        self.assertTrue(new_items[0] in l2.items())
        self.assertTrue(new_items[1] in l2.items())
        self.assertTrue(new_items[2] in l2.items())
        self.assertTrue(new_items[3] in l2.items())
        self.assertTrue(new_items2[0] in l2.items())
        self.assertTrue(new_items2[1] in l2.items())
        self.assertTrue(new_items2[2] in l2.items())
        self.assertTrue(new_items2[3] in l2.items())
        self.assertNotIn(new_items2[0].uuid(), uuids)
        uuids.add(new_items[0].uuid())
        self.assertNotIn(new_items2[1].uuid(), uuids)
        uuids.add(new_items[1].uuid())
        self.assertNotIn(new_items2[2].uuid(), uuids)
        uuids.add(new_items[2].uuid())
        self.assertNotIn(new_items2[3].uuid(), uuids)
        uuids.add(new_items[3].uuid())

        self.assertNotIn(multiframes2[0].uuid(), [multiframe1.uuid(), multiframe2.uuid(), new_multiframe1.uuid(), new_multiframe2.uuid()])
        self.assertNotIn(multiframes2[1].uuid(), [multiframe1.uuid(), multiframe2.uuid(), new_multiframe1.uuid(), new_multiframe2.uuid()])

        new_multiframe1b = [i for i in multiframes2 if i.html() == 'mf1'][0]
        self.assertEqual(new_multiframe1b.layout(), l2)
        new_multiframe2b = [i for i in multiframes2 if i.html() == 'mf2'][0]
        self.assertEqual(new_multiframe2b.layout(), l2)
        new_frame1b = sip.cast([i for i in items if i.id() == 'frame1' and i.uuid() != new_frame1.uuid()][0], QgsLayoutFrame)
        new_frame2b = sip.cast([i for i in items if i.id() == 'frame2' and i.uuid() != new_frame2.uuid()][0], QgsLayoutFrame)
        self.assertEqual(new_frame1b.multiFrame(), new_multiframe1b)
        self.assertEqual(new_multiframe1b.frames()[0].uuid(), new_frame1b.uuid())
        self.assertEqual(new_frame2b.multiFrame(), new_multiframe2b)
        self.assertEqual(new_multiframe2b.frames()[0].uuid(), new_frame2b.uuid())

        # clearing existing items
        new_items3, ok = l2.loadFromTemplate(doc, QgsReadWriteContext(), True)
        new_multiframes = l2.multiFrames()
        self.assertTrue(ok)
        self.assertEqual(len(new_items3), 5) # includes page
        self.assertEqual(len(new_multiframes), 2)
        items = l2.items()
        self.assertTrue([i for i in items if isinstance(i, QgsLayoutItem) and i.id() == 'xxyyxx'])
        self.assertTrue([i for i in items if isinstance(i, QgsLayoutItem) and i.id() == 'zzyyzz'])
        self.assertTrue([i for i in items if isinstance(i, QgsLayoutItem) and i.id() == 'frame1'])
        self.assertTrue([i for i in items if isinstance(i, QgsLayoutItem) and i.id() == 'frame2'])
        self.assertTrue(new_items3[0] in l2.items())
        self.assertTrue(new_items3[1] in l2.items())
        self.assertTrue(new_items3[2] in l2.items())
        self.assertTrue(new_items3[3] in l2.items())
        new_multiframe1 = [i for i in new_multiframes if i.html() == 'mf1'][0]
        new_multiframe2 = [i for i in new_multiframes if i.html() == 'mf2'][0]

        new_frame1 = sip.cast([i for i in items if isinstance(i, QgsLayoutItem) and i.id() == 'frame1'][0], QgsLayoutFrame)
        new_frame2 = sip.cast([i for i in items if isinstance(i, QgsLayoutItem) and i.id() == 'frame2'][0], QgsLayoutFrame)
        self.assertEqual(new_frame1.multiFrame(), new_multiframe1)
        self.assertEqual(new_multiframe1.frames()[0].uuid(), new_frame1.uuid())
        self.assertEqual(new_frame2.multiFrame(), new_multiframe2)
        self.assertEqual(new_multiframe2.frames()[0].uuid(), new_frame2.uuid())

    def testSelectedItems(self):
        p = QgsProject()
        l = QgsLayout(p)

        # add some items
        item1 = QgsLayoutItemMap(l)
        l.addItem(item1)
        item2 = QgsLayoutItemMap(l)
        l.addItem(item2)
        item3 = QgsLayoutItemMap(l)
        l.addItem(item3)

        self.assertFalse(l.selectedLayoutItems())
        item1.setSelected(True)
        self.assertEqual(set(l.selectedLayoutItems()), set([item1]))
        item2.setSelected(True)
        self.assertEqual(set(l.selectedLayoutItems()), set([item1, item2]))
        item3.setSelected(True)
        self.assertEqual(set(l.selectedLayoutItems()), set([item1, item2, item3]))
        item3.setLocked(True)
        self.assertEqual(set(l.selectedLayoutItems(False)), set([item1, item2]))
        self.assertEqual(set(l.selectedLayoutItems(True)), set([item1, item2, item3]))

    def testSelections(self):
        p = QgsProject()
        l = QgsLayout(p)

        # add some items
        item1 = QgsLayoutItemMap(l)
        l.addItem(item1)
        item2 = QgsLayoutItemMap(l)
        l.addItem(item2)
        item3 = QgsLayoutItemMap(l)
        l.addItem(item3)

        select_changed_spy = QSignalSpy(l.selectedItemChanged)
        l.setSelectedItem(None)
        self.assertFalse(l.selectedLayoutItems())
        self.assertEqual(len(select_changed_spy), 1)
        self.assertEqual(select_changed_spy[-1][0], None)

        l.setSelectedItem(item1)
        self.assertEqual(l.selectedLayoutItems(), [item1])
        self.assertEqual(len(select_changed_spy), 2)
        self.assertEqual(select_changed_spy[-1][0], item1)

        l.setSelectedItem(None)
        self.assertFalse(l.selectedLayoutItems())
        self.assertEqual(len(select_changed_spy), 3)
        self.assertEqual(select_changed_spy[-1][0], None)

        l.setSelectedItem(item2)
        self.assertEqual(l.selectedLayoutItems(), [item2])
        self.assertEqual(len(select_changed_spy), 4)
        self.assertEqual(select_changed_spy[-1][0], item2)

        l.deselectAll()
        self.assertFalse(l.selectedLayoutItems())
        self.assertEqual(len(select_changed_spy), 5)
        self.assertEqual(select_changed_spy[-1][0], None)

    def testLayoutItemAt(self):
        p = QgsProject()
        l = QgsLayout(p)

        # add some items
        item1 = QgsLayoutItemMap(l)
        item1.attemptMove(QgsLayoutPoint(4, 8, QgsUnitTypes.LayoutMillimeters))
        item1.attemptResize(QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        l.addItem(item1)

        item2 = QgsLayoutItemMap(l)
        item2.attemptMove(QgsLayoutPoint(6, 10, QgsUnitTypes.LayoutMillimeters))
        item2.attemptResize(QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        l.addItem(item2)

        item3 = QgsLayoutItemMap(l)
        item3.attemptMove(QgsLayoutPoint(8, 12, QgsUnitTypes.LayoutMillimeters))
        item3.attemptResize(QgsLayoutSize(18, 12, QgsUnitTypes.LayoutMillimeters))
        item3.setLocked(True)
        l.addItem(item3)

        self.assertIsNone(l.layoutItemAt(QPointF(0, 0)))
        self.assertIsNone(l.layoutItemAt(QPointF(100, 100)))

        self.assertEqual(l.layoutItemAt(QPointF(5, 9)), item1)
        self.assertEqual(l.layoutItemAt(QPointF(25, 23)), item3)
        self.assertIsNone(l.layoutItemAt(QPointF(25, 23), True))
        self.assertEqual(l.layoutItemAt(QPointF(7, 11)), item2)
        self.assertEqual(l.layoutItemAt(QPointF(9, 13)), item3)
        self.assertEqual(l.layoutItemAt(QPointF(9, 13), True), item2)

        self.assertEqual(l.layoutItemAt(QPointF(9, 13), item3), item2)
        self.assertEqual(l.layoutItemAt(QPointF(9, 13), item2), item1)
        self.assertIsNone(l.layoutItemAt(QPointF(9, 13), item1))
        item2.setLocked(True)
        self.assertEqual(l.layoutItemAt(QPointF(9, 13), item3, True), item1)

    def testStacking(self):
        p = QgsProject()
        l = QgsLayout(p)

        # add some items
        item1 = QgsLayoutItemMap(l)
        l.addLayoutItem(item1)
        item2 = QgsLayoutItemMap(l)
        l.addLayoutItem(item2)
        item3 = QgsLayoutItemMap(l)
        l.addLayoutItem(item3)

        self.assertEqual(item1.zValue(), 1)
        self.assertEqual(item2.zValue(), 2)
        self.assertEqual(item3.zValue(), 3)

        # no effect interactions
        self.assertFalse(l.raiseItem(None))
        self.assertFalse(l.lowerItem(None))
        self.assertFalse(l.moveItemToTop(None))
        self.assertFalse(l.moveItemToBottom(None))

        self.assertEqual(item1.zValue(), 1)
        self.assertEqual(item2.zValue(), 2)
        self.assertEqual(item3.zValue(), 3)

        # raising
        self.assertFalse(l.raiseItem(item3))
        self.assertEqual(item1.zValue(), 1)
        self.assertEqual(item2.zValue(), 2)
        self.assertEqual(item3.zValue(), 3)

        self.assertTrue(l.raiseItem(item2))
        self.assertEqual(item1.zValue(), 1)
        self.assertEqual(item2.zValue(), 3)
        self.assertEqual(item3.zValue(), 2)

        self.assertFalse(l.raiseItem(item2))
        self.assertEqual(item1.zValue(), 1)
        self.assertEqual(item2.zValue(), 3)
        self.assertEqual(item3.zValue(), 2)

        self.assertTrue(l.raiseItem(item1))
        self.assertEqual(item1.zValue(), 2)
        self.assertEqual(item2.zValue(), 3)
        self.assertEqual(item3.zValue(), 1)

        # lower
        self.assertFalse(l.lowerItem(item3))
        self.assertEqual(item1.zValue(), 2)
        self.assertEqual(item2.zValue(), 3)
        self.assertEqual(item3.zValue(), 1)

        self.assertTrue(l.lowerItem(item2))
        self.assertEqual(item1.zValue(), 3)
        self.assertEqual(item2.zValue(), 2)
        self.assertEqual(item3.zValue(), 1)

        self.assertTrue(l.lowerItem(item2))
        self.assertEqual(item1.zValue(), 3)
        self.assertEqual(item2.zValue(), 1)
        self.assertEqual(item3.zValue(), 2)

        # raise to top
        self.assertFalse(l.moveItemToTop(item1))
        self.assertEqual(item1.zValue(), 3)
        self.assertEqual(item2.zValue(), 1)
        self.assertEqual(item3.zValue(), 2)

        self.assertTrue(l.moveItemToTop(item3))
        self.assertEqual(item1.zValue(), 2)
        self.assertEqual(item2.zValue(), 1)
        self.assertEqual(item3.zValue(), 3)

        self.assertTrue(l.moveItemToTop(item2))
        self.assertEqual(item1.zValue(), 1)
        self.assertEqual(item2.zValue(), 3)
        self.assertEqual(item3.zValue(), 2)

        # move to bottom
        self.assertFalse(l.moveItemToBottom(item1))
        self.assertEqual(item1.zValue(), 1)
        self.assertEqual(item2.zValue(), 3)
        self.assertEqual(item3.zValue(), 2)

        self.assertTrue(l.moveItemToBottom(item3))
        self.assertEqual(item1.zValue(), 2)
        self.assertEqual(item2.zValue(), 3)
        self.assertEqual(item3.zValue(), 1)

        self.assertTrue(l.moveItemToBottom(item2))
        self.assertEqual(item1.zValue(), 3)
        self.assertEqual(item2.zValue(), 1)
        self.assertEqual(item3.zValue(), 2)


if __name__ == '__main__':
    unittest.main()
