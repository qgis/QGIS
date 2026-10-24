"""QGIS Unit tests for QgsEsriArcadeScript.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import math

from qgis.core import (
    QgsEsriArcadeArrayLiteralExpression,
    QgsEsriArcadeAssignmentStatement,
    QgsEsriArcadeBinaryOperatorExpression,
    QgsEsriArcadeBlockStatement,
    QgsEsriArcadeBreakStatement,
    QgsEsriArcadeContinueStatement,
    QgsEsriArcadeDictionaryElement,
    QgsEsriArcadeDictionaryLiteralExpression,
    QgsEsriArcadeExpressionStatement,
    QgsEsriArcadeForInStatement,
    QgsEsriArcadeForStatement,
    QgsEsriArcadeFunctionCallExpression,
    QgsEsriArcadeFunctionDeclarationStatement,
    QgsEsriArcadeIfStatement,
    QgsEsriArcadeIndexExpression,
    QgsEsriArcadeLiteralExpression,
    QgsEsriArcadeMemberAccessExpression,
    QgsEsriArcadeProfileVariableReferenceExpression,
    QgsEsriArcadeReturnStatement,
    QgsEsriArcadeScript,
    QgsEsriArcadeUnaryOperatorExpression,
    QgsEsriArcadeVariableDeclarationStatement,
    QgsEsriArcadeVariableReferenceExpression,
    QgsEsriArcadeWhileStatement,
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
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.Plus,
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

    def test_number_literals(self):
        script = QgsEsriArcadeScript("10")
        self.assertFalse(script.hasParserError())
        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertEqual(script.rootBlock().statements()[0].expression().value(), 10)

        script = QgsEsriArcadeScript("1.2 + 2.1")
        self.assertFalse(script.hasParserError())
        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertEqual(
            script.rootBlock().statements()[0].expression().leftOperand().value(), 1.2
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().rightOperand().value(), 2.1
        )

        script = QgsEsriArcadeScript("0.0034 + 0.0071")
        self.assertFalse(script.hasParserError())
        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertEqual(
            script.rootBlock().statements()[0].expression().leftOperand().value(),
            0.0034,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().rightOperand().value(),
            0.0071,
        )

        script = QgsEsriArcadeScript("0.255e3 + 0.255e10")
        self.assertFalse(script.hasParserError())
        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertEqual(
            script.rootBlock().statements()[0].expression().leftOperand().value(), 255.0
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().rightOperand().value(),
            2550000000.0,
        )

        script = QgsEsriArcadeScript("0b11111111 + 0b01010101")
        self.assertFalse(script.hasParserError())
        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertEqual(
            script.rootBlock().statements()[0].expression().leftOperand().value(), 255
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().rightOperand().value(), 85
        )

        script = QgsEsriArcadeScript("0o755 + 0o344")
        self.assertFalse(script.hasParserError())
        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertEqual(
            script.rootBlock().statements()[0].expression().leftOperand().value(), 493
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().rightOperand().value(), 228
        )

        script = QgsEsriArcadeScript("0xff + 0x1a + 0x99")
        self.assertFalse(script.hasParserError())
        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .expression()
            .leftOperand()
            .leftOperand()
            .value(),
            255,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .expression()
            .leftOperand()
            .rightOperand()
            .value(),
            26,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().rightOperand().value(), 153
        )

        script = QgsEsriArcadeScript("PI + Infinity")
        self.assertFalse(script.hasParserError())
        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertAlmostEqual(
            script.rootBlock().statements()[0].expression().leftOperand().value(),
            3.141592,
            places=3,
        )
        self.assertTrue(
            math.isinf(
                script.rootBlock().statements()[0].expression().rightOperand().value()
            )
        )

        script = QgsEsriArcadeScript("NaN + -Infinity")
        self.assertFalse(script.hasParserError())
        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertTrue(
            math.isnan(
                script.rootBlock().statements()[0].expression().leftOperand().value()
            )
        )
        self.assertTrue(
            math.isinf(
                script.rootBlock()
                .statements()[0]
                .expression()
                .rightOperand()
                .operand()
                .value()
            )
        )

    def test_string_literals(self):
        script = QgsEsriArcadeScript('''"This is a Text variable"''')
        self.assertFalse(script.hasParserError())
        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertEqual(
            script.rootBlock().statements()[0].expression().value(),
            """This is a Text variable""",
        )

        script = QgsEsriArcadeScript("""'This is a Text variable'""")
        self.assertFalse(script.hasParserError())
        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertEqual(
            script.rootBlock().statements()[0].expression().value(),
            """This is a Text variable""",
        )

        # stupid, not documented, but supported. wonder what other escape characters are supported?
        script = QgsEsriArcadeScript("""'This is a \\'Text\\' variable'""")
        self.assertFalse(script.hasParserError())
        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertEqual(
            script.rootBlock().statements()[0].expression().value(),
            """This is a 'Text' variable""",
        )

        script = QgsEsriArcadeScript('''"This is a \\"Text\\" variable"''')
        self.assertFalse(script.hasParserError())
        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertEqual(
            script.rootBlock().statements()[0].expression().value(),
            """This is a "Text" variable""",
        )

        script = QgsEsriArcadeScript('''"This is a \\\\Text\\/ variable"''')
        self.assertFalse(script.hasParserError())
        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertEqual(
            script.rootBlock().statements()[0].expression().value(),
            """This is a \\Text/ variable""",
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
        self.assertFalse(script.hasParserError())

        self.assertEqual(
            script.dump(),
            "return ($feature.POP_2010 - $feature.POP_2000) / $feature.POP_2000 * 100;",
        )

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsEsriArcadeReturnStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].expression(),
            QgsEsriArcadeBinaryOperatorExpression,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].expression().leftOperand(),
            QgsEsriArcadeBinaryOperatorExpression,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].expression().leftOperand().leftOperand(),
            QgsEsriArcadeBinaryOperatorExpression,
        )

        self.assertIsInstance(
            script.rootBlock()
            .statements()[0]
            .expression()
            .leftOperand()
            .leftOperand()
            .leftOperand(),
            QgsEsriArcadeMemberAccessExpression,
        )

        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .expression()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .memberName(),
            "POP_2010",
        )
        self.assertIsInstance(
            script.rootBlock()
            .statements()[0]
            .expression()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .containerExpression(),
            QgsEsriArcadeProfileVariableReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .expression()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .containerExpression()
            .name(),
            "$feature",
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
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.Plus,
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
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.Plus,
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
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.Plus,
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
            QgsEsriArcadeAssignmentStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[1].target(),
            QgsEsriArcadeVariableReferenceExpression,
        )
        self.assertEqual(script.rootBlock().statements()[1].target().name(), "age")
        self.assertIsInstance(
            script.rootBlock().statements()[1].value(),
            QgsEsriArcadeLiteralExpression,
        )
        self.assertEqual(script.rootBlock().statements()[1].value().value(), "35 years")

    def test_profile_variable(self):
        script = QgsEsriArcadeScript(
            """var percentCollege = $feature.COLLEGE / $feature.POPULATION;"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsEsriArcadeVariableDeclarationStatement,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].initializer(),
            QgsEsriArcadeBinaryOperatorExpression,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].initializer().leftOperand(),
            QgsEsriArcadeMemberAccessExpression,
        )

        self.assertEqual(
            script.rootBlock().statements()[0].initializer().leftOperand().memberName(),
            "COLLEGE",
        )
        self.assertIsInstance(
            script.rootBlock()
            .statements()[0]
            .initializer()
            .leftOperand()
            .containerExpression(),
            QgsEsriArcadeProfileVariableReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .initializer()
            .leftOperand()
            .containerExpression()
            .name(),
            "$feature",
        )

        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .initializer()
            .rightOperand()
            .memberName(),
            "POPULATION",
        )
        self.assertIsInstance(
            script.rootBlock()
            .statements()[0]
            .initializer()
            .rightOperand()
            .containerExpression(),
            QgsEsriArcadeProfileVariableReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .initializer()
            .rightOperand()
            .containerExpression()
            .name(),
            "$feature",
        )

    def test_profile_variable_bracket(self):
        script = QgsEsriArcadeScript(
            """var percentCollege = $feature["joinKey.fieldName"] / $feature["POPULATION"];"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsEsriArcadeVariableDeclarationStatement,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].initializer(),
            QgsEsriArcadeBinaryOperatorExpression,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].initializer().leftOperand(),
            QgsEsriArcadeIndexExpression,
        )

        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .initializer()
            .leftOperand()
            .indexExpression()
            .value(),
            "joinKey.fieldName",
        )
        self.assertIsInstance(
            script.rootBlock()
            .statements()[0]
            .initializer()
            .leftOperand()
            .containerExpression(),
            QgsEsriArcadeProfileVariableReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .initializer()
            .leftOperand()
            .containerExpression()
            .name(),
            "$feature",
        )

        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .initializer()
            .rightOperand()
            .indexExpression()
            .value(),
            "POPULATION",
        )
        self.assertIsInstance(
            script.rootBlock()
            .statements()[0]
            .initializer()
            .rightOperand()
            .containerExpression(),
            QgsEsriArcadeProfileVariableReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .initializer()
            .rightOperand()
            .containerExpression()
            .name(),
            "$feature",
        )

    def test_array(self):
        script = QgsEsriArcadeScript(
            """var a = [];
a = [5,2,19];
var z = [1,2,3];
var k = [1,2,"hello"];
z[0] = 23;
z[3] = 24;
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 6)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsEsriArcadeVariableDeclarationStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].initializer(),
            QgsEsriArcadeArrayLiteralExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].initializer().elements(), []
        )

        self.assertIsInstance(
            script.rootBlock().statements()[1],
            QgsEsriArcadeAssignmentStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[1].value(),
            QgsEsriArcadeArrayLiteralExpression,
        )
        self.assertEqual(
            [l.value() for l in script.rootBlock().statements()[1].value().elements()],
            [5, 2, 19],
        )

        self.assertIsInstance(
            script.rootBlock().statements()[2],
            QgsEsriArcadeVariableDeclarationStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[2].initializer(),
            QgsEsriArcadeArrayLiteralExpression,
        )
        self.assertEqual(
            [
                l.value()
                for l in script.rootBlock().statements()[2].initializer().elements()
            ],
            [1, 2, 3],
        )

        self.assertIsInstance(
            script.rootBlock().statements()[3],
            QgsEsriArcadeVariableDeclarationStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[3].initializer(),
            QgsEsriArcadeArrayLiteralExpression,
        )
        self.assertEqual(
            [
                l.value()
                for l in script.rootBlock().statements()[3].initializer().elements()
            ],
            [1, 2, "hello"],
        )

        self.assertIsInstance(
            script.rootBlock().statements()[4],
            QgsEsriArcadeAssignmentStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[4].target(),
            QgsEsriArcadeIndexExpression,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[4].target().containerExpression(),
            QgsEsriArcadeVariableReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[4].target().containerExpression().name(),
            "z",
        )
        self.assertIsInstance(
            script.rootBlock().statements()[4].target().indexExpression(),
            QgsEsriArcadeLiteralExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[4].target().indexExpression().value(), 0
        )

        self.assertIsInstance(
            script.rootBlock().statements()[4].value(),
            QgsEsriArcadeLiteralExpression,
        )
        self.assertEqual(script.rootBlock().statements()[4].value().value(), 23)

    def test_boolean_literals(self):
        script = QgsEsriArcadeScript(
            """var x = true;
var y = false;
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
            script.rootBlock().statements()[0].initializer(),
            QgsEsriArcadeLiteralExpression,
        )
        self.assertEqual(script.rootBlock().statements()[0].initializer().value(), True)

        self.assertIsInstance(
            script.rootBlock().statements()[1],
            QgsEsriArcadeVariableDeclarationStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[1].initializer(),
            QgsEsriArcadeLiteralExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[1].initializer().value(), False
        )

    def test_dictionary_literals(self):
        script = QgsEsriArcadeScript(
            """var d = {
  field1: 1,
  field2: 2,
  "full name": "Fred Barker"
};
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsEsriArcadeVariableDeclarationStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].initializer(),
            QgsEsriArcadeDictionaryLiteralExpression,
        )
        self.assertEqual(
            len(script.rootBlock().statements()[0].initializer().elements()), 3
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].initializer().elements()[0],
            QgsEsriArcadeDictionaryElement,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].initializer().elements()[0].key(),
            "field1",
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .initializer()
            .elements()[0]
            .value()
            .value(),
            1,
        )
        self.assertFalse(
            script.rootBlock().statements()[0].initializer().elements()[0].isAutoKey()
        )

        self.assertEqual(
            script.rootBlock().statements()[0].initializer().elements()[1].key(),
            "field2",
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .initializer()
            .elements()[1]
            .value()
            .value(),
            2,
        )
        self.assertFalse(
            script.rootBlock().statements()[0].initializer().elements()[1].isAutoKey()
        )

        self.assertEqual(
            script.rootBlock().statements()[0].initializer().elements()[2].key(),
            "full name",
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .initializer()
            .elements()[2]
            .value()
            .value(),
            "Fred Barker",
        )
        self.assertFalse(
            script.rootBlock().statements()[0].initializer().elements()[2].isAutoKey()
        )

    def test_dictionary_access(self):
        script = QgsEsriArcadeScript(
            """var a = d.field1;
            var b = d?.field2;
            var c = d["field3"];
            var e = d["field 4"];
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 4)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsEsriArcadeVariableDeclarationStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].initializer(),
            QgsEsriArcadeMemberAccessExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].initializer().memberName(), "field1"
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .initializer()
            .containerExpression()
            .name(),
            "d",
        )
        self.assertFalse(
            script.rootBlock().statements()[0].initializer().isSafeAccess()
        )

        self.assertIsInstance(
            script.rootBlock().statements()[1],
            QgsEsriArcadeVariableDeclarationStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[1].initializer(),
            QgsEsriArcadeMemberAccessExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[1].initializer().memberName(), "field2"
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[1]
            .initializer()
            .containerExpression()
            .name(),
            "d",
        )
        self.assertTrue(script.rootBlock().statements()[1].initializer().isSafeAccess())

        self.assertIsInstance(
            script.rootBlock().statements()[2],
            QgsEsriArcadeVariableDeclarationStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[2].initializer(),
            QgsEsriArcadeIndexExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[2].initializer().indexExpression().value(),
            "field3",
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[2]
            .initializer()
            .containerExpression()
            .name(),
            "d",
        )

        self.assertIsInstance(
            script.rootBlock().statements()[3],
            QgsEsriArcadeVariableDeclarationStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[3].initializer(),
            QgsEsriArcadeIndexExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[3].initializer().indexExpression().value(),
            "field 4",
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[3]
            .initializer()
            .containerExpression()
            .name(),
            "d",
        )

    def test_dictionary_auto_keys(self):
        script = QgsEsriArcadeScript(
            """var field1 = 1;
var field2 = 2;
var d = { field1, field2 };
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 3)

        self.assertIsInstance(
            script.rootBlock().statements()[2].initializer(),
            QgsEsriArcadeDictionaryLiteralExpression,
        )
        self.assertEqual(
            len(script.rootBlock().statements()[2].initializer().elements()), 2
        )

        self.assertIsInstance(
            script.rootBlock().statements()[2].initializer().elements()[0],
            QgsEsriArcadeDictionaryElement,
        )
        self.assertEqual(
            script.rootBlock().statements()[2].initializer().elements()[0].key(),
            "field1",
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[2]
            .initializer()
            .elements()[0]
            .value()
            .name(),
            "field1",
        )
        self.assertTrue(
            script.rootBlock().statements()[2].initializer().elements()[0].isAutoKey()
        )

        self.assertEqual(
            script.rootBlock().statements()[2].initializer().elements()[1].key(),
            "field2",
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[2]
            .initializer()
            .elements()[1]
            .value()
            .name(),
            "field2",
        )
        self.assertTrue(
            script.rootBlock().statements()[2].initializer().elements()[1].isAutoKey()
        )

    def test_function_call(self):
        script = QgsEsriArcadeScript(
            """function calculateStuff(num) {
var places = 2;
return num/places;
}

calculateStuff(50);
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 2)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsEsriArcadeFunctionDeclarationStatement,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[1],
            QgsEsriArcadeExpressionStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[1].expression(),
            QgsEsriArcadeFunctionCallExpression,
        )

        self.assertEqual(
            script.rootBlock().statements()[1].expression().functionName(),
            "calculateStuff",
        )

        self.assertEqual(
            len(script.rootBlock().statements()[1].expression().arguments()), 1
        )

        self.assertEqual(
            script.rootBlock().statements()[1].expression().arguments()[0].value(), 50
        )

    def test_binary_operators(self):
        script = QgsEsriArcadeScript(
            """var x = 10 + 20;
            var y = 10 - 20;
            var z = 10 * 20;
            var a = 10 / 20;
            var b = 10 % 20;
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 5)

        self.assertIsInstance(
            script.rootBlock().statements()[0].initializer(),
            QgsEsriArcadeBinaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].initializer().binaryOperator(),
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.Plus,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].initializer().leftOperand().value(), 10
        )
        self.assertEqual(
            script.rootBlock().statements()[0].initializer().rightOperand().value(), 20
        )

        self.assertEqual(
            script.rootBlock().statements()[1].initializer().binaryOperator(),
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.Minus,
        )
        self.assertEqual(
            script.rootBlock().statements()[1].initializer().leftOperand().value(), 10
        )
        self.assertEqual(
            script.rootBlock().statements()[1].initializer().rightOperand().value(), 20
        )

        self.assertEqual(
            script.rootBlock().statements()[2].initializer().binaryOperator(),
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.Multiply,
        )
        self.assertEqual(
            script.rootBlock().statements()[2].initializer().leftOperand().value(), 10
        )
        self.assertEqual(
            script.rootBlock().statements()[2].initializer().rightOperand().value(), 20
        )

        self.assertEqual(
            script.rootBlock().statements()[3].initializer().binaryOperator(),
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.Divide,
        )
        self.assertEqual(
            script.rootBlock().statements()[3].initializer().leftOperand().value(), 10
        )
        self.assertEqual(
            script.rootBlock().statements()[3].initializer().rightOperand().value(), 20
        )

        self.assertEqual(
            script.rootBlock().statements()[4].initializer().binaryOperator(),
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.Modulo,
        )
        self.assertEqual(
            script.rootBlock().statements()[4].initializer().leftOperand().value(), 10
        )
        self.assertEqual(
            script.rootBlock().statements()[4].initializer().rightOperand().value(), 20
        )

    def test_binary_comparisons(self):
        script = QgsEsriArcadeScript(
            """10 == 20;
            10 != 20;
            10 > 20;
            10 >= 20;
            10 < 20;
            10 <= 20;
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 6)

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
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.Equal,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().leftOperand().value(), 10
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().rightOperand().value(), 20
        )

        self.assertIsInstance(
            script.rootBlock().statements()[1].expression(),
            QgsEsriArcadeBinaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[1].expression().binaryOperator(),
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.NotEqual,
        )
        self.assertEqual(
            script.rootBlock().statements()[1].expression().leftOperand().value(), 10
        )
        self.assertEqual(
            script.rootBlock().statements()[1].expression().rightOperand().value(), 20
        )

        self.assertIsInstance(
            script.rootBlock().statements()[2].expression(),
            QgsEsriArcadeBinaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[2].expression().binaryOperator(),
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.GreaterThan,
        )
        self.assertEqual(
            script.rootBlock().statements()[2].expression().leftOperand().value(), 10
        )
        self.assertEqual(
            script.rootBlock().statements()[2].expression().rightOperand().value(), 20
        )

        self.assertIsInstance(
            script.rootBlock().statements()[3].expression(),
            QgsEsriArcadeBinaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[3].expression().binaryOperator(),
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.GreaterThanOrEqual,
        )
        self.assertEqual(
            script.rootBlock().statements()[3].expression().leftOperand().value(), 10
        )
        self.assertEqual(
            script.rootBlock().statements()[3].expression().rightOperand().value(), 20
        )

        self.assertIsInstance(
            script.rootBlock().statements()[4].expression(),
            QgsEsriArcadeBinaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[4].expression().binaryOperator(),
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.LessThan,
        )
        self.assertEqual(
            script.rootBlock().statements()[4].expression().leftOperand().value(), 10
        )
        self.assertEqual(
            script.rootBlock().statements()[4].expression().rightOperand().value(), 20
        )

        self.assertIsInstance(
            script.rootBlock().statements()[5].expression(),
            QgsEsriArcadeBinaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[5].expression().binaryOperator(),
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.LessThanOrEqual,
        )
        self.assertEqual(
            script.rootBlock().statements()[5].expression().leftOperand().value(), 10
        )
        self.assertEqual(
            script.rootBlock().statements()[4].expression().rightOperand().value(), 20
        )

    def test_logical_operators(self):
        script = QgsEsriArcadeScript(
            """10 == 20 || y == 9;
            10 == 20 && y == 9;
            10 == 20 && !b;
            10 == 20 && !(a==b);
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 4)

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
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.LogicalOr,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].expression().leftOperand(),
            QgsEsriArcadeBinaryOperatorExpression,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].expression().rightOperand(),
            QgsEsriArcadeBinaryOperatorExpression,
        )

        self.assertEqual(
            script.rootBlock().statements()[1].expression().binaryOperator(),
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.LogicalAnd,
        )

        self.assertEqual(
            script.rootBlock().statements()[2].expression().binaryOperator(),
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.LogicalAnd,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[2].expression().rightOperand(),
            QgsEsriArcadeUnaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[2]
            .expression()
            .rightOperand()
            .unaryOperator(),
            QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.Not,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[2]
            .expression()
            .rightOperand()
            .operand()
            .name(),
            "b",
        )

        self.assertEqual(
            script.rootBlock().statements()[3].expression().binaryOperator(),
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.LogicalAnd,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[3].expression().rightOperand(),
            QgsEsriArcadeUnaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[3]
            .expression()
            .rightOperand()
            .unaryOperator(),
            QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.Not,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[3].expression().rightOperand().operand(),
            QgsEsriArcadeBinaryOperatorExpression,
        )

    def test_unary_operators(self):
        script = QgsEsriArcadeScript(
            """var x = 10;
            x++;
            ++x;
            x--;
            --x;
            var y = +10;
            var z = -y;
            var z = +y;
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 8)

        self.assertIsInstance(
            script.rootBlock().statements()[1],
            QgsEsriArcadeExpressionStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[1].expression(),
            QgsEsriArcadeUnaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[1].expression().unaryOperator(),
            QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.PostIncrement,
        )
        self.assertEqual(
            script.rootBlock().statements()[1].expression().operand().name(),
            "x",
        )

        self.assertIsInstance(
            script.rootBlock().statements()[2].expression(),
            QgsEsriArcadeUnaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[2].expression().unaryOperator(),
            QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.PreIncrement,
        )
        self.assertEqual(
            script.rootBlock().statements()[2].expression().operand().name(),
            "x",
        )

        self.assertIsInstance(
            script.rootBlock().statements()[3].expression(),
            QgsEsriArcadeUnaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[3].expression().unaryOperator(),
            QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.PostDecrement,
        )
        self.assertEqual(
            script.rootBlock().statements()[3].expression().operand().name(),
            "x",
        )

        self.assertIsInstance(
            script.rootBlock().statements()[4].expression(),
            QgsEsriArcadeUnaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[4].expression().unaryOperator(),
            QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.PreDecrement,
        )
        self.assertEqual(
            script.rootBlock().statements()[4].expression().operand().name(),
            "x",
        )

        self.assertIsInstance(
            script.rootBlock().statements()[5].initializer(),
            QgsEsriArcadeUnaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[5].initializer().unaryOperator(),
            QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.Plus,
        )
        self.assertEqual(
            script.rootBlock().statements()[5].initializer().operand().value(), 10
        )

        self.assertIsInstance(
            script.rootBlock().statements()[6].initializer(),
            QgsEsriArcadeUnaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[6].initializer().unaryOperator(),
            QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.Minus,
        )
        self.assertEqual(
            script.rootBlock().statements()[6].initializer().operand().name(), "y"
        )

        self.assertIsInstance(
            script.rootBlock().statements()[7].initializer(),
            QgsEsriArcadeUnaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[7].initializer().unaryOperator(),
            QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.Plus,
        )
        self.assertEqual(
            script.rootBlock().statements()[7].initializer().operand().name(), "y"
        )

    def test_assignment_operators(self):
        script = QgsEsriArcadeScript(
            """var x = 10;
            x += 5;
            x -= 2;
            x *= 3;
            x /= 4;
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 5)

        self.assertIsInstance(
            script.rootBlock().statements()[1],
            QgsEsriArcadeAssignmentStatement,
        )
        self.assertEqual(
            script.rootBlock().statements()[1].target().name(),
            "x",
        )
        self.assertEqual(
            script.rootBlock().statements()[1].value().value(),
            5,
        )
        self.assertEqual(
            script.rootBlock().statements()[1].assignmentOperator(),
            QgsEsriArcadeAssignmentStatement.AssignmentOperator.PlusAssign,
        )

        self.assertEqual(
            script.rootBlock().statements()[2].target().name(),
            "x",
        )
        self.assertEqual(
            script.rootBlock().statements()[2].value().value(),
            2,
        )
        self.assertEqual(
            script.rootBlock().statements()[2].assignmentOperator(),
            QgsEsriArcadeAssignmentStatement.AssignmentOperator.MinusAssign,
        )

        self.assertEqual(
            script.rootBlock().statements()[3].target().name(),
            "x",
        )
        self.assertEqual(
            script.rootBlock().statements()[3].value().value(),
            3,
        )
        self.assertEqual(
            script.rootBlock().statements()[3].assignmentOperator(),
            QgsEsriArcadeAssignmentStatement.AssignmentOperator.MultiplyAssign,
        )

        self.assertEqual(
            script.rootBlock().statements()[4].target().name(),
            "x",
        )
        self.assertEqual(
            script.rootBlock().statements()[4].value().value(),
            4,
        )
        self.assertEqual(
            script.rootBlock().statements()[4].assignmentOperator(),
            QgsEsriArcadeAssignmentStatement.AssignmentOperator.DivideAssign,
        )

    def test_bitwise_operators(self):
        script = QgsEsriArcadeScript(
            """7 & 14;
            7 | 14;
            7 ^ 14;
            ~7;
            7 << 2;
            7 >> 2;
            7 >>> 2;
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 7)

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
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.BitwiseAnd,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[1],
            QgsEsriArcadeExpressionStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[1].expression(),
            QgsEsriArcadeBinaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[1].expression().binaryOperator(),
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.BitwiseOr,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[2],
            QgsEsriArcadeExpressionStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[2].expression(),
            QgsEsriArcadeBinaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[2].expression().binaryOperator(),
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.BitwiseXor,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[3],
            QgsEsriArcadeExpressionStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[3].expression(),
            QgsEsriArcadeUnaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[3].expression().unaryOperator(),
            QgsEsriArcadeUnaryOperatorExpression.UnaryOperator.BitwiseNot,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[4],
            QgsEsriArcadeExpressionStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[4].expression(),
            QgsEsriArcadeBinaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[4].expression().binaryOperator(),
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.BitwiseLeftShift,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[5],
            QgsEsriArcadeExpressionStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[5].expression(),
            QgsEsriArcadeBinaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[5].expression().binaryOperator(),
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.BitwiseSignedRightShift,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[6],
            QgsEsriArcadeExpressionStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[6].expression(),
            QgsEsriArcadeBinaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[6].expression().binaryOperator(),
            QgsEsriArcadeBinaryOperatorExpression.BinaryOperator.BitwiseUnsignedRightShift,
        )

    def test_if(self):
        script = QgsEsriArcadeScript(
            """if(x > 5){
            x+= 5;
  return x;
}
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsEsriArcadeIfStatement,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].condition(),
            QgsEsriArcadeBinaryOperatorExpression,
        )

        self.assertIsNone(script.rootBlock().statements()[0].falseBlock())

        self.assertIsInstance(
            script.rootBlock().statements()[0].trueBlock(),
            QgsEsriArcadeBlockStatement,
        )

        self.assertEqual(script.rootBlock().statements()[0].trueBlock().count(), 2)

        self.assertIsInstance(
            script.rootBlock().statements()[0].trueBlock().statements()[0],
            QgsEsriArcadeAssignmentStatement,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].trueBlock().statements()[1],
            QgsEsriArcadeReturnStatement,
        )

    def test_if_else(self):
        script = QgsEsriArcadeScript(
            """if(x > 5){
  return x;
  } else {
  return 5;
}
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsEsriArcadeIfStatement,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].condition(),
            QgsEsriArcadeBinaryOperatorExpression,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].trueBlock(),
            QgsEsriArcadeBlockStatement,
        )

        self.assertEqual(script.rootBlock().statements()[0].trueBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0].trueBlock().statements()[0],
            QgsEsriArcadeReturnStatement,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].falseBlock(),
            QgsEsriArcadeBlockStatement,
        )

        self.assertEqual(script.rootBlock().statements()[0].falseBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0].falseBlock().statements()[0],
            QgsEsriArcadeReturnStatement,
        )

        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .falseBlock()
            .statements()[0]
            .expression()
            .value(),
            5,
        )

    def test_for(self):
        script = QgsEsriArcadeScript(
            """for(var z=0; z < 100; z++) {
  result += z;
}
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsEsriArcadeForStatement,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].initialization(),
            QgsEsriArcadeVariableDeclarationStatement,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].condition(),
            QgsEsriArcadeBinaryOperatorExpression,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].step(),
            QgsEsriArcadeUnaryOperatorExpression,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].body(),
            QgsEsriArcadeBlockStatement,
        )

        self.assertEqual(script.rootBlock().statements()[0].body().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0].body().statements()[0],
            QgsEsriArcadeAssignmentStatement,
        )

    def test_for_in(self):
        script = QgsEsriArcadeScript(
            """for(var index in myArray)
            {
  result += 2;
}
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsEsriArcadeForInStatement,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].iterationType(),
            QgsEsriArcadeForInStatement.IterationType.ForIn,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].variableName(),
            "index",
        )
        self.assertTrue(script.rootBlock().statements()[0].isVariableDeclaration())
        self.assertIsInstance(
            script.rootBlock().statements()[0].collection(),
            QgsEsriArcadeVariableReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].collection().name(),
            "myArray",
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].body(),
            QgsEsriArcadeBlockStatement,
        )

        self.assertEqual(script.rootBlock().statements()[0].body().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0].body().statements()[0],
            QgsEsriArcadeAssignmentStatement,
        )

    def test_for_in_no_declare(self):
        script = QgsEsriArcadeScript(
            """for(index in myArray)
            {
  result += 2;
}
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsEsriArcadeForInStatement,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].iterationType(),
            QgsEsriArcadeForInStatement.IterationType.ForIn,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].variableName(),
            "index",
        )
        self.assertFalse(script.rootBlock().statements()[0].isVariableDeclaration())
        self.assertIsInstance(
            script.rootBlock().statements()[0].collection(),
            QgsEsriArcadeVariableReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].collection().name(),
            "myArray",
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].body(),
            QgsEsriArcadeBlockStatement,
        )

        self.assertEqual(script.rootBlock().statements()[0].body().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0].body().statements()[0],
            QgsEsriArcadeAssignmentStatement,
        )

    def test_for_of(self):
        script = QgsEsriArcadeScript(
            """for(var value of myArray)
            {
  break;
  continue;
}
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsEsriArcadeForInStatement,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].iterationType(),
            QgsEsriArcadeForInStatement.IterationType.ForOf,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].variableName(),
            "value",
        )
        self.assertTrue(script.rootBlock().statements()[0].isVariableDeclaration())
        self.assertIsInstance(
            script.rootBlock().statements()[0].collection(),
            QgsEsriArcadeVariableReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].collection().name(),
            "myArray",
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].body(),
            QgsEsriArcadeBlockStatement,
        )

        self.assertEqual(script.rootBlock().statements()[0].body().count(), 2)

        self.assertIsInstance(
            script.rootBlock().statements()[0].body().statements()[0],
            QgsEsriArcadeBreakStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].body().statements()[1],
            QgsEsriArcadeContinueStatement,
        )

    def test_while(self):
        script = QgsEsriArcadeScript(
            """while(y <= 2020){
  total += y;
}
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsEsriArcadeWhileStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].condition(),
            QgsEsriArcadeBinaryOperatorExpression,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].body(),
            QgsEsriArcadeBlockStatement,
        )

        self.assertEqual(script.rootBlock().statements()[0].body().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0].body().statements()[0],
            QgsEsriArcadeAssignmentStatement,
        )

    def test_string_constants(self):
        script = QgsEsriArcadeScript(
            """var a = TextFormatting.BackwardSlash +  'a' + TextFormatting.DoubleQuote + 'b' + TextFormatting.ForwardSlash + 'c' + TextFormatting.NewLine + 'd' + TextFormatting.SingleQuote;"""
        )

        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsEsriArcadeBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsEsriArcadeVariableDeclarationStatement,
        )

        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .initializer()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .value(),
            "\\",
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .initializer()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .rightOperand()
            .value(),
            "a",
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .initializer()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .rightOperand()
            .value(),
            '"',
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .initializer()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .rightOperand()
            .value(),
            "b",
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .initializer()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .rightOperand()
            .value(),
            "/",
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .initializer()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .rightOperand()
            .value(),
            "c",
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .initializer()
            .leftOperand()
            .leftOperand()
            .rightOperand()
            .value(),
            "\n",
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .initializer()
            .leftOperand()
            .rightOperand()
            .value(),
            "d",
        )
        self.assertEqual(
            script.rootBlock().statements()[0].initializer().rightOperand().value(),
            "'",
        )


if __name__ == "__main__":
    unittest.main()
