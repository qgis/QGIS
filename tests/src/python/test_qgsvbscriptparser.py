"""QGIS Unit tests for QgVbScript.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import math

from qgis.core import (
    QgsVbScript,
    QgsVbScriptArrayLiteralExpression,
    QgsVbScriptAssignmentStatement,
    QgsVbScriptBinaryOperatorExpression,
    QgsVbScriptBlockStatement,
    QgsVbScriptExpressionStatement,
    QgsVbScriptFieldReferenceExpression,
    QgsVbScriptForEachStatement,
    QgsVbScriptForStatement,
    QgsVbScriptFunctionCallExpression,
    QgsVbScriptFunctionDeclarationStatement,
    QgsVbScriptIfStatement,
    QgsVbScriptIndexExpression,
    QgsVbScriptLiteralExpression,
    QgsVbScriptMemberAccessExpression,
    QgsVbScriptUnaryOperatorExpression,
    QgsVbScriptVariableDeclarationStatement,
    QgsVbScriptVariableReferenceExpression,
    QgsVbScriptWhileStatement,
)
from qgis.testing import unittest


class TestQgsVbScript(unittest.TestCase):
    def test_parser_error(self):
        script = QgsVbScript("""x = """)
        self.assertTrue(script.hasParserError())
        self.assertEqual(script.parserErrorString(), "syntax error")

    def test_single_line_string(self):
        script = QgsVbScript('"Hello world"')
        self.assertFalse(script.hasParserError())

        self.assertEqual(script.script(), '"Hello world"')
        self.assertEqual(script.dump(), '"Hello world"')

        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsVbScriptExpressionStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].expression(),
            QgsVbScriptLiteralExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().value(), "Hello world"
        )

    def test_single_line_expression(self):
        script = QgsVbScript("1 + 2")
        self.assertFalse(script.hasParserError())

        self.assertEqual(script.script(), "1 + 2")
        self.assertEqual(script.dump(), "1 + 2")

        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsVbScriptExpressionStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].expression(),
            QgsVbScriptBinaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().binaryOperator(),
            QgsVbScriptBinaryOperatorExpression.BinaryOperator.Plus,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].expression().leftOperand(),
            QgsVbScriptLiteralExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().leftOperand().value(), 1
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].expression().rightOperand(),
            QgsVbScriptLiteralExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().rightOperand().value(), 2
        )

    def test_number_literals(self):
        script = QgsVbScript("10")
        self.assertFalse(script.hasParserError())
        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertEqual(script.rootBlock().statements()[0].expression().value(), 10)

        script = QgsVbScript("1.2 + 2.1")
        self.assertFalse(script.hasParserError())
        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertEqual(
            script.rootBlock().statements()[0].expression().leftOperand().value(), 1.2
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().rightOperand().value(), 2.1
        )

        script = QgsVbScript("0.0034 + 0.0071")
        self.assertFalse(script.hasParserError())
        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertEqual(
            script.rootBlock().statements()[0].expression().leftOperand().value(),
            0.0034,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().rightOperand().value(),
            0.0071,
        )

        script = QgsVbScript("0.255e3 + 0.255e10")
        self.assertFalse(script.hasParserError())
        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertEqual(
            script.rootBlock().statements()[0].expression().leftOperand().value(), 255.0
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().rightOperand().value(),
            2550000000.0,
        )

        script = QgsVbScript("&o755 + &o344")
        self.assertFalse(script.hasParserError())
        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertEqual(
            script.rootBlock().statements()[0].expression().leftOperand().value(), 493
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().rightOperand().value(), 228
        )

        script = QgsVbScript("&hff + &h1a + &h99")
        self.assertFalse(script.hasParserError())
        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
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

    def test_string_literals(self):
        script = QgsVbScript('''"This is a Text variable"''')
        self.assertFalse(script.hasParserError())
        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertEqual(
            script.rootBlock().statements()[0].expression().value(),
            """This is a Text variable""",
        )

        script = QgsVbScript('''"This is a ""Text"" variable"''')
        self.assertFalse(script.hasParserError())
        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertEqual(
            script.rootBlock().statements()[0].expression().value(),
            """This is a "Text" variable""",
        )

    def test_single_line_string_complex(self):
        script = QgsVbScript("( (POP_2010 - POP_2000) / POP_2000) * 100")
        self.assertFalse(script.hasParserError())

        self.assertEqual(
            script.dump(),
            "(POP_2010 - POP_2000) / POP_2000 * 100",
        )

        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsVbScriptExpressionStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].expression(),
            QgsVbScriptBinaryOperatorExpression,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].expression().leftOperand(),
            QgsVbScriptBinaryOperatorExpression,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].expression().leftOperand().leftOperand(),
            QgsVbScriptBinaryOperatorExpression,
        )

        self.assertIsInstance(
            script.rootBlock()
            .statements()[0]
            .expression()
            .leftOperand()
            .leftOperand()
            .leftOperand(),
            QgsVbScriptVariableReferenceExpression,
        )

        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .expression()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .name(),
            "POP_2010",
        )

    def test_basic(self):
        script = QgsVbScript("""x = 3
