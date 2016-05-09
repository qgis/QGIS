# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsGraduatedSymbolRendererV2

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Chris Crook'
__date__ = '3/10/2014'
__copyright__ = 'Copyright 2014, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.testing import unittest, start_app
from qgis.core import (QgsGraduatedSymbolRendererV2,
                       QgsRendererRangeV2,
                       QgsRendererRangeV2LabelFormat,
                       QgsMarkerSymbolV2,
                       QgsVectorGradientColorRampV2,
                       QgsVectorLayer,
                       QgsFeature,
                       QgsGeometry,
                       QgsPoint,
                       QgsSymbolV2,
                       QgsSymbolLayerV2Utils,
                       QgsRenderContext
                       )
from qgis.PyQt.QtCore import Qt
from qgis.PyQt.QtXml import QDomDocument
from qgis.PyQt.QtGui import QColor

start_app()

#===========================================================
# Utility functions


def createMarkerSymbol():
    symbol = QgsMarkerSymbolV2.createSimple({
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
        g = QgsGeometry.fromPoint(QgsPoint(x, x))
        feat.setGeometry(g)
        pr.addFeatures([feat])
    ml.updateExtents()
    return ml


def createColorRamp():
    return QgsVectorGradientColorRampV2(
        QColor(255, 0, 0),
        QColor(0, 0, 255)
    )


def createLabelFormat():
    format = QgsRendererRangeV2LabelFormat()
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
    rstr = rstr + str(r.invertedColorRamp()) + ':'
    rstr = rstr + dumpRangeList(r.ranges())
    rstr = rstr + r.rotationField() + ':'
    rstr = rstr + r.sizeScaleField() + ':'
    rstr = rstr + str(r.scaleMethod()) + ':'
    return rstr

#=================================================================
# Tests


class TestQgsGraduatedSymbolRendererV2(unittest.TestCase):

    def testQgsRendererRangeV2_1(self):
        """Test QgsRendererRangeV2 getter/setter functions"""
        range = QgsRendererRangeV2()
        self.assertTrue(range)
        lower = 123.45
        upper = 234.56
        label = "Test label"
        symbol = createMarkerSymbol()
        range.setLowerValue(lower)
        self.assertEqual(range.lowerValue(), lower, "Lower value getter/setter failed")
        range.setUpperValue(upper)
        self.assertEqual(range.upperValue(), upper, "Upper value getter/setter failed")
        range.setLabel(label)
        self.assertEqual(range.label(), label, "Label getter/setter failed")
        range.setRenderState(True)
        self.assertTrue(range.renderState(), "Render state getter/setter failed")
        range.setRenderState(False)
        self.assertFalse(range.renderState(), "Render state getter/setter failed")
        range.setSymbol(symbol.clone())
        self.assertEqual(symbol.dump(), range.symbol().dump(), "Symbol getter/setter failed")
        range2 = QgsRendererRangeV2(lower, upper, symbol.clone(), label, False)
        self.assertEqual(range2.lowerValue(), lower, "Lower value from constructor failed")
        self.assertEqual(range2.upperValue(), upper, "Upper value from constructor failed")
        self.assertEqual(range2.label(), label, "Label from constructor failed")
        self.assertEqual(range2.symbol().dump(), symbol.dump(), "Symbol from constructor failed")
        self.assertFalse(range2.renderState(), "Render state getter/setter failed")

    def testQgsRendererRangeV2LabelFormat_1(self):
        """Test QgsRendererRangeV2LabelFormat getter/setter functions"""
        format = QgsRendererRangeV2LabelFormat()
        self.assertTrue(format, "QgsRendererRangeV2LabelFomat construction failed")
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
        self.assertEqual(QgsRendererRangeV2LabelFormat.MinPrecision, minprecision, "Minimum precision != -6")
        self.assertEqual(QgsRendererRangeV2LabelFormat.MaxPrecision, maxprecision, "Maximum precision != 15")
        format.setPrecision(-10)
        self.assertEqual(format.precision(), minprecision, "Minimum precision not enforced")
        format.setPrecision(20)
        self.assertEqual(format.precision(), maxprecision, "Maximum precision not enforced")

    def testQgsRendererRangeV2LabelFormat_2(self):
        """Test QgsRendererRangeV2LabelFormat number format"""
        format = QgsRendererRangeV2LabelFormat()
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

        range = QgsRendererRangeV2()
        range.setLowerValue(lower)
        range.setUpperValue(upper)
        label = ltests[0][0]
        format.setFormat(label)
        result = format.labelForRange(range)
        self.assertEqual(result, ltests[0][1], "Label for range error {0} => {1}".format(
                         label, result))

    def testQgsGraduatedSymbolRendererV2_1(self):
        """Test QgsGraduatedSymbolRendererV2: Basic get/set functions """

        # Create a renderer
        renderer = QgsGraduatedSymbolRendererV2()

        symbol = createMarkerSymbol()
        renderer.setSourceSymbol(symbol.clone())
        self.assertEqual(symbol.dump(), renderer.sourceSymbol().dump(), "Get/set renderer source symbol")

        attr = '"value"*"value"'
        renderer.setClassAttribute(attr)
        self.assertEqual(attr, renderer.classAttribute(), "Get/set renderer class attribute")

        for m in (
            QgsGraduatedSymbolRendererV2.Custom,
            QgsGraduatedSymbolRendererV2.EqualInterval,
            QgsGraduatedSymbolRendererV2.Quantile,
            QgsGraduatedSymbolRendererV2.Jenks,
            QgsGraduatedSymbolRendererV2.Pretty,
            QgsGraduatedSymbolRendererV2.StdDev,
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

        renderer.setInvertedColorRamp(True)
        self.assertTrue(renderer.invertedColorRamp(),
                        "Get/set renderer inverted color ramp")
        renderer.setInvertedColorRamp(False)
        self.assertFalse(renderer.invertedColorRamp(),
                         "Get/set renderer inverted color ramp")

        value = '"value"*2'
        exp = QgsSymbolLayerV2Utils.fieldOrExpressionToExpression(value)
        valuestr = QgsSymbolLayerV2Utils.fieldOrExpressionFromExpression(exp)
        renderer.setRotationField(value)
        self.assertEqual(valuestr, renderer.rotationField(),
                         "Get/set renderer rotation field")

        value = '"value"*3'
        exp = QgsSymbolLayerV2Utils.fieldOrExpressionToExpression(value)
        valuestr = QgsSymbolLayerV2Utils.fieldOrExpressionFromExpression(exp)
        renderer.setSizeScaleField(value)
        self.assertEqual(valuestr, renderer.sizeScaleField(),
                         "Get/set renderer size scale field")

        renderer.setSourceColorRamp(ramp)
        self.assertEqual(
            dumpColorRamp(ramp),
            dumpColorRamp(renderer.sourceColorRamp()),
            "Get/set renderer color ramp")

        for sm in (
            QgsSymbolV2.ScaleArea,
            QgsSymbolV2.ScaleDiameter,
        ):
            renderer.setScaleMethod(sm)
            self.assertEqual(str(sm), str(renderer.scaleMethod()),
                             "Get/set renderer scale method")
        # test for classificatio with varying size
        renderer.setGraduatedMethod(QgsGraduatedSymbolRendererV2.GraduatedSize)
        renderer.setSourceColorRamp(None)
        renderer.addClassLowerUpper(0, 2)
        renderer.addClassLowerUpper(2, 4)
        renderer.addClassLowerUpper(4, 6)
        renderer.setSymbolSizes(2, 13)
        self.assertEqual(renderer.maxSymbolSize(), 13)
        self.assertEqual(renderer.minSymbolSize(), 2)
        refSizes = [2, (13 + 2) * .5, 13]
        ctx = QgsRenderContext()
        for idx, symbol in enumerate(renderer.symbols2(ctx)):
            self.assertEqual(symbol.size(), refSizes[idx])

    def testQgsGraduatedSymbolRendererV2_2(self):
        """Test QgsGraduatedSymbolRendererV2: Adding /removing/editing classes """
        # Create a renderer
        renderer = QgsGraduatedSymbolRendererV2()
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
        range = QgsRendererRangeV2(20.0, 25.5, symbol.clone(), 'Third range', False)
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
        element = renderer.save(doc)
        renderer2 = QgsGraduatedSymbolRendererV2.create(element)
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


#    void addClass( QgsSymbolV2* symbol );
#    //! @note available in python bindings as addClassRange
#    void addClass( QgsRendererRangeV2 range ) /PyName=addClassRange/;
#    //! @note available in python bindings as addClassLowerUpper
#    void addClass( double lower, double upper ) /PyName=addClassLowerUpper/;
#    void deleteClass( int idx );
#    void deleteAllClasses();

    def testQgsGraduatedSymbolRendererV2_3(self):
        """Test QgsGraduatedSymbolRendererV2: Reading attribute data, calculating classes """

        # Create a renderer
        renderer = QgsGraduatedSymbolRendererV2()
        symbol = createMarkerSymbol()
        renderer.setSourceSymbol(symbol.clone())

        # Test retrieving data values from a layer
        ml = createMemoryLayer((1.2, 0.5, 5.0, 1.0, 1.0, 1.2))
        # ... by attribute
        renderer.setClassAttribute("value")
        self.assertEqual(renderer.classAttribute(), "value", "Error in set/get classAttribute")
        data = renderer.getDataValues(ml)
        datastr = ':'.join([str(x) for x in data])
        self.assertEqual(datastr, '1.2:0.5:5.0:1.0:1.0:1.2', "Error returning field data")
        # ... by expression
        renderer.setClassAttribute('"value"*"value"')
        self.assertEqual(renderer.classAttribute(), '"value"*"value"', "Error in set/get classAttribute")
        data = renderer.getDataValues(ml)
        datastr = ':'.join([str(x) for x in data])
        self.assertEqual(datastr, '1.44:0.25:25.0:1.0:1.0:1.44', "Error returning field expression")

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

        # Tests still needed

        # Other calculation method tests
        # createRenderer function
        # symbolForFeature correctly selects range

if __name__ == "__main__":
    unittest.main()
