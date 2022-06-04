# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsGraduatedSymbolRenderer

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Chris Crook'
__date__ = '3/10/2014'
__copyright__ = 'Copyright 2014, The QGIS Project'

import qgis  # NOQA

from qgis.testing import unittest, start_app
from qgis.core import (QgsGraduatedSymbolRenderer,
                       QgsRendererRange,
                       QgsRendererRangeLabelFormat,
                       QgsMarkerSymbol,
                       QgsGradientColorRamp,
                       QgsVectorLayer,
                       QgsFeature,
                       QgsGeometry,
                       QgsPointXY,
                       QgsReadWriteContext,
                       QgsRenderContext
                       )
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtXml import QDomDocument
from qgis.PyQt.QtGui import QColor

start_app()


# ===========================================================
# Utility functions


def createMarkerSymbol():
    symbol = QgsMarkerSymbol.createSimple({
        "color": "100,150,50",
        "name": "square",
        "size": "3.0"
    })
    return symbol


def createMemoryLayer(values):
    ml = QgsVectorLayer("Point?crs=epsg:4236&field=id:integer&field=value:double",
                        "test_data", "memory")
    # Data as list of x, y, id, value
    assert ml.isValid()
    pr = ml.dataProvider()
    fields = pr.fields()
    for id, value in enumerate(values):
        x = id * 10.0
        feat = QgsFeature(fields)
        feat['id'] = id
        feat['value'] = value
        g = QgsGeometry.fromPointXY(QgsPointXY(x, x))
        feat.setGeometry(g)
        pr.addFeatures([feat])
    ml.updateExtents()
    return ml


def createColorRamp():
    return QgsGradientColorRamp(
        QColor(255, 0, 0),
        QColor(0, 0, 255)
    )


def createLabelFormat():
    format = QgsRendererRangeLabelFormat()
    template = "%1 - %2 meters"
    precision = 5
    format.setFormat(template)
    format.setPrecision(precision)
    format.setTrimTrailingZeroes(True)
    return format


# Note: Dump functions are not designed for a user friendly dump, just
# for a moderately compact representation of a rendererer that is independent
# of the renderer source code and so appropriate for use in unit tests.


def dumpRangeBreaks(ranges):
    return dumpRangeList(ranges, breaksOnly=True)


def dumpRangeLabels(ranges):
    return dumpRangeList(ranges, labelsOnly=True)


def dumpLabelFormat(format):
    return (
        ':' + format.format() +
        ':' + str(format.precision()) +
        ':' + str(format.trimTrailingZeroes()) +
        ':')


def dumpRangeList(rlist, breaksOnly=False, labelsOnly=False):
    rstr = '('
    format = "{0:.4f}-{1:.4f}"
    if not breaksOnly:
        format = format + ":{2}:{3}:{4}:"
    if labelsOnly:
        format = '{2}'
    for r in rlist:
        rstr = rstr + format.format(
            r.lowerValue(),
            r.upperValue(),
            r.label(),
            r.symbol().dump(),
            r.renderState(),
        ) + ","
    return rstr + ')'


# Crude dump for deterministic ramp - just dumps colors at a range of values


def dumpColorRamp(ramp):
    if ramp is None:
        return ':None:'
    rampstr = ':'
    for x in (0.0, 0.33, 0.66, 1.0):
        rampstr = rampstr + ramp.color(x).name() + ':'
    return rampstr


def dumpGraduatedRenderer(r):
    rstr = ':'
    rstr = rstr + r.classAttribute() + ':'
    rstr = rstr + str(r.mode()) + ':'
    symbol = r.sourceSymbol()
    if symbol is None:
        rstr = rstr + 'None' + ':'
    else:
        rstr = rstr + symbol.dump() + ':'
    rstr = rstr + dumpColorRamp(r.sourceColorRamp())
    rstr = rstr + dumpRangeList(r.ranges())
    return rstr


# =================================================================
# Tests


