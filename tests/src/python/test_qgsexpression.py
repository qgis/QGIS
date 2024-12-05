"""QGIS Unit tests for QgsExpression.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "Nathan Woodrow"
__date__ = "4/11/2012"
__copyright__ = "Copyright 2012, The QGIS Project"

from qgis.PyQt.QtCore import QVariant
from qgis.core import (
    NULL,
    QgsCoordinateReferenceSystem,
    QgsExpression,
    QgsExpressionContext,
    QgsExpressionContextUtils,
    QgsFeatureRequest,
    QgsFields,
    QgsFeature,
    QgsField,
    QgsVectorLayer,
)
from qgis.testing import unittest
from qgis.utils import qgsfunction


class TestQgsExpressionCustomFunctions(unittest.TestCase):

    @qgsfunction(1, "testing", register=False)
    def testfun(values, feature, parent):
        """Function help"""
        return f"Testing_{values[0]}"

    @qgsfunction(args="auto", group="testing", register=False)
    def expandargs(value1, value2, value3, feature, parent):
        return value1, value2, value3

    @qgsfunction(args=0, group="testing", register=False)
    def special(values, feature, parent):
        return "test"

    @qgsfunction(1, "testing", register=False)
    def sqrt(values, feature, parent):
        pass

    @qgsfunction(1, "testing", register=False)
    def help_with_docstring(values, feature, parent):
        """The help comes from the python docstring."""
        pass

    help_text = "The help comes from a variable."

    @qgsfunction(1, "testing", register=False, helpText=help_text)
    def help_with_variable(values, feature, parent):
        """This docstring is not used for the help."""
        pass

    @qgsfunction(1, "testing", register=False, usesgeometry=True)
    def geomtest(values, feature, parent):
        pass

    @qgsfunction(args=0, group="testing", register=False)
    def no_referenced_columns_set(values, feature, parent):
        return 1

    @qgsfunction(args=0, group="testing", register=False, referenced_columns=["a", "b"])
    def referenced_columns_set(values, feature, parent):
        return 2

    @qgsfunction(args=-1, group="testing", register=False, handlesnull=True)
    def null_mean(values, feature, parent):
        vals = [val for val in values if val != NULL]
        return sum(vals) / len(vals)

    @qgsfunction(group="testing", register=False)
    def raise_exception(feature, parent):
        # an undefined variable
        foo  # noqa: F821

    @qgsfunction(group="testing", register=False)
    def simple_sum(val1, val2):
        return val1 + val2

    @qgsfunction(group="testing", register=False)
    def sum_vargs(val1, *args):
        return val1 + sum(args)

    @qgsfunction(group="testing", args=-1, register=False)
    def func_params_as_list_legacy(values, feature, parent):
        return values

    @qgsfunction(group="testing", params_as_list=True, register=False)
    def func_params_as_list(values):
        return values

    @qgsfunction(group="testing", register=False)
    def func_params_no_list(values):
        return values

    @qgsfunction(group="testing", register=False)
    def func_feature(*args, feature):
        return (feature["a"] + sum(args)) * feature["b"]

    @qgsfunction(group="testing", register=False)
    def func_layer_name_operation(operation="upper", context=None):
        return getattr(context.variable("layer_name"), operation)()

    def tearDown(self):
        QgsExpression.unregisterFunction("testfun")

    def testCanBeRegistered(self):
        QgsExpression.registerFunction(self.testfun)
        index = QgsExpression.functionIndex("testfun")
        self.assertNotEqual(index, -1)

    def testHelpPythonFunction(self):
        """Test help about python function."""
        QgsExpression.registerFunction(self.help_with_variable)
        html = (
            "<h3>help_with_variable function</h3><br>" "The help comes from a variable."
        )
        self.assertEqual(self.help_with_variable.helpText(), html)

        QgsExpression.registerFunction(self.help_with_docstring)
        html = (
            "<h3>help_with_docstring function</h3><br>"
            "The help comes from the python docstring."
        )
        self.assertEqual(self.help_with_docstring.helpText(), html)

    def testHelpString(self):
        """Test add custom help string."""
        self.assertTrue(
            QgsExpression.addVariableHelpText("custom_variable_help", "custom help 1")
        )
        self.assertFalse(
            QgsExpression.addVariableHelpText("custom_variable_help", "other help 2")
        )
        self.assertEqual(
            QgsExpression.variableHelpText("custom_variable_help"), "custom help 1"
        )

    def testAutoArgsAreExpanded(self):
        function = self.expandargs
        values = [1, 2, 3]
        exp = QgsExpression("")
        result = function.func(values, None, exp, None)
        # Make sure there is no eval error
        self.assertEqual(exp.evalErrorString(), "")
        self.assertEqual(result, (1, 2, 3))

    def testCanUnregisterFunction(self):
        QgsExpression.registerFunction(self.testfun)
        index = QgsExpression.functionIndex("testfun")
        self.assertNotEqual(index, -1)
        error = QgsExpression.unregisterFunction("testfun")
        self.assertTrue(error)
        index = QgsExpression.functionIndex("testfun")
        self.assertEqual(index, -1)

    def testCanEvaluateFunction(self):
        QgsExpression.registerFunction(self.testfun)
        exp = QgsExpression("testfun(1)")
        result = exp.evaluate()
        self.assertEqual("Testing_1", result)

    def testZeroArgFunctionsTakeNoArgs(self):
        QgsExpression.registerFunction(self.special)
        special = self.special
        self.assertEqual(special.name(), "special")
        exp = QgsExpression("special()")
        result = exp.evaluate()
        self.assertEqual("test", result)

    def testDecoratorPreservesAttributes(self):
        func = self.testfun
        self.assertEqual(func.name(), "testfun")
        self.assertEqual(func.group(), "testing")

    def testCantReregister(self):
        QgsExpression.registerFunction(self.testfun)
        success = QgsExpression.registerFunction(self.testfun)
        self.assertFalse(success)

    def testCanReregisterAfterUnregister(self):
        QgsExpression.registerFunction(self.testfun)
        QgsExpression.unregisterFunction("testfun")
        success = QgsExpression.registerFunction(self.testfun)
        self.assertTrue(success)

    def testCantOverrideBuiltinsWithRegister(self):
        success = QgsExpression.registerFunction(self.sqrt)
        self.assertFalse(success)

    def testCanRegisterGeometryFunction(self):
        success = QgsExpression.registerFunction(self.geomtest)
        self.assertTrue(success)

    def testReferencedColumnsNoSet(self):
        QgsExpression.registerFunction(self.no_referenced_columns_set)
        exp = QgsExpression("no_referenced_columns_set()")
        self.assertEqual(exp.referencedColumns(), {QgsFeatureRequest.ALL_ATTRIBUTES})

    def testReferencedColumnsSet(self):
        QgsExpression.registerFunction(self.referenced_columns_set)
        exp = QgsExpression("referenced_columns_set()")
        self.assertEqual(set(exp.referencedColumns()), {"a", "b"})

    def testHandlesNull(self):
        context = QgsExpressionContext()
        QgsExpression.registerFunction(self.null_mean)
        exp = QgsExpression("null_mean(1, 2, NULL, 3)")
        result = exp.evaluate(context)
        self.assertFalse(exp.hasEvalError())
        self.assertEqual(result, 2)

    def testCantOverrideBuiltinsWithUnregister(self):
        success = QgsExpression.unregisterFunction("sqrt")
        self.assertFalse(success)

    def testDump(self):
        for txt in [
            "id",
            "idä",
            '"id abc"',
            '"id	abc"',
            "  abc   ",
            " /* co */ da ",
        ]:
            self.assertEqual(txt, QgsExpression(txt).expression())

    def testBlockComment(self):
        expressions = {
            "'test' /* comment */": "test",
            "/* comment */'test'": "test",
            "/* comment */'test*/'": "test*/",
            "/** comment */'test*/'": "test*/",
            "/* comment **/'test*/' /* comment */": "test*/",
            "'test/*'/* comment */": "test/*",
            """/**
            comment
            **/
            'test*/'""": "test*/",
            """'test*/'
            /**
            comment
            **/""": "test*/",
        }
        for e, exp_res in list(expressions.items()):
            exp = QgsExpression(e)
            result = exp.evaluate()
            self.assertEqual(exp_res, result)

    def testComment(self):
        expressions = {
            "'test' -- comment\n": "test",
            "'test--'\n": "test--",
            "'--test'\n": "--test",
            "'test' -- comment": "test",
            "'test--'": "test--",
            "'--test'": "--test",
        }
        for e, exp_res in list(expressions.items()):
            exp = QgsExpression(e)
            result = exp.evaluate()
            self.assertEqual(exp_res, result)

    def testValid(self):
        e = QgsExpression()
        self.assertFalse(e.isValid())
        e.setExpression("asdf||#@¼")
        self.assertFalse(e.isValid())
        e.setExpression("1")
        self.assertTrue(e.isValid())

    def testCreateFieldEqualityExpression(self):
        e = QgsExpression()

        # test when value is null
        field = "myfield"
        value = NULL
        res = '"myfield" IS NULL'
        self.assertEqual(e.createFieldEqualityExpression(field, value), res)

        # test when value is null and field name has a quote
        field = "my'field"
        value = NULL
        res = '"my\'field" IS NULL'
        self.assertEqual(e.createFieldEqualityExpression(field, value), res)

        # test when field name has a quote and value is an int
        field = "my'field"
        value = 5
        res = '"my\'field" = 5'
        self.assertEqual(e.createFieldEqualityExpression(field, value), res)

        # test when field name has a quote and value is a string
        field = "my'field"
        value = "5"
        res = "\"my'field\" = '5'"
        self.assertEqual(e.createFieldEqualityExpression(field, value), res)

        # test when field name has a quote and value is a boolean
        field = "my'field"
        value = True
        res = '"my\'field" = TRUE'
        self.assertEqual(e.createFieldEqualityExpression(field, value), res)

        # test with field type
        field = "myfield"
        value = 1
        type = QVariant.String
        res = "\"myfield\" = '1'"
        self.assertEqual(e.createFieldEqualityExpression(field, value, type), res)

        # test with field type
        field = "myfield"
        value = "1"
        type = QVariant.Int
        res = '"myfield" = 1'
        self.assertEqual(e.createFieldEqualityExpression(field, value, type), res)

    def testReferencedAttributeIndexesNonExistingField(self):
        e = QgsExpression()
        e.setExpression("foo = 1")
        self.assertTrue(e.isValid())
        self.assertEqual(len(e.referencedAttributeIndexes(QgsFields())), 0)

    def testSuccessfulEvaluationReturnsNoEvalErrorString(self):
        exp = QgsExpression("True is False")  # the result does not matter
        self.assertEqual(exp.evalErrorString(), "")

    def testEvalTemplate(self):
        layer = QgsVectorLayer(
            "Point?field=a:int&field=b:string", "test eval-template", "memory"
        )
        context = layer.createExpressionContext()

        expression = QgsExpression("eval_template('123 [% \"b\" %] 789')")
        expression.prepare(context)
        columns = expression.referencedColumns()

        # Insure prepare has returned all attributes as referenced columns with feature-less context
        self.assertTrue(QgsFeatureRequest.ALL_ATTRIBUTES in columns)

        feature = QgsFeature(layer.fields())
        feature.setAttributes([1, "456"])
        context.setFeature(feature)

        self.assertEqual(expression.evaluate(context), "123 456 789")

    def testExceptionDuringEvalReturnsTraceback(self):
        QgsExpression.registerFunction(self.raise_exception)
        exp = QgsExpression("raise_exception()")
        result = exp.evaluate()
        # The file paths and line offsets are dynamic
        regex = (
            "name 'foo' is not defined:<pre>Traceback \\(most recent call last\\):\n"
            '  File ".*qgsfunction.py", line [0-9]+, in func\n'
            "    return self.function\\(\\*values, \\*\\*kwvalues\\)\n"
            "(.*?\n)?"
            '  File ".*test_qgsexpression.py", line [0-9]+, in raise_exception\n'
            "    foo  # noqa: F821\n"
            "(.*?\n)?"
            "NameError: name 'foo' is not defined"
            "\n</pre>"
        )
        self.assertRegex(exp.evalErrorString(), regex)

    def testBetween(self):

        e = QgsExpression()
        e.setExpression("'b'")
        self.assertTrue(e.isValid(), e.parserErrorString())
        e.setExpression("'b' between 'a' AND 'c'")
        self.assertTrue(e.isValid(), e.parserErrorString())

    def testSimpleSum(self):

        QgsExpression.registerFunction(self.simple_sum)

        e = QgsExpression()
        e.setExpression("simple_sum(4, 5)")
        self.assertEqual(e.evaluate(), 9)

        # Not enough parameters
        e.setExpression("simple_sum(4)")
        self.assertEqual(e.evaluate(), None)

        # Too many parameters
        e.setExpression("simple_sum(4, 7, 8)")
        self.assertEqual(e.evaluate(), None)

    def testSumVargs(self):

        QgsExpression.registerFunction(self.sum_vargs)

        e = QgsExpression()

        # Not enough parameters
        e.setExpression("sum_vargs()")
        self.assertEqual(e.evaluate(), None)

        e.setExpression("sum_vargs(4)")
        self.assertEqual(e.evaluate(), 4)

        e.setExpression("sum_vargs(4, 5)")
        self.assertEqual(e.evaluate(), 9)

        e.setExpression("sum_vargs(4, 5, 6, 7)")
        self.assertEqual(e.evaluate(), 22)

    def testListNoList(self):
        QgsExpression.registerFunction(self.func_params_as_list_legacy)
        QgsExpression.registerFunction(self.func_params_as_list)
        QgsExpression.registerFunction(self.func_params_no_list)

        e = QgsExpression()
        e.setExpression("func_params_as_list_legacy('a', 'b', 'c')")
        self.assertEqual(e.evaluate(), ["a", "b", "c"])

        e.setExpression("func_params_as_list('a', 'b', 'c')")
        self.assertEqual(e.evaluate(), ["a", "b", "c"])

        e.setExpression("func_params_as_list('a')")
        self.assertEqual(e.evaluate(), ["a"])

        e.setExpression("func_params_no_list('a')")
        self.assertEqual(e.evaluate(), "a")

    def testFeature(self):
        QgsExpression.registerFunction(self.func_feature)
        context = QgsExpressionContext()
        feature = QgsFeature(id=10)
        fields = QgsFields()
        fields.append(QgsField("a", QVariant.Int))
        fields.append(QgsField("b", QVariant.String))

        feature.setFields(fields)
        feature["a"] = 4
        feature["b"] = "world"

        context.setFeature(feature)

        e = QgsExpression()
        e.setExpression("func_feature()")
        self.assertEqual(e.evaluate(context), "worldworldworldworld")
        e.setExpression("func_feature(-1, -1)")
        self.assertEqual(e.evaluate(context), "worldworld")

        feature["a"] = 2
        feature["b"] = "_"

        context.setFeature(feature)
        e.setExpression("func_feature()")
        self.assertEqual(e.evaluate(context), "__")
        e.setExpression("func_feature(3)")
        self.assertEqual(e.evaluate(context), "_____")

    def testContext(self):
        QgsExpression.registerFunction(self.func_layer_name_operation)
        context = QgsExpressionContext()

        layer = QgsVectorLayer(
            "Point?field=a:int&field=b:string", "test context", "memory"
        )
        context.appendScope(QgsExpressionContextUtils.layerScope(layer))

        e = QgsExpression()
        e.setExpression("func_layer_name_operation()")
        self.assertEqual(e.evaluate(context), "TEST CONTEXT")
        e.setExpression("func_layer_name_operation('title')")
        self.assertEqual(e.evaluate(context), "Test Context")
        e.setExpression("func_layer_name_operation('split')")
        self.assertEqual(e.evaluate(context), ["test", "context"])

    def testLayerScopeVerticalCrs(self):
        layer = QgsVectorLayer("Point?field=fldtxt:string", "layer", "memory")

        # vertical crs info should be present in layer expression context scope
        layer.setVerticalCrs(QgsCoordinateReferenceSystem("EPSG:5703"))
        scope = QgsExpressionContextUtils.layerScope(layer)
        self.assertEqual(scope.variable("layer_vertical_crs"), "EPSG:5703")
        self.assertEqual(
            scope.variable("layer_vertical_crs_definition"), "+vunits=m +no_defs"
        )
        self.assertEqual(
            scope.variable("layer_vertical_crs_description"), "NAVD88 height"
        )
        self.assertEqual(scope.variable("layer_vertical_crs_wkt")[:7], "VERTCRS")

        layer.setVerticalCrs(QgsCoordinateReferenceSystem())
        scope = QgsExpressionContextUtils.layerScope(layer)
        self.assertFalse(scope.variable("layer_vertical_crs"))
        self.assertFalse(scope.variable("layer_vertical_crs_definition"))
        self.assertFalse(scope.variable("layer_vertical_crs_description"))
        self.assertFalse(scope.variable("layer_vertical_crs_wkt"))


if __name__ == "__main__":
    unittest.main()
