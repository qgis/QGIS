# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsHistoryProviderRegistry.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '18/10/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'

import qgis  # NOQA switch sip api

from qgis.PyQt import sip
from qgis.PyQt.QtCore import (
    QDateTime,
    QDate
)
from qgis.core import Qgis
from qgis.gui import (
    QgsHistoryEntry,
    QgsHistoryProviderRegistry,
    QgsAbstractHistoryProvider
)

from qgis.testing import start_app, unittest

start_app()


class TestHistoryProvider(QgsAbstractHistoryProvider):

    def id(self) -> str:
        return 'test_provider'


class TestHistoryProvider2(QgsAbstractHistoryProvider):

    def id(self) -> str:
        return 'test_provider2'


class TestQgsHistoryProviderRegistry(unittest.TestCase):

    def test_entry(self):
        """
        Test QgsHistoryEntry
        """
        entry = QgsHistoryEntry('my provider', QDateTime(2021, 1, 2, 3, 4, 5), {'somevar': 5})
        self.assertEqual(entry.providerId, 'my provider')
        self.assertEqual(entry.timestamp, QDateTime(2021, 1, 2, 3, 4, 5))
        self.assertEqual(entry.entry, {'somevar': 5})

        self.assertEqual(str(entry), '<QgsHistoryEntry: my provider 2021-01-02T03:04:05>')

        entry = QgsHistoryEntry({'somevar': 7})
        self.assertFalse(entry.providerId)
        self.assertEqual(entry.timestamp.date(), QDateTime.currentDateTime().date())
        self.assertEqual(entry.entry, {'somevar': 7})

    def test_registry_providers(self):
        """
        Test registering providers
        """
        reg = QgsHistoryProviderRegistry(useMemoryDatabase=True)
        self.assertFalse(reg.providerIds())

        provider1 = TestHistoryProvider()
        provider2 = TestHistoryProvider2()
        self.assertTrue(reg.addProvider(provider1))
        self.assertEqual(reg.providerIds(), ['test_provider'])

        # already registered
        self.assertFalse(reg.addProvider(provider1))

        self.assertTrue(reg.addProvider(provider2))
        self.assertCountEqual(reg.providerIds(), ['test_provider', 'test_provider2'])

        self.assertFalse(reg.removeProvider('x'))
        self.assertTrue(reg.removeProvider('test_provider'))

        self.assertTrue(sip.isdeleted(provider1))

        self.assertEqual(reg.providerIds(), ['test_provider2'])

    def test_registry_entries(self):
        """
        Test storing and retrieving registry entries
        """
        reg = QgsHistoryProviderRegistry(useMemoryDatabase=True)
        self.assertFalse(reg.queryEntries())

        id_1, ok = reg.addEntry('my provider', {'some var': 5, 'other_var': [1, 2, 3], 'final_var': {'a': 'b'}},
                                QgsHistoryProviderRegistry.HistoryEntryOptions())
        self.assertTrue(ok)
        id_2, ok = reg.addEntry('my provider 2', {'some var': 6},
                                QgsHistoryProviderRegistry.HistoryEntryOptions())
        self.assertTrue(ok)

        self.assertEqual(len(reg.queryEntries()), 2)
        self.assertEqual(reg.queryEntries()[0].providerId, 'my provider')
        self.assertEqual(reg.queryEntries()[0].timestamp.date(), QDateTime.currentDateTime().date())
        self.assertEqual(reg.queryEntries()[0].entry, {'some var': 5, 'other_var': [1, 2, 3], 'final_var': {'a': 'b'}})

        self.assertEqual(reg.queryEntries()[1].providerId, 'my provider 2')
        self.assertEqual(reg.queryEntries()[1].timestamp.date(), QDateTime.currentDateTime().date())
        self.assertEqual(reg.queryEntries()[1].entry, {'some var': 6})

        entry, ok = reg.entry(1111)
        self.assertFalse(ok)
        entry, ok = reg.entry(id_1)
        self.assertTrue(ok)
        self.assertEqual(entry.providerId, 'my provider')
        self.assertEqual(entry.timestamp.date(), QDateTime.currentDateTime().date())
        self.assertEqual(entry.entry, {'some var': 5, 'other_var': [1, 2, 3], 'final_var': {'a': 'b'}})
        entry, ok = reg.entry(id_2)
        self.assertTrue(ok)
        self.assertEqual(entry.providerId, 'my provider 2')
        self.assertEqual(entry.timestamp.date(), QDateTime.currentDateTime().date())
        self.assertEqual(entry.entry, {'some var': 6})

        entry = QgsHistoryEntry('my provider 3', QDateTime(2021, 1, 2, 3, 4, 5), {'var': 7})
        id_3, ok = reg.addEntry(entry)
        self.assertTrue(ok)
        self.assertEqual(len(reg.queryEntries()), 3)
        self.assertEqual(reg.queryEntries()[0].providerId, 'my provider')
        self.assertEqual(reg.queryEntries()[0].timestamp.date(), QDateTime.currentDateTime().date())
        self.assertEqual(reg.queryEntries()[0].entry, {'some var': 5, 'other_var': [1, 2, 3], 'final_var': {'a': 'b'}})
        self.assertEqual(reg.queryEntries()[1].providerId, 'my provider 2')
        self.assertEqual(reg.queryEntries()[1].timestamp.date(), QDateTime.currentDateTime().date())
        self.assertEqual(reg.queryEntries()[1].entry, {'some var': 6})
        self.assertEqual(reg.queryEntries()[2].providerId, 'my provider 3')
        self.assertEqual(reg.queryEntries()[2].timestamp.date(), QDate(2021, 1, 2))
        self.assertEqual(reg.queryEntries()[2].entry, {'var': 7})

        # query by provider
        entries = reg.queryEntries(providerId='my provider')
        self.assertEqual(len(entries), 1)
        self.assertEqual(entries[0].providerId, 'my provider')
        self.assertEqual(entries[0].timestamp.date(), QDateTime.currentDateTime().date())
        self.assertEqual(entries[0].entry, {'some var': 5, 'other_var': [1, 2, 3], 'final_var': {'a': 'b'}})

        entries = reg.queryEntries(providerId='my provider 2')
        self.assertEqual(len(entries), 1)
        self.assertEqual(entries[0].providerId, 'my provider 2')
        self.assertEqual(entries[0].timestamp.date(), QDateTime.currentDateTime().date())
        self.assertEqual(entries[0].entry, {'some var': 6})

        # query by date
        entries = reg.queryEntries(start=QDateTime(3022, 1, 2, 3, 4, 5))  # this test will break in 3022, sorry
        self.assertEqual(len(entries), 0)
        entries = reg.queryEntries(end=QDateTime(2020, 1, 2, 3, 4, 5))
        self.assertEqual(len(entries), 0)

        entries = reg.queryEntries(start=QDateTime(2021, 3, 2, 3, 4, 5))
        self.assertEqual(len(entries), 2)
        self.assertCountEqual([e.providerId for e in entries], ['my provider', 'my provider 2'])
        entries = reg.queryEntries(end=QDateTime(2021, 3, 2, 3, 4, 5))
        self.assertEqual(len(entries), 1)
        self.assertEqual(entries[0].providerId, 'my provider 3')

        # update an entry
        self.assertTrue(reg.updateEntry(id_1, {'new_props': 54}))
        entry, ok = reg.entry(id_1)
        self.assertEqual(entry.providerId, 'my provider')
        self.assertEqual(entry.timestamp.date(), QDateTime.currentDateTime().date())
        self.assertEqual(entry.entry, {'new_props': 54})

        self.assertTrue(reg.clearHistory(Qgis.HistoryProviderBackend.LocalProfile))
        self.assertFalse(reg.queryEntries())

        # bulk add entries
        self.assertTrue(reg.addEntries([
            QgsHistoryEntry('my provider 4', QDateTime(2021, 1, 2, 3, 4, 5), {'var': 7}),
            QgsHistoryEntry('my provider 5', QDateTime(2021, 1, 2, 3, 4, 5), {'var': 8})
        ]))
        self.assertEqual(len(reg.queryEntries()), 2)
        self.assertEqual(reg.queryEntries()[0].providerId, 'my provider 4')
        self.assertEqual(reg.queryEntries()[0].entry, {'var': 7})

        self.assertEqual(reg.queryEntries()[1].providerId, 'my provider 5')
        self.assertEqual(reg.queryEntries()[1].entry, {'var': 8})


if __name__ == '__main__':
    unittest.main()