class TestQgsGraduatedSymbolRenderer(unittest.TestCase):

    def testQgsRendererRange_1(self):
        """Test QgsRendererRange getter/setter functions"""
        range = QgsRendererRange()
        self.assertTrue(range)
        lower = 123.45
        upper = 234.56
        label = "Test label"
        symbol = createMarkerSymbol()
        range.setLowerValue(lower)
        self.assertEqual(range.lowerValue(), lower, "Lower value getter/setter failed")
        range.setUpperValue(upper)
        self.assertEqual(range.upperValue(), upper, "Upper value getter/setter failed")
        self.assertEqual(range[0], 123.45)
        self.assertEqual(range[1], 234.56)
        with self.assertRaises(IndexError):
            range[2]
        with self.assertRaises(IndexError):
            range[-1]
        range.setLabel(label)
        self.assertEqual(range.label(), label, "Label getter/setter failed")
        range.setRenderState(True)
        self.assertTrue(range.renderState(), "Render state getter/setter failed")
        range.setRenderState(False)
        self.assertFalse(range.renderState(), "Render state getter/setter failed")
        range.setSymbol(symbol.clone())
        self.assertEqual(symbol.dump(), range.symbol().dump(), "Symbol getter/setter failed")
        range2 = QgsRendererRange(lower, upper, symbol.clone(), label, False)
        self.assertEqual(range2.lowerValue(), lower, "Lower value from constructor failed")
        self.assertEqual(range2.upperValue(), upper, "Upper value from constructor failed")
        self.assertEqual(range2.label(), label, "Label from constructor failed")
        self.assertEqual(range2.symbol().dump(), symbol.dump(), "Symbol from constructor failed")
        self.assertFalse(range2.renderState(), "Render state getter/setter failed")

    def testQgsRendererRangeLabelFormat_1(self):
        """Test QgsRendererRangeLabelFormat getter/setter functions"""
        format = QgsRendererRangeLabelFormat()
        self.assertTrue(format, "QgsRendererRangeLabelFomat construction failed")
        template = "%1 - %2 meters"
        precision = 5
        format.setFormat(template)
        self.assertEqual(format.format(), template, "Format getter/setter failed")
        format.setPrecision(precision)
        self.assertEqual(format.precision(), precision, "Precision getter/setter failed")
        format.setTrimTrailingZeroes(True)
        self.assertTrue(format.trimTrailingZeroes(), "TrimTrailingZeroes getter/setter failed")
        format.setTrimTrailingZeroes(False)
        self.assertFalse(format.trimTrailingZeroes(), "TrimTrailingZeroes getter/setter failed")
        minprecision = -6
        maxprecision = 15
        self.assertEqual(QgsRendererRangeLabelFormat.MIN_PRECISION, minprecision, "Minimum precision != -6")
        self.assertEqual(QgsRendererRangeLabelFormat.MAX_PRECISION, maxprecision, "Maximum precision != 15")
        format.setPrecision(-10)
        self.assertEqual(format.precision(), minprecision, "Minimum precision not enforced")
        format.setPrecision(20)
        self.assertEqual(format.precision(), maxprecision, "Maximum precision not enforced")

    def testQgsRendererRangeLabelFormat_2(self):
        """Test QgsRendererRangeLabelFormat number format"""
        format = QgsRendererRangeLabelFormat()
        # Tests have precision, trim, value, expected
        # (Note: Not sure what impact of locale is on these tests)
        tests = (
            (2, False, 1.0, '1.00'),
            (2, True, 1.0, '1'),
            (2, False, 1.234, '1.23'),
            (2, True, 1.234, '1.23'),
            (2, False, 1.236, '1.24'),
            (2, False, -1.236, '-1.24'),
            (2, False, -0.004, '0.00'),
            (2, True, 1.002, '1'),
            (2, True, 1.006, '1.01'),
            (2, True, 1.096, '1.1'),
            (3, True, 1.096, '1.096'),
            (-2, True, 1496.45, '1500'),
            (-2, True, 149.45, '100'),
            (-2, True, 79.45, '100'),
            (-2, True, 49.45, '0'),
            (-2, True, -49.45, '0'),
            (-2, True, -149.45, '-100'),
        )
        for f in tests:
            precision, trim, value, expected = f
            format.setPrecision(precision)
            format.setTrimTrailingZeroes(trim)
            result = format.formatNumber(value)
            self.assertEqual(result, expected,
                             "Number format error {0}:{1}:{2} => {3}".format(
                                 precision, trim, value, result))

        # Label tests - label format, expected result.
        # Labels will be evaluated with lower=1.23 upper=2.34, precision=2
        ltests = (
            ("%1 - %2", "1.23 - 2.34"),
            ("%1", "1.23"),
            ("%2", "2.34"),
            ("%2%", "2.34%"),
            ("%1%1", "1.231.23"),
            ("from %1 to %2 meters", "from 1.23 to 2.34 meters"),
            ("from %2 to %1 meters", "from 2.34 to 1.23 meters"),
        )
        format.setPrecision(2)
        format.setTrimTrailingZeroes(False)
        lower = 1.232
        upper = 2.339
        for t in ltests:
            label, expected = t
            format.setFormat(label)
            result = format.labelForLowerUpper(lower, upper)
            self.assertEqual(result, expected, "Label format error {0} => {1}".format(
                label, result))

        range = QgsRendererRange()
        range.setLowerValue(lower)
        range.setUpperValue(upper)
        label = ltests[0][0]
        format.setFormat(label)
        result = format.labelForRange(range)
        self.assertEqual(result, ltests[0][1], "Label for range error {0} => {1}".format(
            label, result))

    def testQgsGraduatedSymbolRenderer_1(self):
        """Test QgsGraduatedSymbolRenderer: Basic get/set functions """

        # Create a renderer
        renderer = QgsGraduatedSymbolRenderer()

        symbol = createMarkerSymbol()
        renderer.setSourceSymbol(symbol.clone())
        self.assertEqual(symbol.dump(), renderer.sourceSymbol().dump(), "Get/set renderer source symbol")

        attr = '"value"*"value"'
        renderer.setClassAttribute(attr)
        self.assertEqual(attr, renderer.classAttribute(), "Get/set renderer class attribute")

        for m in (
            QgsGraduatedSymbolRenderer.Custom,
            QgsGraduatedSymbolRenderer.EqualInterval,
            QgsGraduatedSymbolRenderer.Quantile,
            QgsGraduatedSymbolRenderer.Jenks,
            QgsGraduatedSymbolRenderer.Pretty,
            QgsGraduatedSymbolRenderer.StdDev,
        ):
            renderer.setMode(m)
            self.assertEqual(m, renderer.mode(), "Get/set renderer mode")

        format = createLabelFormat()
        renderer.setLabelFormat(format)
        self.assertEqual(
            dumpLabelFormat(format),
            dumpLabelFormat(renderer.labelFormat()),
            "Get/set renderer label format")

        ramp = createColorRamp()
        renderer.setSourceColorRamp(ramp)
        self.assertEqual(
            dumpColorRamp(ramp),
            dumpColorRamp(renderer.sourceColorRamp()),
            "Get/set renderer color ramp")

        renderer.setSourceColorRamp(ramp)
        self.assertEqual(
            dumpColorRamp(ramp),
            dumpColorRamp(renderer.sourceColorRamp()),
            "Get/set renderer color ramp")

        # test for classificatio with varying size
        renderer.setGraduatedMethod(QgsGraduatedSymbolRenderer.GraduatedSize)
        renderer.setSourceColorRamp(None)
        renderer.addClassLowerUpper(0, 2)
        renderer.addClassLowerUpper(2, 4)
        renderer.addClassLowerUpper(4, 6)
        renderer.setSymbolSizes(2, 13)
        self.assertEqual(renderer.maxSymbolSize(), 13)
        self.assertEqual(renderer.minSymbolSize(), 2)
        refSizes = [2, (13 + 2) * .5, 13]
        ctx = QgsRenderContext()
        for idx, symbol in enumerate(renderer.symbols(ctx)):
            self.assertEqual(symbol.size(), refSizes[idx])

    def testQgsGraduatedSymbolRenderer_2(self):
        """Test QgsGraduatedSymbolRenderer: Adding /removing/editing classes """
        # Create a renderer
        renderer = QgsGraduatedSymbolRenderer()
        symbol = createMarkerSymbol()
        renderer.setSourceSymbol(symbol.clone())
        symbol.setColor(QColor(255, 0, 0))

        # Add class without start and end ranges

        renderer.addClass(symbol.clone())
        renderer.addClass(symbol.clone())
        renderer.updateRangeLabel(1, 'Second range')
        renderer.updateRangeLowerValue(1, 10.0)
        renderer.updateRangeUpperValue(1, 25.0)
        renderer.updateRangeRenderState(1, False)
        symbol.setColor(QColor(0, 0, 255))
        renderer.updateRangeSymbol(1, symbol.clone())

        # Add as a rangeobject
        symbol.setColor(QColor(0, 255, 0))
        range = QgsRendererRange(20.0, 25.5, symbol.clone(), 'Third range', False)
        renderer.addClassRange(range)

        # Add class by lower and upper
        renderer.addClassLowerUpper(25.5, 30.5)
        # (Update label for sorting tests)
        renderer.updateRangeLabel(3, 'Another range')

        self.assertEqual(
            dumpRangeLabels(renderer.ranges()),
            '(0.0 - 0.0,Second range,Third range,Another range,)',
            'Added ranges labels not correct')
        self.assertEqual(
            dumpRangeBreaks(renderer.ranges()),
            '(0.0000-0.0000,10.0000-25.0000,20.0000-25.5000,25.5000-30.5000,)',
            'Added ranges lower/upper values not correct')

        # Check that clone function works

        renderer2 = renderer.clone()
        self.assertEqual(
            dumpGraduatedRenderer(renderer),
            dumpGraduatedRenderer(renderer2),
            "clone function doesn't replicate renderer properly"
        )

        # Check save and reload from Dom works

        doc = QDomDocument()
        element = renderer.save(doc, QgsReadWriteContext())
        renderer2 = QgsGraduatedSymbolRenderer.create(element, QgsReadWriteContext())
        self.assertEqual(
            dumpGraduatedRenderer(renderer),
            dumpGraduatedRenderer(renderer2),
            "Save/create from DOM doesn't replicate renderer properly"
        )

        # Check sorting

        renderer.sortByLabel()
        self.assertEqual(
            dumpRangeList(renderer.ranges(), labelsOnly=True),
            '(0.0 - 0.0,Another range,Second range,Third range,)',
            'sortByLabel not correct')
        renderer.sortByValue()
        self.assertEqual(
            dumpRangeBreaks(renderer.ranges()),
            '(0.0000-0.0000,10.0000-25.0000,20.0000-25.5000,25.5000-30.5000,)',
            'sortByValue not correct')
        renderer.sortByValue(Qt.DescendingOrder)
        self.assertEqual(
            dumpRangeBreaks(renderer.ranges()),
            '(25.5000-30.5000,20.0000-25.5000,10.0000-25.0000,0.0000-0.0000,)',
            'sortByValue descending not correct')

        # Check deleting

        renderer.deleteClass(2)
        self.assertEqual(
            dumpRangeBreaks(renderer.ranges()),
            '(25.5000-30.5000,20.0000-25.5000,0.0000-0.0000,)',
            'deleteClass not correct')

        renderer.deleteAllClasses()
        self.assertEqual(len(renderer.ranges()), 0, "deleteAllClasses didn't delete all")

    #    void addClass( QgsSymbol* symbol );
    #    //! \note available in python bindings as addClassRange
    #    void addClass( QgsRendererRange range ) /PyName=addClassRange/;
    #    //! \note available in python bindings as addClassLowerUpper
    #    void addClass( double lower, double upper ) /PyName=addClassLowerUpper/;
    #    void deleteClass( int idx );
    #    void deleteAllClasses();

    def testQgsGraduatedSymbolRenderer_3(self):
        """Test QgsGraduatedSymbolRenderer: Reading attribute data, calculating classes """

        # Create a renderer
        renderer = QgsGraduatedSymbolRenderer()
        symbol = createMarkerSymbol()
        renderer.setSourceSymbol(symbol.clone())

        # Test retrieving data values from a layer
        ml = createMemoryLayer((1.2, 0.5, 5.0, 1.0, 1.0, 1.2))

        renderer.setClassAttribute("value")
        # Equal interval calculations
        renderer.updateClasses(ml, renderer.EqualInterval, 3)
        self.assertEqual(
            dumpRangeBreaks(renderer.ranges()),
            '(0.5000-2.0000,2.0000-3.5000,3.5000-5.0000,)',
            'Equal interval classification not correct')

        # Quantile classes
        renderer.updateClasses(ml, renderer.Quantile, 3)
        self.assertEqual(
            dumpRangeBreaks(renderer.ranges()),
            '(0.5000-1.0000,1.0000-1.2000,1.2000-5.0000,)',
            'Quantile classification not correct')
        renderer.updateClasses(ml, renderer.Quantile, 4)
        self.assertEqual(
            dumpRangeBreaks(renderer.ranges()),
            '(0.5000-1.0000,1.0000-1.1000,1.1000-1.2000,1.2000-5.0000,)',
            'Quantile classification not correct')

    def testUsedAttributes(self):
        renderer = QgsGraduatedSymbolRenderer()
        ctx = QgsRenderContext()

        # attribute can contain either attribute name or an expression.
        # Sometimes it is not possible to distinguish between those two,
        # e.g. "a - b" can be both a valid attribute name or expression.
        # Since we do not have access to fields here, the method should return both options.
        renderer.setClassAttribute("value")
        self.assertEqual(renderer.usedAttributes(ctx), {"value"})
        renderer.setClassAttribute("value - 1")
        self.assertEqual(renderer.usedAttributes(ctx), {"value", "value - 1"})
        renderer.setClassAttribute("valuea - valueb")
        self.assertEqual(renderer.usedAttributes(ctx), {"valuea", "valuea - valueb", "valueb"})

    def testFilterNeedsGeometry(self):
        renderer = QgsGraduatedSymbolRenderer()

        renderer.setClassAttribute("value")
        self.assertFalse(renderer.filterNeedsGeometry())
        renderer.setClassAttribute("$area")
        self.assertTrue(renderer.filterNeedsGeometry())
        renderer.setClassAttribute("value - $area")
        self.assertTrue(renderer.filterNeedsGeometry())

    def test_legend_key_to_expression(self):
        renderer = QgsGraduatedSymbolRenderer()
        renderer.setClassAttribute('field_name')

        exp, ok = renderer.legendKeyToExpression('xxxx', None)
        self.assertFalse(ok)

        # no categories
        exp, ok = renderer.legendKeyToExpression('0', None)
        self.assertFalse(ok)

        symbol_a = createMarkerSymbol()
        renderer.addClassRange(QgsRendererRange(1, 2, symbol_a, 'a'))
        symbol_b = createMarkerSymbol()
        renderer.addClassRange(QgsRendererRange(5, 6, symbol_b, 'b'))
        symbol_c = createMarkerSymbol()
        renderer.addClassRange(QgsRendererRange(15.5, 16.5, symbol_c, 'c', False))

        exp, ok = renderer.legendKeyToExpression('0', None)
        self.assertTrue(ok)
        self.assertEqual(exp, "(field_name >= 1) AND (field_name <= 2)")

        exp, ok = renderer.legendKeyToExpression('1', None)
        self.assertTrue(ok)
        self.assertEqual(exp, "(field_name >= 5) AND (field_name <= 6)")

        exp, ok = renderer.legendKeyToExpression('2', None)
        self.assertTrue(ok)
        self.assertEqual(exp, "(field_name >= 15.5) AND (field_name <= 16.5)")

        exp, ok = renderer.legendKeyToExpression('3', None)
        self.assertFalse(ok)

        layer = QgsVectorLayer("Point?field=field_name:double&field=fldint:integer", "addfeat", "memory")
        # with layer
        exp, ok = renderer.legendKeyToExpression('2', layer)
        self.assertTrue(ok)
        self.assertEqual(exp, """("field_name" >= 15.5) AND ("field_name" <= 16.5)""")

        # with expression as attribute
        renderer.setClassAttribute('log("field_name")')

        exp, ok = renderer.legendKeyToExpression('0', None)
        self.assertTrue(ok)
        self.assertEqual(exp, """(log("field_name") >= 1) AND (log("field_name") <= 2)""")

        exp, ok = renderer.legendKeyToExpression('1', None)
        self.assertTrue(ok)
        self.assertEqual(exp, """(log("field_name") >= 5) AND (log("field_name") <= 6)""")

        exp, ok = renderer.legendKeyToExpression('2', None)
        self.assertTrue(ok)
        self.assertEqual(exp, """(log("field_name") >= 15.5) AND (log("field_name") <= 16.5)""")

        exp, ok = renderer.legendKeyToExpression('2', layer)
        self.assertTrue(ok)
        self.assertEqual(exp, """(log("field_name") >= 15.5) AND (log("field_name") <= 16.5)""")


if __name__ == "__main__":
    unittest.main()
