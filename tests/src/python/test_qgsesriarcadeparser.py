"""QGIS Unit tests for QgsEsriArcadeScript.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

import sys

from qgis.core import (
    QgsEsriArcadeArrayLiteralExpression,
    QgsEsriArcadeAssignmentStatement,
    QgsEsriArcadeBinaryOperatorExpression,
    QgsEsriArcadeBlockStatement,
    QgsEsriArcadeDictionaryElement,
    QgsEsriArcadeDictionaryLiteralExpression,
    QgsEsriArcadeExpressionStatement,
    QgsEsriArcadeFunctionDeclarationStatement,
    QgsEsriArcadeIndexExpression,
    QgsEsriArcadeLiteralExpression,
    QgsEsriArcadeMemberAccessExpression,
    QgsEsriArcadeProfileVariableReferenceExpression,
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


if __name__ == "__main__":
    unittest.main()
