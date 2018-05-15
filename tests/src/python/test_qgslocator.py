# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLocator.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2017 by Nyall Dawson'
__date__ = '6/05/2017'
__copyright__ = 'Copyright 2017, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'
import qgis  # NOQA

import os

from qgis.core import (QgsLocator,
                       QgsLocatorFilter,
                       QgsLocatorContext,
                       QgsLocatorResult,
                       QgsLocatorModel,
                       QgsLocatorAutomaticModel,
                       QgsSettings)
from qgis.PyQt.QtCore import QVariant, pyqtSignal, QCoreApplication
from time import sleep
from qgis.testing import start_app, unittest
import sip
start_app()


class test_filter(QgsLocatorFilter):

    def __init__(self, identifier, prefix=None, parent=None):
        super().__init__(parent)
        self.identifier = identifier
        self._prefix = prefix

    def clone(self):
        return test_filter(self.identifier)

    def name(self):
        return 'test_' + self.identifier

    def displayName(self):
        return 'test'

    def prefix(self):
        return self._prefix

    def fetchResults(self, string, context, feedback):
        for i in range(3):
            if feedback.isCanceled():
                return
            sleep(0.001)
            result = QgsLocatorResult()
            result.displayString = self.identifier + str(i)
            self.resultFetched.emit(result)

    def triggerResult(self, result):
        pass


