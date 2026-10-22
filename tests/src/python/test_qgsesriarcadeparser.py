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
    QgsEsriArcadeExpressionStatement,
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

    def test_single_line_string(self):
        script = QgsEsriArcadeScript('"Hello world"')
        self.assertFalse(script.hasParserError())

        self.assertEqual(script.script(), '"Hello world"')
        self.assertEqual(script.dump(), '"Hello world";')

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsEsriArcadeExpressionStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].expression(),
            QgsEsriArcadeLiteralExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().value(), "Hello world"
        )

    def test_single_line_expression(self):
        script = QgsEsriArcadeScript("1 + 2")
        self.assertFalse(script.hasParserError())

        self.assertEqual(script.script(), "1 + 2")
        self.assertEqual(script.dump(), "1 + 2;")

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsEsriArcadeExpressionStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].expression(),
            QgsEsriArcadeBinaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().binaryOperator(),
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.boPlus,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].expression().leftOperand(),
            QgsEsriArcadeLiteralExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().leftOperand().value(), 1
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].expression().rightOperand(),
            QgsEsriArcadeLiteralExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().rightOperand().value(), 2
        )

    def test_single_line_string_with_explicit_return(self):
        script = QgsEsriArcadeScript('return "Hello world"')
        self.assertFalse(script.hasParserError())

        self.assertEqual(script.script(), 'return "Hello world"')
        self.assertEqual(script.dump(), 'return "Hello world";')

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsEsriArcadeReturnStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].expression(),
            QgsEsriArcadeLiteralExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().value(), "Hello world"
        )

    def test_single_line_string_with_explicit_return_complex(self):
        script = QgsEsriArcadeScript(
            "return ( ($feature.POP_2010 - $feature.POP_2000) / $feature.POP_2000) * 100;"
        )
        # TODO
        self.assertFalse(script.hasParserError())

        self.assertEqual(script.script(), 'return "Hello world"')
        self.assertEqual(script.dump(), 'return "Hello world";')

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsEsriArcadeReturnStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].expression(),
            QgsEsriArcadeLiteralExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().value(), "Hello world"
        )

    def test_basic(self):
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

    def test_complex_assignment(self):
        script = QgsEsriArcadeScript(
            """
var first = "Paul";
var last = "Richard";
var fullName = first + " " + last;
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 3)

        self.assertIsInstance(
            script.rootBlock().statements()[2],
            QgsEsriArcadeVariableDeclarationStatement,
        )
        self.assertEqual(script.rootBlock().statements()[2].variableName(), "fullName")
        self.assertIsInstance(
            script.rootBlock().statements()[2].initializer(),
            QgsEsriArcadeBinaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[2].initializer().binaryOperator(),
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.boPlus,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[2].initializer().leftOperand(),
            QgsEsriArcadeBinaryOperatorExpression,
        )

        self.assertEqual(
            script.rootBlock()
            .statements()[2]
            .initializer()
            .leftOperand()
            .binaryOperator(),
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.boPlus,
        )
        self.assertIsInstance(
            script.rootBlock()
            .statements()[2]
            .initializer()
            .leftOperand()
            .leftOperand(),
            QgsEsriArcadeVariableReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[2]
            .initializer()
            .leftOperand()
            .leftOperand()
            .name(),
            "first",
        )
        self.assertIsInstance(
            script.rootBlock()
            .statements()[2]
            .initializer()
            .leftOperand()
            .rightOperand(),
            QgsEsriArcadeLiteralExpression,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[2]
            .initializer()
            .leftOperand()
            .rightOperand()
            .value(),
            " ",
        )

        self.assertIsInstance(
            script.rootBlock().statements()[2].initializer().rightOperand(),
            QgsEsriArcadeVariableReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[2].initializer().rightOperand().name(),
            "last",
        )

    def test_reassignment(self):
        script = QgsEsriArcadeScript(
            """
            var age = 35;
// reassigned as text
age = "35 years";
            """
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 2)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsEsriArcadeVariableDeclarationStatement,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[1],
            QgsEsriArcadeExpressionStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[1].expression(),
            QgsEsriArcadeLiteralExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[1].expression().value(), "Hello world"
        )


if __name__ == "__main__":
    unittest.main()