y = 4
x + y""")

        self.assertFalse(script.hasParserError())

        self.assertEqual(
            script.script(),
            """x = 3
y = 4
x + y""",
        )
        self.assertEqual(
            script.dump(),
            """x = 3
y = 4
x + y""",
        )

        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 3)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsVbScriptAssignmentStatement,
        )
        self.assertEqual(script.rootBlock().statements()[0].target().name(), "x")
        self.assertIsInstance(
            script.rootBlock().statements()[0].value(),
            QgsVbScriptLiteralExpression,
        )
        self.assertEqual(script.rootBlock().statements()[0].value().value(), 3)

        self.assertIsInstance(
            script.rootBlock().statements()[1],
            QgsVbScriptAssignmentStatement,
        )
        self.assertEqual(script.rootBlock().statements()[1].target().name(), "y")
        self.assertIsInstance(
            script.rootBlock().statements()[1].value(),
            QgsVbScriptLiteralExpression,
        )
        self.assertEqual(script.rootBlock().statements()[1].value().value(), 4)

        self.assertIsInstance(
            script.rootBlock().statements()[2], QgsVbScriptExpressionStatement
        )

        self.assertIsInstance(
            script.rootBlock().statements()[2].expression(),
            QgsVbScriptBinaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[2].expression().binaryOperator(),
            QgsVbScriptBinaryOperatorExpression.BinaryOperator.Plus,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[2].expression().leftOperand(),
            QgsVbScriptVariableReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[2].expression().leftOperand().name(), "x"
        )

        self.assertIsInstance(
            script.rootBlock().statements()[2].expression().rightOperand(),
            QgsVbScriptVariableReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[2].expression().rightOperand().name(), "y"
        )

    def test_function(self):
        script = QgsVbScript(
            """function calculateRatio(value, index)

    value2 = value * 2
calculateRatio = value / index
end function"""
        )

        self.assertFalse(script.hasParserError())
        self.assertEqual(
            script.dump(),
            """Function calculateRatio(value, index)
value2 = value * 2
calculateRatio = value / index
End Function""",
        )
        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsVbScriptFunctionDeclarationStatement,
        )

        self.assertEqual(
            script.rootBlock().statements()[0].functionName(), "calculateRatio"
        )
        self.assertEqual(
            script.rootBlock().statements()[0].parameters(), ["value", "index"]
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].body(), QgsVbScriptBlockStatement
        )
        self.assertEqual(script.rootBlock().statements()[0].body().count(), 2)

        self.assertIsInstance(
            script.rootBlock().statements()[0].body().statements()[0],
            QgsVbScriptAssignmentStatement,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].body().statements()[0].target().name(),
            "value2",
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].body().statements()[0].value(),
            QgsVbScriptBinaryOperatorExpression,
        )
        self.assertIsInstance(
            script.rootBlock()
            .statements()[0]
            .body()
            .statements()[0]
            .value()
            .leftOperand(),
            QgsVbScriptVariableReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .body()
            .statements()[0]
            .value()
            .leftOperand()
            .name(),
            "value",
        )

    def test_comments(self):
        script = QgsVbScript(
            """x = 3
