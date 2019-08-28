# -*- coding: utf-8 -*-
"""QGIS Unit tests for the DBManager SQL Window

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Stephen Knox'
__date__ = '2019-08-27'
__copyright__ = 'Copyright 2019, Stephen Knox'

from qgis.testing import unittest
from plugins.db_manager.dlg_sql_window import check_comments_in_sql


class TestPyQgsDBManagerSQLWindow(unittest.TestCase):

    def test_no_comment_parsing(self):
        query = "SELECT * FROM test"
        self.assertEqual(check_comments_in_sql(query), query)

    def test_comment_parsing(self):
        query = "SELECT * FROM test -- WHERE a = 1 "
        self.assertEqual(check_comments_in_sql(query), "SELECT * FROM test")

    def test_comment_parsing_newline(self):
        query = "SELECT * FROM test -- WHERE a = 1 \n ORDER BY b"
        self.assertEqual(check_comments_in_sql(query), "SELECT * FROM test   ORDER BY b")

    def test_comment_parsing_newline2(self):
        query = "SELECT * FROM test \n-- WHERE a = 1 \n ORDER BY b"
        self.assertEqual(check_comments_in_sql(query), "SELECT * FROM test   ORDER BY b")

    def test_comment_parsing_nothing(self):
        query = "--SELECT * FROM test"
        self.assertEqual(check_comments_in_sql(query), "")

    def test_comment_parsing_quote(self):
        query = "SELECT * FROM test WHERE a = '--sdf'"
        self.assertEqual(check_comments_in_sql(query), query)
        
    def test_comment_parsing_identifier(self):
        query = 'SELECT * FROM "test--1" WHERE a = 1'
        self.assertEqual(check_comments_in_sql(query), query)

if __name__ == '__main__':
    unittest.main()