class TestQgsLocator(unittest.TestCase):

    def testRegisteringFilters(self):
        l = QgsLocator()
        filter_a = test_filter('a')
        filter_b = test_filter('b')
        l.registerFilter(filter_a)
        l.registerFilter(filter_b)

        self.assertEqual(set(l.filters()), {filter_a, filter_b})

        # ownership should be transferred to locator
        del l
        self.assertTrue(sip.isdeleted(filter_a))
        self.assertTrue(sip.isdeleted(filter_b))

        # try manually deregistering
        l = QgsLocator()
        filter_c = test_filter('c')
        filter_d = test_filter('d')
        l.registerFilter(filter_c)
        l.registerFilter(filter_d)
        self.assertEqual(set(l.filters()), {filter_c, filter_d})

        l.deregisterFilter(filter_c)
        self.assertTrue(sip.isdeleted(filter_c))
        self.assertFalse(sip.isdeleted(filter_d))
        self.assertEqual(l.filters(), [filter_d])
        del l
        self.assertTrue(sip.isdeleted(filter_c))
        self.assertTrue(sip.isdeleted(filter_d))

    def testFetchingResults(self):

        def got_hit(result):
            got_hit._results_.append(result.displayString)

        got_hit._results_ = []

        context = QgsLocatorContext()

        # one filter
        l = QgsLocator()
        filter_a = test_filter('a')

        l.registerFilter(filter_a)

        l.foundResult.connect(got_hit)
        l.fetchResults('a', context)

        for i in range(100):
            sleep(0.002)
            QCoreApplication.processEvents()

        self.assertEqual(set(got_hit._results_), {'a0', 'a1', 'a2'})

        # two filters
        filter_b = test_filter('b')
        l.registerFilter(filter_b)
        got_hit._results_ = []
        l.fetchResults('a', context)

        for i in range(100):
            sleep(0.002)
            QCoreApplication.processEvents()

        self.assertEqual(set(got_hit._results_), {'a0', 'a1', 'a2', 'b0', 'b1', 'b2'})

    def testDeleteWhileFetchingResults(self):
        """
        Delete locator whilst fetching results
        """

        def got_hit(result):
            got_hit._results_.append(result.displayString)

        got_hit._results_ = []

        context = QgsLocatorContext()

        l = QgsLocator()
        filter_a = test_filter('a')
        l.registerFilter(filter_a)

        l.foundResult.connect(got_hit)
        l.fetchResults('a', context)
        del l

    def testCancelWhileFetchingResults(self):
        """
        Cancel locator whilst fetching results
        """

        def got_hit(result):
            got_hit._results_.append(result.displayString)

        got_hit._results_ = []

        context = QgsLocatorContext()

        l = QgsLocator()
        filter_a = test_filter('a')
        l.registerFilter(filter_a)

        l.foundResult.connect(got_hit)
        l.fetchResults('a', context)
        l.cancel()

    def testPrefixes(self):
        """
        Test custom (active) prefixes
        """

        def got_hit(result):
            got_hit._results_.append(result.displayString)

        got_hit._results_ = []

        context = QgsLocatorContext()

        l = QgsLocator()

        # filter with prefix
        filter_a = test_filter('a', 'aaa')
        l.registerFilter(filter_a)
        self.assertEqual(filter_a.prefix(), 'aaa')
        self.assertEqual(filter_a.activePrefix(), 'aaa')
        self.assertEqual(filter_a.useWithoutPrefix(), True)
        l.foundResult.connect(got_hit)
        l.fetchResults('aaa a', context)
        for i in range(100):
            sleep(0.002)
            QCoreApplication.processEvents()
        self.assertEqual(set(got_hit._results_), {'a0', 'a1', 'a2'})
        got_hit._results_ = []
        l.fetchResults('bbb b', context)
        for i in range(100):
            sleep(0.002)
            QCoreApplication.processEvents()
        self.assertEqual(set(got_hit._results_), {'a0', 'a1', 'a2'})
        got_hit._results_ = []
        filter_a.setUseWithoutPrefix(False)
        self.assertEqual(filter_a.useWithoutPrefix(), False)
        l.fetchResults('bbb b', context)
        for i in range(100):
            sleep(0.002)
            QCoreApplication.processEvents()
        self.assertEqual(got_hit._results_, [])
        got_hit._results_ = []

        # test with two filters
        filter_b = test_filter('b', 'bbb')
        l.registerFilter(filter_b)
        self.assertEqual(filter_b.prefix(), 'bbb')
        self.assertEqual(filter_b.activePrefix(), 'bbb')
        got_hit._results_ = []
        l.fetchResults('bbb b', context)
        for i in range(100):
            sleep(0.002)
            QCoreApplication.processEvents()
        self.assertEqual(set(got_hit._results_), {'b0', 'b1', 'b2'})
        l.deregisterFilter(filter_b)

        # test with two filters with same prefix
        filter_b = test_filter('b', 'aaa')
        l.registerFilter(filter_b)
        self.assertEqual(filter_b.prefix(), 'aaa')
        self.assertEqual(filter_b.activePrefix(), 'aaa')
        got_hit._results_ = []
        l.fetchResults('aaa b', context)
        for i in range(100):
            sleep(0.002)
            QCoreApplication.processEvents()
        self.assertEqual(set(got_hit._results_), {'a0', 'a1', 'a2', 'b0', 'b1', 'b2'})
        l.deregisterFilter(filter_b)

        # filter with invalid prefix (less than 3 char)
        filter_c = test_filter('c', 'bb')
        l.registerFilter(filter_c)
        self.assertEqual(filter_c.prefix(), 'bb')
        self.assertEqual(filter_c.activePrefix(), '')
        got_hit._results_ = []
        l.fetchResults('b', context)
        for i in range(100):
            sleep(0.002)
            QCoreApplication.processEvents()
        self.assertEqual(set(got_hit._results_), {'c0', 'c1', 'c2'})
        l.deregisterFilter(filter_c)

        # filter with custom prefix
        QgsSettings().setValue("locator_filters/prefix_test_custom", 'xyz', QgsSettings.Gui)
        filter_c = test_filter('custom', 'abc')
        l.registerFilter(filter_c)
        self.assertEqual(filter_c.prefix(), 'abc')
        self.assertEqual(filter_c.activePrefix(), 'xyz')
        got_hit._results_ = []
        l.fetchResults('b', context)
        for i in range(100):
            sleep(0.002)
            QCoreApplication.processEvents()
        self.assertEqual(set(got_hit._results_), {'custom0', 'custom1', 'custom2'})
        l.deregisterFilter(filter_c)

        del l

    def testModel(self):
        m = QgsLocatorModel()
        l = QgsLocator()

        filter_a = test_filter('a')
        l.registerFilter(filter_a)
        l.foundResult.connect(m.addResult)
        context = QgsLocatorContext()

        l.fetchResults('a', context)

        for i in range(100):
            sleep(0.002)
            QCoreApplication.processEvents()

        # 4 results - one is locator name
        self.assertEqual(m.rowCount(), 4)
        self.assertEqual(m.data(m.index(0, 0)), 'test')
        self.assertEqual(m.data(m.index(0, 0), QgsLocatorModel.ResultTypeRole), 0)
        self.assertEqual(m.data(m.index(0, 0), QgsLocatorModel.ResultFilterNameRole), 'test')
        self.assertEqual(m.data(m.index(1, 0)), 'a0')
        self.assertEqual(m.data(m.index(1, 0), QgsLocatorModel.ResultTypeRole), 1)
        self.assertEqual(m.data(m.index(1, 0), QgsLocatorModel.ResultFilterNameRole), 'test')
        self.assertEqual(m.data(m.index(2, 0)), 'a1')
        self.assertEqual(m.data(m.index(2, 0), QgsLocatorModel.ResultTypeRole), 1)
        self.assertEqual(m.data(m.index(2, 0), QgsLocatorModel.ResultFilterNameRole), 'test')
        self.assertEqual(m.data(m.index(3, 0)), 'a2')
        self.assertEqual(m.data(m.index(3, 0), QgsLocatorModel.ResultTypeRole), 1)
        self.assertEqual(m.data(m.index(3, 0), QgsLocatorModel.ResultFilterNameRole), 'test')

        m.clear()
        self.assertEqual(m.rowCount(), 0)
        l.fetchResults('b', context)

        for i in range(100):
            sleep(0.002)
            QCoreApplication.processEvents()

        self.assertEqual(m.rowCount(), 4)
        self.assertEqual(m.data(m.index(1, 0)), 'a0')
        self.assertEqual(m.data(m.index(2, 0)), 'a1')
        self.assertEqual(m.data(m.index(3, 0)), 'a2')

        m.deferredClear()
        # should not be immediately cleared!
        self.assertEqual(m.rowCount(), 4)
        for i in range(100):
            sleep(0.002)
            QCoreApplication.processEvents()
        self.assertEqual(m.rowCount(), 0)

    def testAutoModel(self):
        """
        Test automatic model, QgsLocatorAutomaticModel - should be no need
        for any manual connections
        """
        l = QgsLocator()
        m = QgsLocatorAutomaticModel(l)

        filter_a = test_filter('a')
        l.registerFilter(filter_a)

        m.search('a')

        for i in range(100):
            sleep(0.002)
            QCoreApplication.processEvents()

        # 4 results - one is locator name
        self.assertEqual(m.rowCount(), 4)
        self.assertEqual(m.data(m.index(0, 0)), 'test')
        self.assertEqual(m.data(m.index(0, 0), QgsLocatorModel.ResultTypeRole), 0)
        self.assertEqual(m.data(m.index(0, 0), QgsLocatorModel.ResultFilterNameRole), 'test')
        self.assertEqual(m.data(m.index(1, 0)), 'a0')
        self.assertEqual(m.data(m.index(1, 0), QgsLocatorModel.ResultTypeRole), 1)
        self.assertEqual(m.data(m.index(1, 0), QgsLocatorModel.ResultFilterNameRole), 'test')
        self.assertEqual(m.data(m.index(2, 0)), 'a1')
        self.assertEqual(m.data(m.index(2, 0), QgsLocatorModel.ResultTypeRole), 1)
        self.assertEqual(m.data(m.index(2, 0), QgsLocatorModel.ResultFilterNameRole), 'test')
        self.assertEqual(m.data(m.index(3, 0)), 'a2')
        self.assertEqual(m.data(m.index(3, 0), QgsLocatorModel.ResultTypeRole), 1)
        self.assertEqual(m.data(m.index(3, 0), QgsLocatorModel.ResultFilterNameRole), 'test')

        m.search('a')

        for i in range(100):
            sleep(0.002)
            QCoreApplication.processEvents()

        # 4 results - one is locator name
        self.assertEqual(m.rowCount(), 4)
        self.assertEqual(m.data(m.index(0, 0)), 'test')
        self.assertEqual(m.data(m.index(1, 0)), 'a0')
        self.assertEqual(m.data(m.index(2, 0)), 'a1')
        self.assertEqual(m.data(m.index(3, 0)), 'a2')

    def testStringMatches(self):
        self.assertFalse(QgsLocatorFilter.stringMatches('xxx', 'yyyy'))
        self.assertTrue(QgsLocatorFilter.stringMatches('axxxy', 'xxx'))
        self.assertTrue(QgsLocatorFilter.stringMatches('aXXXXy', 'xxx'))
        self.assertFalse(QgsLocatorFilter.stringMatches('aXXXXy', ''))


if __name__ == '__main__':
    unittest.main()
