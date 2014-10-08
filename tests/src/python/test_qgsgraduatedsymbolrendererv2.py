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

import qgis
from utilities import unittest, TestCase
from qgis.core import QgsGraduatedSymbolRendererV2, QgsRendererRangeV2, QgsRendererRangeV2LabelFormat
from qgis.core import QgsMarkerSymbolV2

class TestGSRUtilities:

    @staticmethod
    def createMarkerSymbol():
        symbol=QgsMarkerSymbolV2.createSimple({
            "color":"100,150,50",
            "name":"square",
            "size":"3.0"
        })
        return symbol

class TestQgsGraduatedSymbolRendererV2(TestCase):


    def testQgsRendererRangeV2_1(self):
        """Test QgsRendererRangeV2 getter/setter functions"""
        range=QgsRendererRangeV2()
        self.assertTrue(range)
        lower=123.45
        upper=234.56
        label="Test label"
        symbol=TestGSRUtilities.createMarkerSymbol()
        range.setLowerValue(lower)
        self.assertEqual(range.lowerValue(),lower,"Lower value getter/setter failed")
        range.setUpperValue(upper)
        self.assertEqual(range.upperValue(),upper,"Upper value getter/setter failed")
        range.setLabel(label)
        self.assertEqual(range.label(),label,"Label getter/setter failed")
        range.setRenderState(True)
        self.assertTrue(range.renderState(),"Render state getter/setter failed")
        range.setRenderState(False)
        self.assertFalse(range.renderState(),"Render state getter/setter failed")
        range.setSymbol(symbol.clone())
        self.assertEqual(symbol.dump(),range.symbol().dump(),"Symbol getter/setter failed")
        range2=QgsRendererRangeV2(lower,upper,symbol.clone(),label,False)
        self.assertEqual(range2.lowerValue(),lower,"Lower value from constructor failed")
        self.assertEqual(range2.upperValue(),upper,"Upper value from constructor failed")
        self.assertEqual(range2.label(),label,"Label from constructor failed")
        self.assertEqual(range2.symbol().dump(),symbol.dump(),"Symbol from constructor failed")
        self.assertFalse(range2.renderState(),"Render state getter/setter failed")

    def testQgsRendererRangeV2LabelFormat_1(self):
        """Test QgsRendererRangeV2LabelFormat getter/setter functions"""
        format=QgsRendererRangeV2LabelFormat()
        self.assertTrue(format,"QgsRendererRangeV2LabelFomat construction failed")
        template="%1 - %2 metres"
        precision=5
        format.setFormat(template)
        self.assertEqual(format.format(),template,"Format getter/setter failed")
        format.setPrecision(precision)
        self.assertEqual(format.precision(),precision,"Precision getter/setter failed")
        format.setTrimTrailingZeroes(True)
        self.assertTrue(format.trimTrailingZeroes(),"TrimTrailingZeroes getter/setter failed")
        format.setTrimTrailingZeroes(False)
        self.assertFalse(format.trimTrailingZeroes(),"TrimTrailingZeroes getter/setter failed")
        minprecision=-6;
        maxprecision=15;
        self.assertEqual(QgsRendererRangeV2LabelFormat.MinPrecision,minprecision,"Minimum precision != -6")
        self.assertEqual(QgsRendererRangeV2LabelFormat.MaxPrecision,maxprecision,"Maximum precision != 15")
        format.setPrecision(-10)
        self.assertEqual(format.precision(),minprecision,"Minimum precision not enforced")
        format.setPrecision(20)
        self.assertEqual(format.precision(),maxprecision,"Maximum precision not enforced")

    def testQgsRendererRangeV2LabelFormat_2(self):
        """Test QgsRendererRangeV2LabelFormat number format"""
        format=QgsRendererRangeV2LabelFormat()
        # Tests have precision, trim, value, expected
        # (Note: Not sure what impact of locale is on these tests)
        tests=(
            (2,False,1.0,'1.00'),
            (2,True,1.0,'1'),
            (2,False,1.234,'1.23'),
            (2,True,1.234,'1.23'),
            (2,False,1.236,'1.24'),
            (2,False,-1.236,'-1.24'),
            (2,False,-0.004,'0.00'),
            (2,True,1.002,'1'),
            (2,True,1.006,'1.01'),
            (2,True,1.096,'1.1'),
            (3,True,1.096,'1.096'),
            (-2,True,1496.45,'1500'),
            (-2,True,149.45,'100'),
            (-2,True,79.45,'100'),
            (-2,True,49.45,'0'),
            (-2,True,-49.45,'0'),
            (-2,True,-149.45,'-100'),
        )
        for f in tests:
            precision,trim,value,expected=f
            format.setPrecision(precision)
            format.setTrimTrailingZeroes(trim)
            result=format.formatNumber(value)
            testname="{0}:{1}:{2}".format(precision,trim,value)
            self.assertEqual(result,expected,
                             "Number format error {0}:{1}:{2} => {3}".format(
                             precision,trim,value,result))

        # Label tests - label format, expected result. 
        # Labels will be evaluated with lower=1.23 upper=2.34, precision=2
        ltests=(
            ("%1 - %2","1.23 - 2.34"),
            ("%1","1.23"),
            ("%2","2.34"),
            ("%2%","2.34%"),
            ("%1%1","1.231.23"),
            ("from %1 to %2 metres","from 1.23 to 2.34 metres"),
            ("from %2 to %1 metres","from 2.34 to 1.23 metres"),
            )
        format.setPrecision(2)
        format.setTrimTrailingZeroes(False)
        lower=1.232
        upper=2.339
        for t in ltests:
            label,expected=t
            format.setFormat(label)
            result=format.labelForLowerUpper(lower,upper)
            self.assertEqual(result,expected,"Label format error {0} => {1}".format(
                label,result))

        range=QgsRendererRangeV2()
        range.setLowerValue(lower)
        range.setUpperValue(upper)
        label=ltests[0][0]
        format.setFormat(label)
        result=format.labelForRange(range)
        self.assertEqual(result,ltests[0][1],"Label for range error {0} => {1}".format(
                label,result))


    def testQgsGraduatedSymbolRendererV2_1(self):
        """Test QgsGraduatedSymbolRendererV2 """
        pass
          



if __name__ == "__main__":
	unittest.main()
