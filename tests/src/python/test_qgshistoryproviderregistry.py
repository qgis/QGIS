"""QGIS Unit tests for QgsHistoryProviderRegistry.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nyall Dawson"
__date__ = "18/10/2016"
__copyright__ = "Copyright 2016, The QGIS Project"

from qgis.PyQt import sip
from qgis.PyQt.QtCore import QDate, QDateTime, QTime, QModelIndex
from qgis.PyQt.QtTest import QSignalSpy

from qgis.core import Qgis
from qgis.gui import (
    QgsAbstractHistoryProvider,
    QgsHistoryEntry,
    QgsHistoryProviderRegistry,
    QgsGui,
    QgsHistoryEntryNode,
    QgsHistoryEntryGroup,
    QgsHistoryEntryModel,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class TestEntryNode(QgsHistoryEntryGroup):

    def __init__(self, val):
        super().__init__()
        self.val = val

    def data(self, role):
        return self.val


class TestHistoryProvider(QgsAbstractHistoryProvider):

    def id(self) -> str:
        return "test_provider"

    def createNodeForEntry(self, entry, context):
        return TestEntryNode(entry.entry)

    def updateNodeForEntry(self, node, entry, context):
        node.val = entry.entry

        new_child_node = TestEntryNode("my child")
        node.addChild(new_child_node)
        new_child_node = TestEntryNode("my child 2")
        node.addChild(new_child_node)


class TestHistoryProvider2(QgsAbstractHistoryProvider):

    def id(self) -> str:
        return "test_provider2"


class TestNode(QgsHistoryEntryNode):

    def data(self, role):
        return "test"


class TestGroup(QgsHistoryEntryGroup):

    def data(self, role):
        return "test"


class TestQgsHistoryProviderRegistry(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        # trigger metatype registration
        QgsGui.instance()

    def test_entry(self):
        """
        Test QgsHistoryEntry
        """
        entry = QgsHistoryEntry()
        self.assertFalse(entry.isValid())

        entry = QgsHistoryEntry(
            "my provider", QDateTime(2021, 1, 2, 3, 4, 5), {"somevar": 5}
        )
        self.assertTrue(entry.isValid())

        self.assertEqual(entry.providerId, "my provider")
        self.assertEqual(entry.timestamp, QDateTime(2021, 1, 2, 3, 4, 5))
        self.assertEqual(entry.entry, {"somevar": 5})

        self.assertEqual(
            str(entry), "<QgsHistoryEntry: my provider 2021-01-02T03:04:05>"
        )

        entry = QgsHistoryEntry({"somevar": 7})
        self.assertTrue(entry.isValid())

        self.assertFalse(entry.providerId)
        self.assertEqual(entry.timestamp.date(), QDateTime.currentDateTime().date())
        self.assertEqual(entry.entry, {"somevar": 7})

    def test_registry_providers(self):
        """
        Test registering providers
        """
        reg = QgsHistoryProviderRegistry(useMemoryDatabase=True)
        self.assertFalse(reg.providerIds())

        provider1 = TestHistoryProvider()
        provider2 = TestHistoryProvider2()
        self.assertTrue(reg.addProvider(provider1))
        self.assertEqual(reg.providerIds(), ["test_provider"])

        # already registered
        self.assertFalse(reg.addProvider(provider1))

        self.assertTrue(reg.addProvider(provider2))
        self.assertCountEqual(reg.providerIds(), ["test_provider", "test_provider2"])

        self.assertFalse(reg.removeProvider("x"))
        self.assertTrue(reg.removeProvider("test_provider"))

        self.assertTrue(sip.isdeleted(provider1))

        self.assertEqual(reg.providerIds(), ["test_provider2"])

    def test_registry_entries(self):
        """
        Test storing and retrieving registry entries
        """
        reg = QgsHistoryProviderRegistry(useMemoryDatabase=True)
        self.assertFalse(reg.queryEntries())

        added_spy = QSignalSpy(reg.entryAdded)
        updated_spy = QSignalSpy(reg.entryUpdated)

        id_1, ok = reg.addEntry(
            "my provider",
            {"some var": 5, "other_var": [1, 2, 3], "final_var": {"a": "b"}},
            QgsHistoryProviderRegistry.HistoryEntryOptions(),
        )
        self.assertTrue(ok)

        self.assertEqual(len(added_spy), 1)
        self.assertEqual(added_spy[-1][0], id_1)
        self.assertEqual(added_spy[-1][1].providerId, "my provider")
        self.assertEqual(added_spy[-1][1].id, id_1)
        self.assertEqual(
            added_spy[-1][1].entry,
            {"some var": 5, "other_var": [1, 2, 3], "final_var": {"a": "b"}},
        )

        id_2, ok = reg.addEntry(
            "my provider 2",
            {"some var": 6},
            QgsHistoryProviderRegistry.HistoryEntryOptions(),
        )
        self.assertTrue(ok)
        self.assertEqual(len(added_spy), 2)
        self.assertEqual(added_spy[-1][0], id_2)
        self.assertEqual(added_spy[-1][1].providerId, "my provider 2")
        self.assertEqual(added_spy[-1][1].id, id_2)
        self.assertEqual(added_spy[-1][1].entry, {"some var": 6})

        self.assertEqual(len(reg.queryEntries()), 2)
        self.assertEqual(reg.queryEntries()[0].providerId, "my provider")
        self.assertEqual(reg.queryEntries()[0].id, id_1)
        self.assertEqual(
            reg.queryEntries()[0].timestamp.date(), QDateTime.currentDateTime().date()
        )
        self.assertEqual(
            reg.queryEntries()[0].entry,
            {"some var": 5, "other_var": [1, 2, 3], "final_var": {"a": "b"}},
        )

        self.assertEqual(reg.queryEntries()[1].providerId, "my provider 2")
        self.assertEqual(reg.queryEntries()[1].id, id_2)
        self.assertEqual(
            reg.queryEntries()[1].timestamp.date(), QDateTime.currentDateTime().date()
        )
        self.assertEqual(reg.queryEntries()[1].entry, {"some var": 6})

        entry, ok = reg.entry(1111)
        self.assertFalse(ok)
        entry, ok = reg.entry(id_1)
        self.assertTrue(ok)
        self.assertEqual(entry.providerId, "my provider")
        self.assertEqual(entry.id, id_1)
        self.assertEqual(entry.timestamp.date(), QDateTime.currentDateTime().date())
        self.assertEqual(
            entry.entry,
            {"some var": 5, "other_var": [1, 2, 3], "final_var": {"a": "b"}},
        )
        entry, ok = reg.entry(id_2)
        self.assertTrue(ok)
        self.assertEqual(entry.providerId, "my provider 2")
        self.assertEqual(entry.id, id_2)
        self.assertEqual(entry.timestamp.date(), QDateTime.currentDateTime().date())
        self.assertEqual(entry.entry, {"some var": 6})

        entry = QgsHistoryEntry(
            "my provider 3", QDateTime(2021, 1, 2, 3, 4, 5), {"var": 7}
        )
        id_3, ok = reg.addEntry(entry)
        self.assertTrue(ok)
        self.assertEqual(len(added_spy), 3)
        self.assertEqual(added_spy[-1][0], id_3)
        self.assertEqual(added_spy[-1][1].providerId, "my provider 3")
        self.assertEqual(added_spy[-1][1].id, id_3)
        self.assertEqual(added_spy[-1][1].entry, {"var": 7})

        self.assertEqual(len(reg.queryEntries()), 3)
        self.assertEqual(reg.queryEntries()[0].providerId, "my provider")
        self.assertEqual(reg.queryEntries()[0].id, id_1)
        self.assertEqual(
            reg.queryEntries()[0].timestamp.date(), QDateTime.currentDateTime().date()
        )
        self.assertEqual(
            reg.queryEntries()[0].entry,
            {"some var": 5, "other_var": [1, 2, 3], "final_var": {"a": "b"}},
        )
        self.assertEqual(reg.queryEntries()[1].providerId, "my provider 2")
        self.assertEqual(reg.queryEntries()[1].id, id_2)
        self.assertEqual(
            reg.queryEntries()[1].timestamp.date(), QDateTime.currentDateTime().date()
        )
        self.assertEqual(reg.queryEntries()[1].entry, {"some var": 6})
        self.assertEqual(reg.queryEntries()[2].providerId, "my provider 3")
        self.assertEqual(reg.queryEntries()[2].id, id_3)
        self.assertEqual(reg.queryEntries()[2].timestamp.date(), QDate(2021, 1, 2))
        self.assertEqual(reg.queryEntries()[2].entry, {"var": 7})

        # query by provider
        entries = reg.queryEntries(providerId="my provider")
        self.assertEqual(len(entries), 1)
        self.assertEqual(entries[0].providerId, "my provider")
        self.assertEqual(entries[0].id, id_1)
        self.assertEqual(
            entries[0].timestamp.date(), QDateTime.currentDateTime().date()
        )
        self.assertEqual(
            entries[0].entry,
            {"some var": 5, "other_var": [1, 2, 3], "final_var": {"a": "b"}},
        )

        entries = reg.queryEntries(providerId="my provider 2")
        self.assertEqual(len(entries), 1)
        self.assertEqual(entries[0].providerId, "my provider 2")
        self.assertEqual(entries[0].id, id_2)
        self.assertEqual(
            entries[0].timestamp.date(), QDateTime.currentDateTime().date()
        )
        self.assertEqual(entries[0].entry, {"some var": 6})

        # query by date
        entries = reg.queryEntries(
            start=QDateTime(3022, 1, 2, 3, 4, 5)
        )  # this test will break in 3022, sorry
        self.assertEqual(len(entries), 0)
        entries = reg.queryEntries(end=QDateTime(2020, 1, 2, 3, 4, 5))
        self.assertEqual(len(entries), 0)

        entries = reg.queryEntries(start=QDateTime(2021, 3, 2, 3, 4, 5))
        self.assertEqual(len(entries), 2)
        self.assertCountEqual(
            [e.providerId for e in entries], ["my provider", "my provider 2"]
        )
        entries = reg.queryEntries(end=QDateTime(2021, 3, 2, 3, 4, 5))
        self.assertEqual(len(entries), 1)
        self.assertEqual(entries[0].providerId, "my provider 3")

        # update an entry
        self.assertTrue(reg.updateEntry(id_1, {"new_props": 54}))
        self.assertEqual(len(updated_spy), 1)
        self.assertEqual(updated_spy[-1][0], id_1)
        self.assertEqual(updated_spy[-1][1], {"new_props": 54})

        entry, ok = reg.entry(id_1)
        self.assertEqual(entry.providerId, "my provider")
        self.assertEqual(entry.id, id_1)
        self.assertEqual(entry.timestamp.date(), QDateTime.currentDateTime().date())
        self.assertEqual(entry.entry, {"new_props": 54})

        clear_spy = QSignalSpy(reg.historyCleared)
        self.assertTrue(reg.clearHistory(Qgis.HistoryProviderBackend.LocalProfile))
        self.assertEqual(len(clear_spy), 1)

        self.assertFalse(reg.queryEntries())

        # bulk add entries
        self.assertTrue(
            reg.addEntries(
                [
                    QgsHistoryEntry(
                        "my provider 4", QDateTime(2021, 1, 2, 3, 4, 5), {"var": 7}
                    ),
                    QgsHistoryEntry(
                        "my provider 5", QDateTime(2021, 1, 2, 3, 4, 5), {"var": 8}
                    ),
                ]
            )
        )
        self.assertEqual(len(reg.queryEntries()), 2)

        self.assertEqual(len(added_spy), 5)
        self.assertEqual(added_spy[-2][1].providerId, "my provider 4")
        self.assertEqual(added_spy[-2][1].id, 1)
        self.assertEqual(added_spy[-2][1].entry, {"var": 7})
        self.assertEqual(added_spy[-1][1].providerId, "my provider 5")
        self.assertEqual(added_spy[-1][1].id, 2)
        self.assertEqual(added_spy[-1][1].entry, {"var": 8})

        self.assertEqual(reg.queryEntries()[0].providerId, "my provider 4")
        self.assertEqual(reg.queryEntries()[0].id, 1)
        self.assertEqual(reg.queryEntries()[0].entry, {"var": 7})

        self.assertEqual(reg.queryEntries()[1].providerId, "my provider 5")
        self.assertEqual(reg.queryEntries()[1].id, 2)
        self.assertEqual(reg.queryEntries()[1].entry, {"var": 8})

    def test_nodes(self):
        node = TestNode()
        self.assertEqual(node.childCount(), 0)
        self.assertFalse(node.parent())

        group = TestGroup()
        self.assertEqual(group.childCount(), 0)
        self.assertFalse(group.parent())
        self.assertFalse(group.childAt(0))
        self.assertEqual(group.indexOf(node), -1)

        group.addChild(node)
        self.assertEqual(group.childCount(), 1)
        self.assertFalse(group.parent())
        self.assertEqual(node.parent(), group)
        self.assertEqual(group.childAt(0), node)
        self.assertEqual(group.indexOf(node), 0)

        node2 = TestNode()
        group.addChild(node2)
        self.assertEqual(group.childCount(), 2)
        self.assertFalse(group.parent())
        self.assertEqual(node2.parent(), group)
        self.assertEqual(group.childAt(0), node)
        self.assertEqual(group.childAt(1), node2)
        self.assertEqual(group.indexOf(node), 0)
        self.assertEqual(group.indexOf(node2), 1)

        # insert
        node3 = TestNode()
        group.insertChild(1, node3)
        self.assertEqual(group.childCount(), 3)
        self.assertEqual(node3.parent(), group)
        self.assertEqual(group.childAt(0), node)
        self.assertEqual(group.childAt(1), node3)
        self.assertEqual(group.childAt(2), node2)
        self.assertEqual(group.indexOf(node), 0)
        self.assertEqual(group.indexOf(node3), 1)
        self.assertEqual(group.indexOf(node2), 2)

        node4 = TestNode()
        group.insertChild(0, node4)
        self.assertEqual(group.childCount(), 4)
        self.assertEqual(node4.parent(), group)
        self.assertEqual(group.childAt(0), node4)
        self.assertEqual(group.childAt(1), node)
        self.assertEqual(group.childAt(2), node3)
        self.assertEqual(group.childAt(3), node2)
        self.assertEqual(group.indexOf(node4), 0)
        self.assertEqual(group.indexOf(node), 1)
        self.assertEqual(group.indexOf(node3), 2)
        self.assertEqual(group.indexOf(node2), 3)

        node5 = TestNode()
        group.insertChild(4, node5)
        self.assertEqual(group.childCount(), 5)
        self.assertEqual(node5.parent(), group)
        self.assertEqual(group.childAt(0), node4)
        self.assertEqual(group.childAt(1), node)
        self.assertEqual(group.childAt(2), node3)
        self.assertEqual(group.childAt(3), node2)
        self.assertEqual(group.childAt(4), node5)
        self.assertEqual(group.indexOf(node4), 0)
        self.assertEqual(group.indexOf(node), 1)
        self.assertEqual(group.indexOf(node3), 2)
        self.assertEqual(group.indexOf(node2), 3)
        self.assertEqual(group.indexOf(node5), 4)

        group.clear()
        self.assertEqual(group.childCount(), 0)

    def test_model(self):
        registry = QgsHistoryProviderRegistry()
        provider = TestHistoryProvider()
        registry.addProvider(provider)
        registry.addEntry(provider.id(), {"a": 1})
        registry.addEntry(provider.id(), {"a": 2})
        registry.addEntry(provider.id(), {"a": 3})

        model = QgsHistoryEntryModel(provider.id(), registry=registry)
        # find Today group
        self.assertEqual(model.rowCount(), 1)
        date_group_1_index = model.index(0, 0, QModelIndex())
        self.assertEqual(model.data(date_group_1_index), "Today")

        self.assertEqual(model.rowCount(date_group_1_index), 3)
        entry_1_index = model.index(2, 0, date_group_1_index)
        self.assertEqual(model.data(entry_1_index), {"a": 1})
        entry_2_index = model.index(1, 0, date_group_1_index)
        self.assertEqual(model.data(entry_2_index), {"a": 2})
        entry_3_index = model.index(0, 0, date_group_1_index)
        self.assertEqual(model.data(entry_3_index), {"a": 3})

        # an entry from yesterday
        yesterday = QDateTime.currentDateTime().addDays(-1)
        yesterday_entry = QgsHistoryEntry(provider.id(), yesterday, {"a": 4})
        registry.addEntry(yesterday_entry)

        self.assertEqual(model.rowCount(), 2)
        yesterday_index = model.index(1, 0, QModelIndex())
        self.assertEqual(model.data(yesterday_index), "Yesterday")

        self.assertEqual(model.rowCount(yesterday_index), 1)
        entry_4_index = model.index(0, 0, yesterday_index)
        self.assertEqual(model.data(entry_4_index), {"a": 4})

        # another entry from yesterday
        yesterday_entry2 = QgsHistoryEntry(provider.id(), yesterday, {"a": 5})
        registry.addEntry(yesterday_entry2)

        self.assertEqual(model.data(yesterday_index), "Yesterday")

        self.assertEqual(model.rowCount(yesterday_index), 2)
        self.assertEqual(model.data(entry_4_index), {"a": 4})
        entry_5_index = model.index(0, 0, yesterday_index)
        self.assertEqual(model.data(entry_5_index), {"a": 5})
        self.assertEqual(model.data(entry_4_index), {"a": 4})

        # an entry from an earlier month
        earlier_entry = QgsHistoryEntry(
            provider.id(), QDateTime(QDate(2020, 6, 3), QTime(12, 13, 14)), {"a": 6}
        )
        earlier_entry_id, _ = registry.addEntry(earlier_entry)

        self.assertEqual(model.rowCount(), 3)
        june2020_index = model.index(2, 0, QModelIndex())
        self.assertEqual(model.data(date_group_1_index), "Today")
        self.assertEqual(model.data(yesterday_index), "Yesterday")
        self.assertEqual(model.data(june2020_index), "June 2020")

        self.assertEqual(model.rowCount(june2020_index), 1)
        entry_6_index = model.index(0, 0, june2020_index)
        self.assertEqual(model.data(entry_6_index), {"a": 6})

        # an entry from an earlier month which is later than the previous one
        earlier_entry2 = QgsHistoryEntry(
            provider.id(), QDateTime(QDate(2020, 10, 3), QTime(12, 13, 14)), {"a": 7}
        )
        registry.addEntry(earlier_entry2)

        self.assertEqual(model.rowCount(), 4)
        october2020_index = model.index(2, 0, QModelIndex())
        self.assertEqual(model.data(date_group_1_index), "Today")
        self.assertEqual(model.data(yesterday_index), "Yesterday")
        self.assertEqual(model.data(june2020_index), "June 2020")
        self.assertEqual(model.data(october2020_index), "October 2020")

        self.assertEqual(model.rowCount(october2020_index), 1)
        entry_7_index = model.index(0, 0, october2020_index)
        self.assertEqual(model.data(entry_7_index), {"a": 7})

        # an entry from last week
        last_week = QDateTime.currentDateTime().addDays(-7)
        last_week_entry = QgsHistoryEntry(provider.id(), last_week, {"a": "last_week"})
        registry.addEntry(last_week_entry)

        self.assertEqual(model.rowCount(), 5)
        last_week_index = model.index(2, 0, QModelIndex())
        self.assertEqual(model.data(date_group_1_index), "Today")
        self.assertEqual(model.data(yesterday_index), "Yesterday")
        self.assertEqual(model.data(june2020_index), "June 2020")
        self.assertEqual(model.data(october2020_index), "October 2020")
        self.assertEqual(model.data(last_week_index), "Last 7 days")

        self.assertEqual(model.rowCount(last_week_index), 1)
        entry_last_week_index = model.index(0, 0, last_week_index)
        self.assertEqual(model.data(entry_last_week_index), {"a": "last_week"})

        # update an entry
        registry.updateEntry(earlier_entry_id, {"a": 8})
        entry_6_index = model.index(0, 0, june2020_index)
        self.assertEqual(model.data(entry_6_index), {"a": 8})
        self.assertEqual(model.rowCount(entry_6_index), 2)
        entry_6a_index = model.index(0, 0, entry_6_index)
        self.assertEqual(model.data(entry_6a_index), "my child")
        entry_6b_index = model.index(1, 0, entry_6_index)
        self.assertEqual(model.data(entry_6b_index), "my child 2")

        # clear
        registry.clearHistory(Qgis.HistoryProviderBackend.LocalProfile)
        self.assertEqual(model.rowCount(), 0)


if __name__ == "__main__":
    unittest.main()
