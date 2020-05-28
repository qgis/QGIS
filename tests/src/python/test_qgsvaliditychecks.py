# -*- coding: utf-8 -*-
"""QGIS Unit tests for validity checks

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '03/12/2018'
__copyright__ = 'Copyright 2018, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsApplication,
                       QgsAbstractValidityCheck,
                       QgsValidityCheckRegistry,
                       QgsValidityCheckResult,
                       QgsValidityCheckContext,
                       QgsFeedback,
                       check)
from qgis.testing import start_app, unittest

app = start_app()


class TestCheck(QgsAbstractValidityCheck):

    def __init__(self, id, name, type, results):
        super().__init__()
        self._name = name
        self._id = id
        self._type = type
        self._results = results

    def create(self):
        return TestCheck(self._id, self._name, self._type, self._results)

    def id(self):
        return self._id

    def checkType(self):
        return self._type

    def runCheck(self, _, __):
        return self._results


class TestContext(QgsValidityCheckContext):

    def type(self):
        return 0


# register some checks using the decorator syntax
@check.register(type=QgsAbstractValidityCheck.TypeLayoutCheck)
def my_check(context, feedback):
    assert context


@check.register(type=QgsAbstractValidityCheck.TypeLayoutCheck)
def my_check2(context, feedback):
    res = QgsValidityCheckResult()
    res.type = QgsValidityCheckResult.Warning
    res.title = 'test'
    res.detailedDescription = 'blah blah'
    return [res]


class TestQgsValidityChecks(unittest.TestCase):

    def testAppRegistry(self):
        # ensure there is an application instance
        self.assertIsNotNone(QgsApplication.validityCheckRegistry())

    def testDecorator(self):
        # test that checks registered using the decorator have worked
        self.assertEqual(len(QgsApplication.validityCheckRegistry().checks()), 2)

        context = TestContext()
        feedback = QgsFeedback()
        res = QgsApplication.validityCheckRegistry().runChecks(QgsAbstractValidityCheck.TypeLayoutCheck, context, feedback)
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].title, 'test')

    def testRegistry(self):
        registry = QgsValidityCheckRegistry()
        self.assertFalse(registry.checks())

        # add a new check
        c1 = TestCheck('c1', 'my check', 1, [])
        registry.addCheck(c1)
        self.assertEqual(registry.checks(), [c1])

        c2 = TestCheck('c2', 'my check2', 1, [])
        registry.addCheck(c2)
        self.assertEqual(registry.checks(), [c1, c2])

        registry.removeCheck(None)
        c3 = TestCheck('c3', 'my check3', 1, [])
        # not in registry yet
        registry.removeCheck(c3)

        registry.removeCheck(c1)
        self.assertEqual(registry.checks(), [c2])

        registry.removeCheck(c2)
        self.assertFalse(registry.checks())

    def testRegistryChecks(self):
        registry = QgsValidityCheckRegistry()
        c1 = TestCheck('c1', 'my check', 1, [])
        registry.addCheck(c1)
        c2 = TestCheck('c2', 'my check2', 2, [])
        registry.addCheck(c2)
        c3 = TestCheck('c3', 'my check3', 1, [])
        registry.addCheck(c3)

        self.assertFalse(registry.checks(0))
        self.assertEqual(registry.checks(1), [c1, c3])
        self.assertEqual(registry.checks(2), [c2])

    def testRunChecks(self):
        registry = QgsValidityCheckRegistry()
        res1 = QgsValidityCheckResult()
        res1.type = QgsValidityCheckResult.Warning
        res1.title = 'test'
        res1.detailedDescription = 'blah blah'

        c1 = TestCheck('c1', 'my check', 1, [res1])
        registry.addCheck(c1)

        res2 = QgsValidityCheckResult()
        res2.type = QgsValidityCheckResult.Critical
        res2.title = 'test2'
        res2.detailedDescription = 'blah blah2'
        c2 = TestCheck('c2', 'my check2', 2, [res2])
        registry.addCheck(c2)

        res3 = QgsValidityCheckResult()
        res3.type = QgsValidityCheckResult.Warning
        res3.title = 'test3'
        res3.detailedDescription = 'blah blah3'
        res4 = QgsValidityCheckResult()
        res4.type = QgsValidityCheckResult.Warning
        res4.title = 'test4'
        res4.detailedDescription = 'blah blah4'
        c3 = TestCheck('c3', 'my check3', 1, [res3, res4])
        registry.addCheck(c3)

        context = TestContext()
        feedback = QgsFeedback()
        self.assertFalse(registry.runChecks(0, context, feedback))

        self.assertEqual([r.type for r in registry.runChecks(1, context, feedback)],
                         [QgsValidityCheckResult.Warning, QgsValidityCheckResult.Warning,
                          QgsValidityCheckResult.Warning])
        self.assertEqual([r.title for r in registry.runChecks(1, context, feedback)], ['test', 'test3', 'test4'])
        self.assertEqual([r.detailedDescription for r in registry.runChecks(1, context, feedback)],
                         ['blah blah', 'blah blah3', 'blah blah4'])

        self.assertEqual([r.type for r in registry.runChecks(2, context, feedback)], [QgsValidityCheckResult.Critical])
        self.assertEqual([r.title for r in registry.runChecks(2, context, feedback)], ['test2'])
        self.assertEqual([r.detailedDescription for r in registry.runChecks(2, context, feedback)], ['blah blah2'])


if __name__ == '__main__':
    unittest.main()
