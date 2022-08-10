# -*- coding: utf-8 -*-
"""Contains tests which reveal broken behavior in QGIS.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '10/08/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

from qgis.testing import start_app, unittest
from utilities import unitTestDataPath

app = start_app()
TEST_DATA_DIR = unitTestDataPath()



"""
This file contains tests which reveal actual broken behavior in QGIS, where the fix for the
underlying issue is unknown or non-trivial.

(It is not designed for broken *tests*, only for working tests which show broken behavior and
accordingly can't be run on the CI)

DO NOT ADD TESTS TO THIS FILE WITHOUT A DETAILED EXPLANATION ON WHY!!!!
"""


class TestQgsDisabledTests(unittest.TestCase):

    pass


if __name__ == '__main__':
    unittest.main()
