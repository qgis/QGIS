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
                       QgsLocatorAutomaticModel)
from qgis.PyQt.QtCore import QVariant, pyqtSignal, QCoreApplication
from time import sleep
from qgis.testing import start_app, unittest
import sip
start_app()


class test_filter(QgsLocatorFilter):

    def __init__(self, prefix, parent=None):
        super().__init__(parent)
        self.prefix = prefix

    def name(self):
        return 'test'

    def displayName(self):
        return 'test'

    def fetchResults(self, string, context, feedback):
        for i in range(3):
            #if feedback.isCanceled():
            #    return
            sleep(0.00001)
            result = QgsLocatorResult()
            result.displayString = self.prefix + str(i)
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


if __name__ == '__main__':
    unittest.main()
