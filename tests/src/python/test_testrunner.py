
# -*- coding: utf-8 -*-
"""QGIS Unit tests for the docker python test runner

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Alessandro Pasotti'
__date__ = '19.11.2018'
__copyright__ = 'Copyright 2018, The QGIS Project'

import qgis  # NOQA
import sys

from qgis.testing import unittest
from console import console
from qgis.core import Qgis


class TestTestRunner(unittest.TestCase):

    def test_fails(self):
        self.assertTrue(False)

    def test_passes(self):
        self.assertTrue(Qgis.QGIS_VERSION_INT > 0)

    @unittest.skip('Skipped!')
    def test_skipped(self):
        self.assertTrue(False)


def _make_runner(tests=[]):
    suite = unittest.TestSuite()
    for t in tests:
        suite.addTest(TestTestRunner(t))
    runner = unittest.TextTestRunner(verbosity=2)
    return runner.run(suite)


# Test functions to be called by the runner

def run_all():
    """Default function that is called by the runner if nothing else is specified"""
    return _make_runner(['test_fails', 'test_skipped', 'test_passes'])


def run_failing():
    """Run failing test only"""
    return _make_runner(['test_fails'])


def run_passing():
    """Run passing test only"""
    return _make_runner(['test_passes'])


def run_skipped():
    """Run skipped test only"""
    return _make_runner(['test_skipped'])


def run_skipped_and_passing():
    """Run skipped and passing test only"""
    return _make_runner(['test_skipped', 'test_passes'])
