"""QGIS Unit tests for the DBManager SQL Window

From build dir, run: ctest -R PyQgsDBManagerSQLWindow -V

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Stephen Knox"
__date__ = "2019-08-27"
__copyright__ = "Copyright 2019, Stephen Knox"

from plugins.db_manager.dlg_sql_window import check_comments_in_sql
from qgis.testing import unittest


class TestPyQgsDBManagerSQLWindow(unittest.TestCase):

    def test_check_comment_parsing(self):
        """Test we can parse the comment in a SQL query."""
        # No comment
        query = "SELECT * FROM test"
        self.assertEqual(check_comments_in_sql(query), query)

        # One comment
        query = "SELECT * FROM test -- WHERE a = 1 "
        self.assertEqual(check_comments_in_sql(query), "SELECT * FROM test")

        # One comment with a new line
        query = "SELECT * FROM test -- WHERE a = 1 \n ORDER BY b"
        self.assertEqual(
            check_comments_in_sql(query), "SELECT * FROM test   ORDER BY b"
        )

        # One comment with 2 new lines
        query = "SELECT * FROM test \n-- WHERE a = 1 \n ORDER BY b"
        self.assertEqual(
            check_comments_in_sql(query), "SELECT * FROM test   ORDER BY b"
        )

        # Only comment
        query = "--SELECT * FROM test"
        self.assertEqual(check_comments_in_sql(query), "")

        # Not a comment within a value
        query = "SELECT * FROM test WHERE a = '--sdf'"
        self.assertEqual(check_comments_in_sql(query), query)

        # Not a comment within a field
        query = 'SELECT * FROM "test--1" WHERE a = 1'
        self.assertEqual(check_comments_in_sql(query), query)


if __name__ == "__main__":
    unittest.main()