' this is a comment"""
        )

        self.assertFalse(script.hasParserError())

        self.assertEqual(
            script.script(),
            """x = 3
' this is a comment""",
        )
        # comments are ignored in dump
        self.assertEqual(script.dump(), """x = 3""")
        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)
        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsVbScriptAssignmentStatement,
        )

    def test_complex_assignment(self):
        script = QgsVbScript(
            """
first = "Paul"
last = "Richard"
fullName = first & " " & last
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 3)

        self.assertIsInstance(
            script.rootBlock().statements()[2],
            QgsVbScriptAssignmentStatement,
        )
        self.assertEqual(script.rootBlock().statements()[2].target().name(), "fullName")
        self.assertIsInstance(
            script.rootBlock().statements()[2].value(),
            QgsVbScriptBinaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[2].value().binaryOperator(),
            QgsVbScriptBinaryOperatorExpression.BinaryOperator.Concat,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[2].value().leftOperand(),
            QgsVbScriptBinaryOperatorExpression,
        )

        self.assertEqual(
            script.rootBlock().statements()[2].value().leftOperand().binaryOperator(),
            QgsVbScriptBinaryOperatorExpression.BinaryOperator.Concat,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[2].value().leftOperand().leftOperand(),
            QgsVbScriptVariableReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[2]
            .value()
            .leftOperand()
            .leftOperand()
            .name(),
            "first",
        )
        self.assertIsInstance(
            script.rootBlock().statements()[2].value().leftOperand().rightOperand(),
            QgsVbScriptLiteralExpression,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[2]
            .value()
            .leftOperand()
            .rightOperand()
            .value(),
            " ",
        )

        self.assertIsInstance(
            script.rootBlock().statements()[2].value().rightOperand(),
            QgsVbScriptVariableReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[2].value().rightOperand().name(),
            "last",
        )

    def test_reassignment(self):
        script = QgsVbScript(
            """
         age = 35
' reassigned as text
age = "35 years"
            """
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 2)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsVbScriptAssignmentStatement,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[1],
            QgsVbScriptAssignmentStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[1].target(),
            QgsVbScriptVariableReferenceExpression,
        )
        self.assertEqual(script.rootBlock().statements()[1].target().name(), "age")
        self.assertIsInstance(
            script.rootBlock().statements()[1].value(),
            QgsVbScriptLiteralExpression,
        )
        self.assertEqual(script.rootBlock().statements()[1].value().value(), "35 years")

    def test_array(self):
        script = QgsVbScript(
            """DIM a
A(0) = 10
A(1) = 20
A(2) = 30
A = Array(10, 20, 30)
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 5)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsVbScriptVariableDeclarationStatement,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[1],
            QgsVbScriptAssignmentStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[1].target(),
            QgsVbScriptFunctionCallExpression,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[1].value(),
            QgsVbScriptLiteralExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[1].value().value(),
            10,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[2],
            QgsVbScriptAssignmentStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[2].value(),
            QgsVbScriptLiteralExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[2].value().value(),
            20,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[3],
            QgsVbScriptAssignmentStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[3].value(),
            QgsVbScriptLiteralExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[3].value().value(),
            30,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[4],
            QgsVbScriptAssignmentStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[4].value(),
            QgsVbScriptArrayLiteralExpression,
        )
        self.assertEqual(
            [l.value() for l in script.rootBlock().statements()[4].value().elements()],
            [10, 20, 30],
        )

    def test_boolean_literals(self):
        script = QgsVbScript(
            """x = true
y = false
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 2)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsVbScriptAssignmentStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].value(),
            QgsVbScriptLiteralExpression,
        )
        self.assertEqual(script.rootBlock().statements()[0].value().value(), True)

        self.assertIsInstance(
            script.rootBlock().statements()[1],
            QgsVbScriptAssignmentStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[1].value(),
            QgsVbScriptLiteralExpression,
        )
        self.assertEqual(script.rootBlock().statements()[1].value().value(), False)

    def test_function_call(self):
        script = QgsVbScript(
            """function calculateStuff(num)
places = 2
calculateStuff = num/places
end function

calculateStuff(50)
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 2)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsVbScriptFunctionDeclarationStatement,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[1],
            QgsVbScriptExpressionStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[1].expression(),
            QgsVbScriptFunctionCallExpression,
        )

        self.assertEqual(
            script.rootBlock().statements()[1].expression().targetExpression().name(),
            "calculateStuff",
        )

        self.assertEqual(
            len(script.rootBlock().statements()[1].expression().arguments()), 1
        )

        self.assertEqual(
            script.rootBlock().statements()[1].expression().arguments()[0].value(), 50
        )

    def test_binary_operators(self):
        script = QgsVbScript(
            """x = 10 + 20
            y = 10 - 20
            z = 10 * 20
            a = 10 / 20
            b = 10 Mod 20
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 5)

        self.assertIsInstance(
            script.rootBlock().statements()[0].value(),
            QgsVbScriptBinaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].value().binaryOperator(),
            QgsVbScriptBinaryOperatorExpression.BinaryOperator.Plus,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].value().leftOperand().value(), 10
        )
        self.assertEqual(
            script.rootBlock().statements()[0].value().rightOperand().value(), 20
        )

        self.assertEqual(
            script.rootBlock().statements()[1].value().binaryOperator(),
            QgsVbScriptBinaryOperatorExpression.BinaryOperator.Minus,
        )
        self.assertEqual(
            script.rootBlock().statements()[1].value().leftOperand().value(), 10
        )
        self.assertEqual(
            script.rootBlock().statements()[1].value().rightOperand().value(), 20
        )

        self.assertEqual(
            script.rootBlock().statements()[2].value().binaryOperator(),
            QgsVbScriptBinaryOperatorExpression.BinaryOperator.Multiply,
        )
        self.assertEqual(
            script.rootBlock().statements()[2].value().leftOperand().value(), 10
        )
        self.assertEqual(
            script.rootBlock().statements()[2].value().rightOperand().value(), 20
        )

        self.assertEqual(
            script.rootBlock().statements()[3].value().binaryOperator(),
            QgsVbScriptBinaryOperatorExpression.BinaryOperator.Divide,
        )
        self.assertEqual(
            script.rootBlock().statements()[3].value().leftOperand().value(), 10
        )
        self.assertEqual(
            script.rootBlock().statements()[3].value().rightOperand().value(), 20
        )

        self.assertEqual(
            script.rootBlock().statements()[4].value().binaryOperator(),
            QgsVbScriptBinaryOperatorExpression.BinaryOperator.Modulo,
        )
        self.assertEqual(
            script.rootBlock().statements()[4].value().leftOperand().value(), 10
        )
        self.assertEqual(
            script.rootBlock().statements()[4].value().rightOperand().value(), 20
        )

    def test_binary_comparisons(self):
        script = QgsVbScript(
            """if 10 = 20 then
                10 <> 20
            end if
            10 > 20
            10 >= 20
            10 < 20
            10 <= 20
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 5)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsVbScriptIfStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].condition(),
            QgsVbScriptBinaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].condition().binaryOperator(),
            QgsVbScriptBinaryOperatorExpression.BinaryOperator.Equal,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].condition().leftOperand().value(), 10
        )
        self.assertEqual(
            script.rootBlock().statements()[0].condition().rightOperand().value(), 20
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].trueBlock().statements()[0],
            QgsVbScriptExpressionStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].trueBlock().statements()[0].expression(),
            QgsVbScriptBinaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .trueBlock()
            .statements()[0]
            .expression()
            .binaryOperator(),
            QgsVbScriptBinaryOperatorExpression.BinaryOperator.NotEqual,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .trueBlock()
            .statements()[0]
            .expression()
            .leftOperand()
            .value(),
            10,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .trueBlock()
            .statements()[0]
            .expression()
            .rightOperand()
            .value(),
            20,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[1].expression(),
            QgsVbScriptBinaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[1].expression().binaryOperator(),
            QgsVbScriptBinaryOperatorExpression.BinaryOperator.GreaterThan,
        )
        self.assertEqual(
            script.rootBlock().statements()[1].expression().leftOperand().value(), 10
        )
        self.assertEqual(
            script.rootBlock().statements()[1].expression().rightOperand().value(), 20
        )

        self.assertIsInstance(
            script.rootBlock().statements()[2].expression(),
            QgsVbScriptBinaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[2].expression().binaryOperator(),
            QgsVbScriptBinaryOperatorExpression.BinaryOperator.GreaterThanOrEqual,
        )
        self.assertEqual(
            script.rootBlock().statements()[2].expression().leftOperand().value(), 10
        )
        self.assertEqual(
            script.rootBlock().statements()[2].expression().rightOperand().value(), 20
        )

        self.assertIsInstance(
            script.rootBlock().statements()[3].expression(),
            QgsVbScriptBinaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[3].expression().binaryOperator(),
            QgsVbScriptBinaryOperatorExpression.BinaryOperator.LessThan,
        )
        self.assertEqual(
            script.rootBlock().statements()[3].expression().leftOperand().value(), 10
        )
        self.assertEqual(
            script.rootBlock().statements()[3].expression().rightOperand().value(), 20
        )

        self.assertIsInstance(
            script.rootBlock().statements()[4].expression(),
            QgsVbScriptBinaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[4].expression().binaryOperator(),
            QgsVbScriptBinaryOperatorExpression.BinaryOperator.LessThanOrEqual,
        )
        self.assertEqual(
            script.rootBlock().statements()[4].expression().leftOperand().value(), 10
        )
        self.assertEqual(
            script.rootBlock().statements()[4].expression().rightOperand().value(), 20
        )

    def test_logical_operators(self):
        script = QgsVbScript(
            """10 > 20 or y < 9
            10 > 20 and y < 9
            10 > 20 and not y < 9
            10 > 20 and not (a > b)
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 4)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsVbScriptExpressionStatement,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].expression(),
            QgsVbScriptBinaryOperatorExpression,
        )

        self.assertEqual(
            script.rootBlock().statements()[0].expression().binaryOperator(),
            QgsVbScriptBinaryOperatorExpression.BinaryOperator.LogicalOr,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].expression().leftOperand(),
            QgsVbScriptBinaryOperatorExpression,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].expression().rightOperand(),
            QgsVbScriptBinaryOperatorExpression,
        )

        self.assertEqual(
            script.rootBlock().statements()[1].expression().binaryOperator(),
            QgsVbScriptBinaryOperatorExpression.BinaryOperator.LogicalAnd,
        )

        self.assertEqual(
            script.rootBlock().statements()[2].expression().binaryOperator(),
            QgsVbScriptBinaryOperatorExpression.BinaryOperator.LogicalAnd,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[2].expression().rightOperand(),
            QgsVbScriptUnaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[2]
            .expression()
            .rightOperand()
            .unaryOperator(),
            QgsVbScriptUnaryOperatorExpression.UnaryOperator.Not,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[2]
            .expression()
            .rightOperand()
            .operand()
            .binaryOperator(),
            QgsVbScriptBinaryOperatorExpression.BinaryOperator.LessThan,
        )

        self.assertEqual(
            script.rootBlock().statements()[3].expression().binaryOperator(),
            QgsVbScriptBinaryOperatorExpression.BinaryOperator.LogicalAnd,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[3].expression().rightOperand(),
            QgsVbScriptUnaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[3]
            .expression()
            .rightOperand()
            .unaryOperator(),
            QgsVbScriptUnaryOperatorExpression.UnaryOperator.Not,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[3].expression().rightOperand().operand(),
            QgsVbScriptBinaryOperatorExpression,
        )

    def test_unary_operators(self):
        script = QgsVbScript(
            """y = +10
            z = -y
            z = +y
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 3)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsVbScriptAssignmentStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].value(),
            QgsVbScriptUnaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].value().unaryOperator(),
            QgsVbScriptUnaryOperatorExpression.UnaryOperator.Plus,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].value().operand().value(),
            10,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[1].value(),
            QgsVbScriptUnaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[1].value().unaryOperator(),
            QgsVbScriptUnaryOperatorExpression.UnaryOperator.Minus,
        )
        self.assertEqual(
            script.rootBlock().statements()[1].value().operand().name(),
            "y",
        )

        self.assertIsInstance(
            script.rootBlock().statements()[2].value(),
            QgsVbScriptUnaryOperatorExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[2].value().unaryOperator(),
            QgsVbScriptUnaryOperatorExpression.UnaryOperator.Plus,
        )
        self.assertEqual(
            script.rootBlock().statements()[2].value().operand().name(),
            "y",
        )

    def test_if(self):
        script = QgsVbScript(
            """if x > 5 then
            x = x + 5
  end if
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsVbScriptIfStatement,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].condition(),
            QgsVbScriptBinaryOperatorExpression,
        )

        self.assertIsNone(script.rootBlock().statements()[0].falseBlock())

        self.assertIsInstance(
            script.rootBlock().statements()[0].trueBlock(),
            QgsVbScriptBlockStatement,
        )

        self.assertEqual(script.rootBlock().statements()[0].trueBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0].trueBlock().statements()[0],
            QgsVbScriptAssignmentStatement,
        )

    def test_if_else(self):
        script = QgsVbScript(
            """if x > 5 then
  val = x
  else
  val = 5
end if
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsVbScriptIfStatement,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].condition(),
            QgsVbScriptBinaryOperatorExpression,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].trueBlock(),
            QgsVbScriptBlockStatement,
        )

        self.assertEqual(script.rootBlock().statements()[0].trueBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0].trueBlock().statements()[0],
            QgsVbScriptAssignmentStatement,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].falseBlock(),
            QgsVbScriptBlockStatement,
        )

        self.assertEqual(script.rootBlock().statements()[0].falseBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0].falseBlock().statements()[0],
            QgsVbScriptAssignmentStatement,
        )

        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .falseBlock()
            .statements()[0]
            .value()
            .value(),
            5,
        )

    def test_for(self):
        script = QgsVbScript(
            """for z = 0 to 100
  result = result + z
next
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsVbScriptForStatement,
        )

        self.assertEqual(script.rootBlock().statements()[0].variableName(), "z")

        self.assertIsInstance(
            script.rootBlock().statements()[0].startExpression(),
            QgsVbScriptLiteralExpression,
        )

        self.assertEqual(
            script.rootBlock().statements()[0].startExpression().value(),
            0,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].endExpression(),
            QgsVbScriptLiteralExpression,
        )

        self.assertEqual(
            script.rootBlock().statements()[0].endExpression().value(),
            100,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].body(),
            QgsVbScriptBlockStatement,
        )

        self.assertEqual(script.rootBlock().statements()[0].body().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0].body().statements()[0],
            QgsVbScriptAssignmentStatement,
        )

    def test_for_each(self):
        script = QgsVbScript(
            """for each index in myArray
  result = result + 2
next
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsVbScriptForEachStatement,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].variableName(),
            "index",
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].collection(),
            QgsVbScriptVariableReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].collection().name(),
            "myArray",
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].body(),
            QgsVbScriptBlockStatement,
        )

        self.assertEqual(script.rootBlock().statements()[0].body().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0].body().statements()[0],
            QgsVbScriptAssignmentStatement,
        )

    def test_while(self):
        script = QgsVbScript(
            """While counter > 0
  total = total + y
wend
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsVbScriptWhileStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].condition(),
            QgsVbScriptBinaryOperatorExpression,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].body(),
            QgsVbScriptBlockStatement,
        )

        self.assertEqual(script.rootBlock().statements()[0].body().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0].body().statements()[0],
            QgsVbScriptAssignmentStatement,
        )

    def test_string_constants(self):
        script = QgsVbScript("""a =  "a" + vbNewLine + "b" + vbCrLf + "d" + vbTab""")

        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsVbScriptAssignmentStatement,
        )

        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .value()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .value(),
            "a",
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .value()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .leftOperand()
            .rightOperand()
            .value(),
            "\n",
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .value()
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
            .value()
            .leftOperand()
            .leftOperand()
            .rightOperand()
            .value(),
            "\n",
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .value()
            .leftOperand()
            .rightOperand()
            .value(),
            "d",
        )
        self.assertEqual(
            script.rootBlock().statements()[0].value().rightOperand().value(),
            "\t",
        )

    def test_esri(self):
        script = QgsVbScript(
            """Function FindLabel ( [ACRES], [PARCEL_ID] )
  If IsNull([ACRES[m]]]) Or [ACRES] = 0 Then
    FindLabel = [PARCEL_ID]
  Else
    FindLabel = [PARCEL_ID] & " (" & Round([ACRES], 2) & " ac)"
  End If
