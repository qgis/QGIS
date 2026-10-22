"""QGIS Unit tests for QgsEsriArcadeScript.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import sys

from qgis.core import (
    QgsEsriArcadeBinaryOperatorExpression,
    QgsEsriArcadeBlockStatement,
    QgsEsriArcadeFunctionDeclarationStatement,
    QgsEsriArcadeIndexExpression,
    QgsEsriArcadeLiteralExpression,
    QgsEsriArcadeReturnStatement,
    QgsEsriArcadeScript,
    QgsEsriArcadeVariableDeclarationStatement,
    QgsEsriArcadeVariableReferenceExpression,
)
from qgis.testing import unittest


class TestQgsEsriArcadeScript(unittest.TestCase):
    def test_parser_error(self):
        script = QgsEsriArcadeScript("""var x = """)
        self.assertTrue(script.hasParserError())
        self.assertEqual(script.parserErrorString(), "syntax error")

    def test_a(self):
        script = QgsEsriArcadeScript("""var x = 3; var y = 4;
            return x + y;""")

        self.assertFalse(script.hasParserError())

        self.assertEqual(
            script.script(),
            """var x = 3; var y = 4;
            return x + y;""",
        )
        self.assertEqual(
            script.dump(),
            """var x = 3;
var y = 4;
return x + y;""",
        )

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 3)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsEsriArcadeVariableDeclarationStatement,
        )
        self.assertEqual(script.rootBlock().statements()[0].variableName(), "x")
        self.assertIsInstance(
            script.rootBlock().statements()[0].initializer(),
            QgsEsriArcadeLiteralExpression,
        )
        self.assertEqual(script.rootBlock().statements()[0].initializer().value(), 3)

        self.assertIsInstance(
            script.rootBlock().statements()[1],
            QgsEsriArcadeVariableDeclarationStatement,
        )
        self.assertEqual(script.rootBlock().statements()[1].variableName(), "y")
        self.assertIsInstance(
            script.rootBlock().statements()[1].initializer(),
            QgsEsriArcadeLiteralExpression,
        )
        self.assertEqual(script.rootBlock().statements()[1].initializer().value(), 4)

        self.assertIsInstance(
            script.rootBlock().statements()[2], QgsEsriArcadeReturnStatement
        )

        self.assertIsInstance(
            script.rootBlock().statements()[2].expression(),
            QgsEsriArcadeBinaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[2].expression().binaryOperator(),
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.boPlus,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[2].expression().leftOperand(),
            QgsEsriArcadeVariableReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[2].expression().leftOperand().name(), "x"
        )

        self.assertIsInstance(
            script.rootBlock().statements()[2].expression().rightOperand(),
            QgsEsriArcadeVariableReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[2].expression().rightOperand().name(), "y"
        )

    def test_multiline(self):
        script = QgsEsriArcadeScript(
            """function calculateRatio(values, index){
  var value = values[index];
  return value / Sum(values);
}"""
        )
        self.assertFalse(script.hasParserError())
        self.assertEqual(
            script.dump(),
            """function calculateRatio(values, index) {
var value = values[index];
return value / Sum(values);
}""",
        )
        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsEsriArcadeFunctionDeclarationStatement,
        )

        self.assertEqual(
            script.rootBlock().statements()[0].functionName(), "calculateRatio"
        )
        self.assertEqual(
            script.rootBlock().statements()[0].parameters(), ["values", "index"]
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].body(), QgsEsriArcadeBlockStatement
        )
        self.assertEqual(script.rootBlock().statements()[0].body().count(), 2)

        self.assertIsInstance(
            script.rootBlock().statements()[0].body().statements()[0],
            QgsEsriArcadeVariableDeclarationStatement,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].body().statements()[0].variableName(),
            "value",
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].body().statements()[0].initializer(),
            QgsEsriArcadeIndexExpression,
        )
        self.assertIsInstance(
            script.rootBlock()
            .statements()[0]
            .body()
            .statements()[0]
            .initializer()
            .containerExpression(),
            QgsEsriArcadeVariableReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .body()
            .statements()[0]
            .initializer()
            .containerExpression()
            .name(),
            "values",
        )

        self.assertIsInstance(
            script.rootBlock()
            .statements()[0]
            .body()
            .statements()[0]
            .initializer()
            .indexExpression(),
            QgsEsriArcadeVariableReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .body()
            .statements()[0]
            .initializer()
            .indexExpression()
            .name(),
            "index",
        )

    def test_comments(self):
        script = QgsEsriArcadeScript(
            """var x = 3;
// this is a comment"""
        )

        self.assertFalse(script.hasParserError())

        self.assertEqual(
            script.script(),
            """var x = 3;
// this is a comment""",
        )
        # comments are ignored in dump
        self.assertEqual(script.dump(), """var x = 3;""")
        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsEsriArcadeVariableDeclarationStatement,
        )

        script = QgsEsriArcadeScript(
            """var x = 3;
/*
 this is a
 multiline
 comment
*/"""
        )

        self.assertFalse(script.hasParserError())

        self.assertEqual(
            script.script(),
            """var x = 3;
/*
 this is a
 multiline
 comment
*/""",
        )
        # comments are ignored in dump
        self.assertEqual(script.dump(), """var x = 3;""")
        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsEsriArcadeVariableDeclarationStatement,
        )

        script = QgsEsriArcadeScript("""var x /* inline comment */ = 3; // comment""")

        self.assertFalse(script.hasParserError())

        self.assertEqual(
            script.script(), """var x /* inline comment */ = 3; // comment"""
        )
        # comments are ignored in dump
        self.assertEqual(script.dump(), """var x = 3;""")
        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsEsriArcadeVariableDeclarationStatement,
        )


if __name__ == "__main__":
    unittest.main()
