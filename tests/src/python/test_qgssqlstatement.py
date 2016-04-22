# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsSQLStatement.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Even Rouault'
__date__ = '4/4/2016'
__copyright__ = 'Copyright 2016, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.testing import unittest
from qgis.core import QgsSQLStatement


class TestQgsSQLStatementCustomFunctions(unittest.TestCase):

    def checkNominal(self, statement, expected_dump=None):
        exp = QgsSQLStatement(statement)
        self.assertEqual(exp.hasParserError(), False)
        self.assertEqual(exp.parserErrorString(), "")
        if expected_dump is None:
            expected_dump = statement
        self.assertEqual(exp.dump(), expected_dump)
        self.assertEqual(exp.dump(), exp.rootNode().clone().dump())

    def testNominalSimple(self):
        statement = "SELECT a FROM t"
        self.checkNominal(statement)
        exp = QgsSQLStatement(statement)
        statement_node = exp.rootNode()
        self.assertEqual(statement_node.nodeType(), QgsSQLStatement.ntSelect)
        tables = statement_node.tables()
        self.assertEqual(len(tables), 1)
        table = tables[0]
        self.assertEqual(table.nodeType(), QgsSQLStatement.ntTableDef)
        self.assertEqual(table.name(), 't')
        self.assertEqual(table.alias(), '')
        columns = statement_node.columns()
        self.assertEqual(len(columns), 1)
        column = columns[0]
        self.assertEqual(column.nodeType(), QgsSQLStatement.ntSelectedColumn)
        column_ref = column.column()
        self.assertEqual(column.alias(), '')
        self.assertEqual(column_ref.nodeType(), QgsSQLStatement.ntColumnRef)
        self.assertEqual(column_ref.name(), 'a')
        self.assertEqual(column_ref.tableName(), '')

    def testNominalSelectDistinct(self):
        statement = "SELECT DISTINCT a FROM t"
        self.checkNominal(statement)

    def testNominalColumns(self):
        statement = "SELECT null, 1234567890123456789, a, b b_alias, 'literal', CAST(1 AS varchar), "
        statement += "\"1c\", *, \"*\", a.*, foo(), bar(baz, baw), t.c AS \"1quoted\", "
        statement += "COUNT(*), COUNT(*) a, COUNT(DISTINCT x), COUNT(DISTINCT x) AS a, \"select\" FROM t"
        expected_dump = "SELECT NULL, 1234567890123456789, a, b AS b_alias, 'literal', CAST(1 AS varchar), "
        expected_dump += "\"1c\", *, \"*\", a.*, foo(), bar(baz, baw), t.c AS \"1quoted\", "
        expected_dump += "COUNT(*), COUNT(*) AS a, COUNT(DISTINCT x), COUNT(DISTINCT x) AS a, \"select\" FROM t"
        self.checkNominal(statement, expected_dump)

    def testNominalFrom(self):
        statement = "SELECT a FROM t1, t2 at2, t3 AS at3, \"1quoted\", t4 AS \"2quoted\""
        expected_dump = "SELECT a FROM t1, t2 AS at2, t3 AS at3, \"1quoted\", t4 AS \"2quoted\""
        self.checkNominal(statement, expected_dump)

    def testNominalWhere(self):
        statement = "SELECT a FROM t WHERE 1.5 <= 'a' OR TRUE OR FALSE OR a IS NULL AND b IS NOT NULL " + \
                    "OR NOT d OR 1 + (2 - 3) * 4 / 5 ^ 6 <> 0 OR a IN (1, 2) OR b NOT IN (5) " + \
                    "OR x BETWEEN 5 AND 6 OR x NOT BETWEEN 5 AND 6 OR c = d OR c > d OR c < d OR c >= d OR c <= d"
        self.checkNominal(statement)

    def checkJoinType(self, joinType):
        statement = "SELECT a FROM t " + joinType + " j1 ON TRUE"
        self.checkNominal(statement)

    def testJoinTypes(self):
        self.checkJoinType('JOIN')
        self.checkJoinType('LEFT JOIN')
        self.checkJoinType('LEFT OUTER JOIN')
        self.checkJoinType('RIGHT JOIN')
        self.checkJoinType('RIGHT OUTER JOIN')
        self.checkJoinType('CROSS JOIN')
        self.checkJoinType('FULL JOIN')
        self.checkJoinType('INNER JOIN')

    def testJoin(self):
        statement = "SELECT a FROM t JOIN j1 ON TRUE JOIN j2 USING (a) JOIN j3 USING (\"1a\", b)"
        self.checkNominal(statement)

    def testNominalOrderBy(self):
        statement = "SELECT a FROM t ORDER BY a, b ASC, c DESC"
        expected_dump = "SELECT a FROM t ORDER BY a, b, c DESC"
        self.checkNominal(statement, expected_dump)

    def testNominalFull(self):
        statement = \
            "SELECT a FROM t JOIN j1 ON cond1 JOIN j2 ON cond2 WHERE TRUE ORDER BY c"
        self.checkNominal(statement)

    def checkError(self, statement):
        exp = QgsSQLStatement(statement)
        self.assertEqual(exp.hasParserError(), True)
        self.assertNotEqual(exp.parserErrorString(), '')
        self.assertEqual(exp.dump(), "(no root)")
        self.assertEqual(exp.rootNode(), None)

    def testError(self):
        self.checkError("1")
        self.checkError("SELECT")
        self.checkError("SELECT a")
        self.checkError("SELECT a, FROM b")
        self.checkError("SELECT 1a FROM b")
        self.checkError("SELECT a AS FROM b")
        self.checkError("SELECT a,. FROM b")
        self.checkError("SELECT f(*) FROM b")
        self.checkError("SELECT f(*) a FROM b")
        self.checkError("SELECT .")
        self.checkError("SELECT a FROM")
        self.checkError("SELECT a FROM b WHERE")
        self.checkError("SELECT a FROM b WHERE .")
        self.checkError("SELECT a FROM b,")
        self.checkError("SELECT a FROM b,.")
        self.checkError("SELECT a FROM b JOIN")
        self.checkError("SELECT a FROM b JOIN c")
        self.checkError("SELECT a FROM b JOIN c ON")
        self.checkError("SELECT a FROM b JOIN c USING")
        self.checkError("SELECT a FROM b JOIN c ON d JOIN")
        self.checkError("SELECT a FROM b ORDER BY")
        self.checkError("SELECT a FROM b JOIN c ON d ORDER BY e unexpected")

    def testBasicValidationCheck(self):
        exp = QgsSQLStatement("error")
        (b, errorMsg) = exp.doBasicValidationChecks()
        self.assertFalse(b)
        self.assertEqual(errorMsg, 'No root node')

        exp = QgsSQLStatement("SELECT c FROM t")
        (b, errorMsg) = exp.doBasicValidationChecks()
        self.assertTrue(b)
        self.assertEqual(errorMsg, '')

        exp = QgsSQLStatement("SELECT t.c FROM t ORDER BY t.c")
        (b, errorMsg) = exp.doBasicValidationChecks()
        self.assertTrue(b)
        self.assertEqual(errorMsg, '')

        exp = QgsSQLStatement("SELECT t.c FROM t t_alias")
        (b, errorMsg) = exp.doBasicValidationChecks()
        self.assertFalse(b)
        self.assertEqual(
            errorMsg, 'Table t is referenced by column c, but not selected in FROM / JOIN.')

        exp = QgsSQLStatement(
            "SELECT CAST(1 + foo(t_unknown.a) AS varchar) FROM t")
        (b, errorMsg) = exp.doBasicValidationChecks()
        self.assertFalse(b)
        self.assertEqual(
            errorMsg, 'Table t_unknown is referenced by column a, but not selected in FROM / JOIN.')

        exp = QgsSQLStatement("SELECT c FROM t WHERE t_unknown.a = 1")
        (b, errorMsg) = exp.doBasicValidationChecks()
        self.assertFalse(b)
        self.assertEqual(
            errorMsg, 'Table t_unknown is referenced by column a, but not selected in FROM / JOIN.')

        exp = QgsSQLStatement(
            "SELECT c FROM t JOIN t2 ON t.c1 = t2.c2 AND t3.c3 IS NOT NULL")
        (b, errorMsg) = exp.doBasicValidationChecks()
        self.assertFalse(b)
        self.assertEqual(
            errorMsg, 'Table t3 is referenced by column c3, but not selected in FROM / JOIN.')

        exp = QgsSQLStatement("SELECT c FROM t ORDER BY t_unknown.c")
        (b, errorMsg) = exp.doBasicValidationChecks()
        self.assertFalse(b)
        self.assertEqual(
            errorMsg, 'Table t_unknown is referenced by column c, but not selected in FROM / JOIN.')

if __name__ == "__main__":
    unittest.main()