End Function
"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsVbScriptFunctionDeclarationStatement,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].parameters(), ["[ACRES]", "[PARCEL_ID]"]
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].body(),
            QgsVbScriptBlockStatement,
        )

        self.assertEqual(script.rootBlock().statements()[0].body().count(), 1)
        self.assertIsInstance(
            script.rootBlock().statements()[0].body().statements()[0],
            QgsVbScriptIfStatement,
        )

        self.assertIsInstance(
            script.rootBlock().statements()[0].body().statements()[0].condition(),
            QgsVbScriptBinaryOperatorExpression,
        )

        self.assertIsInstance(
            script.rootBlock()
            .statements()[0]
            .body()
            .statements()[0]
            .condition()
            .leftOperand(),
            QgsVbScriptFunctionCallExpression,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .body()
            .statements()[0]
            .condition()
            .leftOperand()
            .targetExpression()
            .name(),
            "IsNull",
        )
        self.assertEqual(
            len(
                script.rootBlock()
                .statements()[0]
                .body()
                .statements()[0]
                .condition()
                .leftOperand()
                .arguments()
            ),
            1,
        )
        self.assertIsInstance(
            script.rootBlock()
            .statements()[0]
            .body()
            .statements()[0]
            .condition()
            .leftOperand()
            .arguments()[0],
            QgsVbScriptFieldReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .body()
            .statements()[0]
            .condition()
            .leftOperand()
            .arguments()[0]
            .fieldName(),
            "ACRES[m]",
        )

        self.assertIsInstance(
            script.rootBlock()
            .statements()[0]
            .body()
            .statements()[0]
            .condition()
            .rightOperand(),
            QgsVbScriptBinaryOperatorExpression,
        )

        self.assertIsInstance(
            script.rootBlock()
            .statements()[0]
            .body()
            .statements()[0]
            .condition()
            .rightOperand()
            .leftOperand(),
            QgsVbScriptFieldReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .body()
            .statements()[0]
            .condition()
            .rightOperand()
            .leftOperand()
            .fieldName(),
            "ACRES",
        )

        self.assertIsInstance(
            script.rootBlock()
            .statements()[0]
            .body()
            .statements()[0]
            .trueBlock()
            .statements()[0],
            QgsVbScriptAssignmentStatement,
        )
        self.assertIsInstance(
            script.rootBlock()
            .statements()[0]
            .body()
            .statements()[0]
            .trueBlock()
            .statements()[0]
            .value(),
            QgsVbScriptFieldReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock()
            .statements()[0]
            .body()
            .statements()[0]
            .trueBlock()
            .statements()[0]
            .value()
            .fieldName(),
            "PARCEL_ID",
        )

    def test_esri_field_reference_unicode(self):
        script = QgsVbScript("""[§_Schutz]""")
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsVbScriptExpressionStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].expression(),
            QgsVbScriptFieldReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().fieldName(), "§_Schutz"
        )

        script = QgsVbScript("""[Rivers, Streams.WC_LLID_NR]""")
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsVbScriptExpressionStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].expression(),
            QgsVbScriptFieldReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().fieldName(),
            "Rivers, Streams.WC_LLID_NR",
        )

        script = QgsVbScript(
            """[NAME


            ]"""
        )
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsVbScriptExpressionStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].expression(),
            QgsVbScriptFieldReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().fieldName(), """NAME"""
        )

        script = QgsVbScript("""oid]""")
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsVbScriptExpressionStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].expression(),
            QgsVbScriptFieldReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().fieldName(), "oid"
        )

        script = QgsVbScript("""oid OID]""")
        self.assertFalse(script.hasParserError())

        self.assertIsInstance(script.rootBlock(), QgsVbScriptBlockStatement)
        self.assertEqual(script.rootBlock().count(), 1)

        self.assertIsInstance(
            script.rootBlock().statements()[0],
            QgsVbScriptExpressionStatement,
        )
        self.assertIsInstance(
            script.rootBlock().statements()[0].expression(),
            QgsVbScriptFieldReferenceExpression,
        )
        self.assertEqual(
            script.rootBlock().statements()[0].expression().fieldName(), "oid OID"
        )


if __name__ == "__main__":
    unittest.main()
