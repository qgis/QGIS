# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsLayoutTable.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = '(C) 2022 by Nyall Dawson'
__date__ = '13/06/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

from qgis.core import (QgsLayoutTableColumn)
from qgis.testing import (start_app,
                          unittest
                          )

from utilities import unitTestDataPath

start_app()
TEST_DATA_DIR = unitTestDataPath()


class TestQgsLayoutTable(unittest.TestCase):

    def test_column(self):
        """Test initial size of legend with a symbol size in map units"""
        col = QgsLayoutTableColumn()
        col.setAttribute('attribute')
        self.assertEqual(col.__repr__(), '<QgsLayoutTableColumn: attribute>')
        col.setHeading('heading')
        self.assertEqual(col.__repr__(), '<QgsLayoutTableColumn: attribute ("heading")>')


if __name__ == '__main__':
    unittest.main()
