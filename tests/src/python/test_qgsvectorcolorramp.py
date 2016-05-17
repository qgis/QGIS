# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsVectorColorRamps.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2015-08'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import qgis  # NOQA

from qgis.core import (QgsVectorGradientColorRampV2,
                       QgsGradientStop,
                       QgsVectorRandomColorRampV2,
                       QgsRandomColorsV2,
                       QgsVectorColorBrewerColorRampV2)
from qgis.PyQt.QtGui import QColor, QGradient
from qgis.testing import unittest


class PyQgsVectorColorRamp(unittest.TestCase):

    def testQgsVectorGradientRampV2(self):
        # test QgsGradientStop
        stop = QgsGradientStop(0.9, QColor(200, 150, 100))
        self.assertEqual(stop.offset, 0.9)
        self.assertEqual(stop.color, QColor(200, 150, 100))
        self.assertEqual(QgsGradientStop(0.1, QColor(180, 20, 30)), QgsGradientStop(0.1, QColor(180, 20, 30)))
        self.assertNotEqual(QgsGradientStop(0.1, QColor(180, 20, 30)), QgsGradientStop(0.2, QColor(180, 20, 30)))
        self.assertNotEqual(QgsGradientStop(0.1, QColor(180, 20, 30)), QgsGradientStop(0.1, QColor(180, 40, 30)))

        # test gradient with only start/end color
        r = QgsVectorGradientColorRampV2(QColor(200, 0, 0, 100), QColor(0, 200, 0, 200))
        self.assertEqual(r.type(), 'gradient')
        self.assertEqual(r.color1(), QColor(200, 0, 0, 100))
        self.assertEqual(r.color2(), QColor(0, 200, 0, 200))
        self.assertEqual(r.isDiscrete(), False)
        self.assertEqual(len(r.stops()), 0)
        self.assertEqual(r.count(), 2)
        self.assertEqual(r.value(0), 0.0)
        self.assertEqual(r.value(1), 1.0)
        self.assertEqual(r.color(0), QColor(200, 0, 0, 100))
        self.assertEqual(r.color(1), QColor(0, 200, 0, 200))
        self.assertEqual(r.color(0.5), QColor(100, 100, 0, 150))

        # test gradient with stops
        r = QgsVectorGradientColorRampV2(QColor(200, 0, 0), QColor(0, 200, 0), False, [QgsGradientStop(0.1, QColor(180, 20, 40)),
                                                                                       QgsGradientStop(0.9, QColor(40, 60, 100))])
        self.assertEqual(r.color1(), QColor(200, 0, 0))
        self.assertEqual(r.color2(), QColor(0, 200, 0))
        self.assertEqual(r.isDiscrete(), False)
        self.assertEqual(len(r.stops()), 2)
        self.assertEqual(r.count(), 4)
        self.assertEqual(r.value(0), 0.0)
        self.assertEqual(r.value(1), 0.1)
        self.assertEqual(r.value(2), 0.9)
        self.assertEqual(r.value(3), 1.0)
        self.assertEqual(r.color(0), QColor(200, 0, 0))
        self.assertEqual(r.color(0.05), QColor(190, 10, 20))
        self.assertEqual(r.color(0.1), QColor(180, 20, 40))
        self.assertEqual(r.color(0.5), QColor(110, 40, 70))
        self.assertEqual(r.color(0.9), QColor(40, 60, 100))
        self.assertEqual(r.color(0.95), QColor(20, 130, 50))
        self.assertEqual(r.color(1), QColor(0, 200, 0))

        # test setters
        r.setColor1(QColor(0, 0, 200))
        self.assertEqual(r.color1(), QColor(0, 0, 200))
        self.assertEqual(r.color(0), QColor(0, 0, 200))
        r.setColor2(QColor(0, 0, 100))
        self.assertEqual(r.color2(), QColor(0, 0, 100))
        self.assertEqual(r.color(1.0), QColor(0, 0, 100))
        r.setStops([QgsGradientStop(0.4, QColor(100, 100, 40))])
        s = r.stops()
        self.assertEqual(len(s), 1)
        self.assertEqual(s[0].offset, 0.4)
        self.assertEqual(s[0].color, QColor(100, 100, 40))

        # test info
        r.setInfo({'key1': 'val1', 'key2': 'val2'})
        self.assertEqual(r.info()['key1'], 'val1')
        self.assertEqual(r.info()['key2'], 'val2')

        # test creating from properties
        props = r.properties()
        fromProps = QgsVectorGradientColorRampV2.create(props)
        self.assertEqual(fromProps.color1(), QColor(0, 0, 200))
        self.assertEqual(fromProps.color2(), QColor(0, 0, 100))
        s = fromProps.stops()
        self.assertEqual(len(s), 1)
        self.assertEqual(s[0].offset, 0.4)
        c = QColor(s[0].color)
        self.assertEqual(c, QColor(100, 100, 40))
        self.assertEqual(fromProps.info()['key1'], 'val1')
        self.assertEqual(fromProps.info()['key2'], 'val2')
        self.assertEqual(fromProps.isDiscrete(), False)

        # test cloning ramp
        cloned = r.clone()
        self.assertEqual(cloned.color1(), QColor(0, 0, 200))
        self.assertEqual(cloned.color2(), QColor(0, 0, 100))
        s = cloned.stops()
        self.assertEqual(len(s), 1)
        self.assertEqual(s[0].offset, 0.4)
        c = QColor(s[0].color)
        self.assertEqual(c, QColor(100, 100, 40))
        self.assertEqual(cloned.info()['key1'], 'val1')
        self.assertEqual(cloned.info()['key2'], 'val2')
        self.assertEqual(cloned.isDiscrete(), False)

        # test discrete ramps
        # first with no stops
        d = QgsVectorGradientColorRampV2(QColor(200, 0, 0), QColor(0, 200, 0), True)
        self.assertEqual(d.isDiscrete(), True)
        self.assertEqual(d.color(0), QColor(200, 0, 0))
        self.assertEqual(d.color(0.5), QColor(200, 0, 0))
        self.assertEqual(d.color(1), QColor(0, 200, 0))
        # then with stops
        d = QgsVectorGradientColorRampV2(QColor(200, 0, 0), QColor(0, 200, 0), True, [QgsGradientStop(0.1, QColor(180, 20, 40)),
                                                                                      QgsGradientStop(0.9, QColor(40, 60, 100))])
        self.assertEqual(d.isDiscrete(), True)
        self.assertEqual(d.color(0), QColor(200, 0, 0))
        self.assertEqual(d.color(0.05), QColor(200, 0, 0))
        self.assertEqual(d.color(0.1), QColor(180, 20, 40))
        self.assertEqual(d.color(0.5), QColor(180, 20, 40))
        self.assertEqual(d.color(0.9), QColor(40, 60, 100))
        self.assertEqual(d.color(0.95), QColor(40, 60, 100))
        self.assertEqual(d.color(1), QColor(0, 200, 0))

        # to gradient
        g = QGradient()
        r = QgsVectorGradientColorRampV2(QColor(200, 0, 0), QColor(0, 200, 0), False, [QgsGradientStop(0.1, QColor(180, 20, 40)),
                                                                                       QgsGradientStop(0.9, QColor(40, 60, 100))])
        r.addStopsToGradient(g, 0.5)
        self.assertEqual(len(g.stops()), 4)
        self.assertEqual(g.stops()[0], (0.0, QColor(200, 0, 0, 127)))
        self.assertEqual(g.stops()[1], (0.1, QColor(180, 20, 40, 127)))
        self.assertEqual(g.stops()[2], (0.9, QColor(40, 60, 100, 127)))
        self.assertEqual(g.stops()[3], (1.0, QColor(0, 200, 0, 127)))

        # test that stops are ordered when setting them
        # first add some out-of-order stops
        r.setStops([QgsGradientStop(0.4, QColor(100, 100, 40)),
                    QgsGradientStop(0.2, QColor(200, 200, 80)),
                    QgsGradientStop(0.8, QColor(50, 20, 10)),
                    QgsGradientStop(0.6, QColor(10, 10, 4))])
        s = r.stops()
        self.assertEqual(len(s), 4)
        self.assertEqual(s[0].offset, 0.2)
        self.assertEqual(s[0].color, QColor(200, 200, 80))
        self.assertEqual(s[1].offset, 0.4)
        self.assertEqual(s[1].color, QColor(100, 100, 40))
        self.assertEqual(s[2].offset, 0.6)
        self.assertEqual(s[2].color, QColor(10, 10, 4))
        self.assertEqual(s[3].offset, 0.8)
        self.assertEqual(s[3].color, QColor(50, 20, 10))

    def testQgsVectorRandomColorRampV2(self):
        # test random color ramp
        r = QgsVectorRandomColorRampV2(5)
        self.assertEqual(r.type(), 'random')
        self.assertEqual(r.count(), 5)
        self.assertEqual(r.value(0), 0)
        self.assertEqual(r.value(1), 0.25)
        self.assertEqual(r.value(2), 0.5)
        self.assertEqual(r.value(3), 0.75)
        self.assertEqual(r.value(4), 1)

        self.assertTrue(not r.color(-1).isValid())
        self.assertTrue(not r.color(5).isValid())

        # test that generated random colors are all valid
        for i in range(10000):
            r.updateColors()
            for j in range(5):
                self.assertTrue(r.color(r.value(j)).isValid())

        # test setters
        r.setHueMin(10)
        self.assertEqual(r.hueMin(), 10)
        r.setHueMax(60)
        self.assertEqual(r.hueMax(), 60)
        r.setSatMin(70)
        self.assertEqual(r.satMin(), 70)
        r.setSatMax(100)
        self.assertEqual(r.satMax(), 100)
        r.setValMin(150)
        self.assertEqual(r.valMin(), 150)
        r.setValMax(200)
        self.assertEqual(r.valMax(), 200)
        # test that generated random colors are within range
        for i in range(10000):
            r.updateColors()
            for j in range(5):
                c = r.color(r.value(j))
                self.assertTrue(c.isValid())
                self.assertTrue(c.hue() >= r.hueMin())
                self.assertTrue(c.hue() <= r.hueMax())
                self.assertTrue(c.saturation() >= r.satMin())
                self.assertTrue(c.saturation() <= r.satMax())
                self.assertTrue(c.value() >= r.valMin())
                self.assertTrue(c.value() <= r.valMax())

        # test creating from properties
        props = r.properties()
        fromProps = QgsVectorRandomColorRampV2.create(props)
        self.assertEqual(fromProps.count(), 5)
        self.assertEqual(fromProps.hueMin(), 10)
        self.assertEqual(fromProps.hueMax(), 60)
        self.assertEqual(fromProps.satMin(), 70)
        self.assertEqual(fromProps.satMax(), 100)
        self.assertEqual(fromProps.valMin(), 150)
        self.assertEqual(fromProps.valMax(), 200)

        # test cloning ramp
        cloned = r.clone()
        self.assertEqual(cloned.count(), 5)
        self.assertEqual(cloned.hueMin(), 10)
        self.assertEqual(cloned.hueMax(), 60)
        self.assertEqual(cloned.satMin(), 70)
        self.assertEqual(cloned.satMax(), 100)
        self.assertEqual(cloned.valMin(), 150)
        self.assertEqual(cloned.valMax(), 200)

        # test randomColors static method
        for i in range(10000):
            cols = r.randomColors(10, 30, 60, 90, 120, 150, 180)
            self.assertEqual(len(cols), 10)
            for c in cols:
                self.assertTrue(c.isValid())
                self.assertTrue(c.hue() >= 30)
                self.assertTrue(c.hue() <= 60)
                self.assertTrue(c.saturation() >= 90)
                self.assertTrue(c.saturation() <= 120)
                self.assertTrue(c.value() >= 150)
                self.assertTrue(c.value() <= 180)

    def testQgsRandomColorsV2(self):
        # test random colors
        r = QgsRandomColorsV2()
        self.assertEqual(r.type(), 'randomcolors')
        self.assertEqual(r.count(), -1)  # no color count
        self.assertEqual(r.value(0), 0)  # all values should be 0
        self.assertEqual(r.value(1), 0)

        # test non-pregenerated colors. All should be valid
        for i in range(10000):
            c = r.color(0)
            self.assertTrue(c.isValid())

        # test creating from properties
        # QgsRandomColorsV2 has no properties for now, but test to ensure no crash
        props = r.properties()  # NOQA

        # test cloning ramp
        cloned = r.clone()
        self.assertEqual(cloned.type(), 'randomcolors')

        # test with pregenerated colors
        for i in range(10000):
            r.setTotalColorCount(10)
            for j in range(10):
                c = r.color(j * 0.1)
                self.assertTrue(c.isValid())

    def testQgsVectorColorBrewerColorRampV2(self):
        # test color brewer color ramps
        r = QgsVectorColorBrewerColorRampV2('OrRd', 6)
        self.assertEqual(r.type(), 'colorbrewer')
        self.assertEqual(r.schemeName(), 'OrRd')
        self.assertEqual(r.count(), 6)
        self.assertEqual(r.value(0), 0)
        self.assertEqual(r.value(1), 0.2)
        self.assertEqual(r.value(2), 0.4)
        self.assertEqual(r.value(3), 0.6)
        self.assertEqual(r.value(4), 0.8)
        self.assertEqual(r.value(5), 1)

        self.assertTrue(not r.color(-1).isValid())
        self.assertTrue(not r.color(6).isValid())
        self.assertEqual(r.color(0), QColor(254, 240, 217))
        self.assertEqual(r.color(0.2), QColor(253, 212, 158))
        self.assertEqual(r.color(0.4), QColor(253, 187, 132))
        self.assertEqual(r.color(0.6), QColor(252, 141, 89))
        self.assertEqual(r.color(0.8), QColor(227, 74, 51))
        self.assertEqual(r.color(1.0), QColor(179, 0, 0))

        # try using an invalid scheme name
        bad = QgsVectorColorBrewerColorRampV2('badscheme', 6)
        self.assertFalse(bad.color(0).isValid())
        self.assertEqual(bad.value(1), 0)

        # test creating from properties
        props = r.properties()
        fromProps = QgsVectorColorBrewerColorRampV2.create(props)
        self.assertEqual(fromProps.type(), 'colorbrewer')
        self.assertEqual(fromProps.schemeName(), 'OrRd')
        self.assertEqual(fromProps.count(), 6)
        self.assertEqual(fromProps.color(0), QColor(254, 240, 217))
        self.assertEqual(fromProps.color(0.2), QColor(253, 212, 158))
        self.assertEqual(fromProps.color(0.4), QColor(253, 187, 132))
        self.assertEqual(fromProps.color(0.6), QColor(252, 141, 89))
        self.assertEqual(fromProps.color(0.8), QColor(227, 74, 51))
        self.assertEqual(fromProps.color(1.0), QColor(179, 0, 0))

        # test cloning ramp
        cloned = r.clone()
        self.assertEqual(cloned.type(), 'colorbrewer')
        self.assertEqual(cloned.schemeName(), 'OrRd')
        self.assertEqual(cloned.count(), 6)
        self.assertEqual(cloned.color(0), QColor(254, 240, 217))
        self.assertEqual(cloned.color(0.2), QColor(253, 212, 158))
        self.assertEqual(cloned.color(0.4), QColor(253, 187, 132))
        self.assertEqual(cloned.color(0.6), QColor(252, 141, 89))
        self.assertEqual(cloned.color(0.8), QColor(227, 74, 51))
        self.assertEqual(cloned.color(1.0), QColor(179, 0, 0))

        # set scheme name
        r.setSchemeName('Reds')
        self.assertEqual(r.schemeName(), 'Reds')
        self.assertEqual(r.count(), 6)
        self.assertEqual(r.color(0), QColor(254, 229, 217))
        self.assertEqual(r.color(0.2), QColor(252, 187, 161))
        self.assertEqual(r.color(0.4), QColor(252, 146, 114))
        self.assertEqual(r.color(0.6), QColor(251, 106, 74))
        self.assertEqual(r.color(0.8), QColor(222, 45, 38))
        self.assertEqual(r.color(1.0), QColor(165, 15, 21))

        # set colors
        r.setColors(3)
        self.assertEqual(r.colors(), 3)
        self.assertEqual(r.count(), 3)
        self.assertEqual(r.color(0), QColor(254, 224, 210))
        self.assertEqual(r.color(0.5), QColor(252, 146, 114))
        self.assertEqual(r.color(1.0), QColor(222, 45, 38))

        # test static members
        names = QgsVectorColorBrewerColorRampV2.listSchemeNames()
        self.assertTrue('Reds' in names and 'OrRd' in names)
        self.assertEqual(len(QgsVectorColorBrewerColorRampV2.listSchemeVariants('bad scheme')), 0)
        variants = QgsVectorColorBrewerColorRampV2.listSchemeVariants('Reds')
        self.assertEqual(variants, [3, 4, 5, 6, 7, 8, 9])

if __name__ == '__main__':
    unittest.main()
