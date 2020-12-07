# -*- coding: utf-8 -*-
"""QGIS Unit tests for ColorRamps.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Nyall Dawson'
__date__ = '2015-08'
__copyright__ = 'Copyright 2015, The QGIS Project'

import qgis  # NOQA

from qgis.core import (QgsGradientColorRamp,
                       QgsGradientStop,
                       QgsLimitedRandomColorRamp,
                       QgsRandomColorRamp,
                       QgsColorBrewerColorRamp,
                       QgsCptCityColorRamp,
                       QgsPresetSchemeColorRamp)
from qgis.PyQt.QtGui import QColor, QGradient
from qgis.testing import unittest


class PyQgsColorRamp(unittest.TestCase):

    def testQgsGradientColorRamp(self):
        # test QgsGradientStop
        stop = QgsGradientStop(0.9, QColor(200, 150, 100))
        self.assertEqual(stop.offset, 0.9)
        self.assertEqual(stop.color, QColor(200, 150, 100))
        self.assertEqual(QgsGradientStop(0.1, QColor(180, 20, 30)), QgsGradientStop(0.1, QColor(180, 20, 30)))
        self.assertNotEqual(QgsGradientStop(0.1, QColor(180, 20, 30)), QgsGradientStop(0.2, QColor(180, 20, 30)))
        self.assertNotEqual(QgsGradientStop(0.1, QColor(180, 20, 30)), QgsGradientStop(0.1, QColor(180, 40, 30)))

        # test gradient with only start/end color
        r = QgsGradientColorRamp(QColor(200, 0, 0, 100), QColor(0, 200, 0, 200))
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
        r = QgsGradientColorRamp(QColor(200, 0, 0), QColor(0, 200, 0), False, [QgsGradientStop(0.1, QColor(180, 20, 40)),
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
        fromProps = QgsGradientColorRamp.create(props)
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
        d = QgsGradientColorRamp(QColor(200, 0, 0), QColor(0, 200, 0), True)
        self.assertEqual(d.isDiscrete(), True)
        self.assertEqual(d.color(0), QColor(200, 0, 0))
        self.assertEqual(d.color(0.5), QColor(200, 0, 0))
        self.assertEqual(d.color(1), QColor(0, 200, 0))
        # then with stops
        d = QgsGradientColorRamp(QColor(200, 0, 0), QColor(0, 200, 0), True, [QgsGradientStop(0.1, QColor(180, 20, 40)),
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
        r = QgsGradientColorRamp(QColor(200, 0, 0), QColor(0, 200, 0), False, [QgsGradientStop(0.1, QColor(180, 20, 40)),
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

        # test continuous invert function
        r.invert()
        self.assertEqual(r.color(0), QColor(0, 200, 0))
        self.assertEqual(r.color(1), QColor(200, 0, 0))
        self.assertEqual(r.color(0.2), QColor(50, 20, 10))

        # test discrete invert function
        r = QgsGradientColorRamp(QColor(255, 255, 255), QColor(0, 0, 0), True, [QgsGradientStop(0.33, QColor(128, 128, 128)),
                                                                                QgsGradientStop(0.66, QColor(0, 0, 0))])
        self.assertEqual(r.color(0.2), QColor(255, 255, 255))
        self.assertEqual(r.color(0.5), QColor(128, 128, 128))
        self.assertEqual(r.color(0.8), QColor(0, 0, 0))
        r.invert()
        self.assertEqual(r.color(0.2), QColor(0, 0, 0))
        self.assertEqual(r.color(0.5), QColor(128, 128, 128))
        self.assertEqual(r.color(0.8), QColor(255, 255, 255))

        # test invalid value range
        r = QgsGradientColorRamp(color1=QColor(0, 0, 255), color2=QColor(0, 255, 0), discrete=False)
        self.assertEqual(r.color(0), QColor(0, 0, 255))
        self.assertEqual(r.color(1), QColor(0, 255, 0))
        self.assertEqual(r.color(0.5).name(), QColor(0, 128, 128).name())
        self.assertEqual(r.color(2), QColor(0, 255, 0))
        self.assertEqual(r.color(-1), QColor(0, 0, 255))
        self.assertEqual(r.color(float('nan')), QColor(0, 255, 0))

    def testQgsLimitedRandomColorRamp(self):
        # test random color ramp
        r = QgsLimitedRandomColorRamp(5)
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
        fromProps = QgsLimitedRandomColorRamp.create(props)
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

    def testQgsRandomColorRamp(self):
        # test random colors
        r = QgsRandomColorRamp()
        self.assertEqual(r.type(), 'randomcolors')
        self.assertEqual(r.count(), -1)  # no color count
        self.assertEqual(r.value(0), 0)  # all values should be 0
        self.assertEqual(r.value(1), 0)

        # test non-pregenerated colors. All should be valid
        for i in range(10000):
            c = r.color(0)
            self.assertTrue(c.isValid())

        # test creating from properties
        # QgsRandomColorRamp has no properties for now, but test to ensure no crash
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

    def testQgsPresetSchemeColorRamp(self):
        # test preset color ramp
        r = QgsPresetSchemeColorRamp()
        self.assertEqual(r.type(), 'preset')
        # should be forced to have at least one color
        self.assertEqual(r.count(), 1)

        # test getter/setter
        r = QgsPresetSchemeColorRamp([QColor(255, 0, 0), QColor(0, 255, 0), QColor(0, 0, 255), QColor(0, 0, 0)])
        self.assertEqual(r.colors(), [QColor(255, 0, 0), QColor(0, 255, 0), QColor(0, 0, 255), QColor(0, 0, 0)])
        r.setColors([(QColor(255, 0, 0), '1'), (QColor(0, 255, 0), '2')])
        self.assertEqual(r.colors(), [QColor(255, 0, 0), QColor(0, 255, 0)])
        self.assertEqual(r.fetchColors(), [(QColor(255, 0, 0), '1'), (QColor(0, 255, 0), '2')])

        # test value
        r = QgsPresetSchemeColorRamp([QColor(255, 0, 0), QColor(0, 255, 0), QColor(0, 0, 255), QColor(0, 0, 0), QColor(255, 255, 255)])
        self.assertEqual(r.value(0), 0)
        self.assertEqual(r.value(1), 0.25)
        self.assertEqual(r.value(2), 0.5)
        self.assertEqual(r.value(3), 0.75)
        self.assertEqual(r.value(4), 1)

        self.assertTrue(not r.color(-1).isValid())
        self.assertTrue(not r.color(5).isValid())

        # test generated colors
        for i in range(5):
            self.assertEqual(r.color(r.value(i)), r.colors()[i])

        # test creating from properties
        r.setColors([(QColor(255, 0, 0), '1'), (QColor(0, 255, 0), '2')])
        props = r.properties()
        fromProps = QgsPresetSchemeColorRamp.create(props)
        self.assertEqual(fromProps.count(), 2)
        self.assertEqual(fromProps.fetchColors(), r.fetchColors())

        # test cloning ramp
        cloned = r.clone()
        self.assertEqual(cloned.count(), 2)
        self.assertEqual(cloned.fetchColors(), r.fetchColors())

        # test invert function
        r.invert()
        self.assertEqual(r.color(0), QColor(0, 255, 0))
        self.assertEqual(r.color(1), QColor(255, 0, 0))

    def testQgsColorBrewerColorRamp(self):
        # test color brewer color ramps
        r = QgsColorBrewerColorRamp('OrRd', 6)
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
        bad = QgsColorBrewerColorRamp('badscheme', 6)
        self.assertFalse(bad.color(0).isValid())
        self.assertEqual(bad.value(1), 0)

        # test creating from properties
        props = r.properties()
        fromProps = QgsColorBrewerColorRamp.create(props)
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

        # test invert function
        r.invert()
        self.assertEqual(r.color(0), QColor(165, 15, 21))
        self.assertEqual(r.color(0.2), QColor(222, 45, 38))
        self.assertEqual(r.color(1), QColor(254, 229, 217))
        r.invert()

        # set colors
        r.setColors(3)
        self.assertEqual(r.colors(), 3)
        self.assertEqual(r.count(), 3)
        self.assertEqual(r.color(0), QColor(254, 224, 210))
        self.assertEqual(r.color(0.5), QColor(252, 146, 114))
        self.assertEqual(r.color(1.0), QColor(222, 45, 38))

        # test static members
        names = QgsColorBrewerColorRamp.listSchemeNames()
        self.assertTrue('Reds' in names and 'OrRd' in names)
        self.assertEqual(len(QgsColorBrewerColorRamp.listSchemeVariants('bad scheme')), 0)
        variants = QgsColorBrewerColorRamp.listSchemeVariants('Reds')
        self.assertEqual(variants, [3, 4, 5, 6, 7, 8, 9])

        # test invalid value range
        r = QgsColorBrewerColorRamp("Spectral", 5)
        self.assertEqual(r.color(0), QColor(215, 25, 28))
        self.assertEqual(r.color(1), QColor(43, 131, 186))
        self.assertEqual(r.color(0.5), QColor(255, 255, 191))
        self.assertFalse(r.color(2).isValid())
        self.assertFalse(r.color(-1).isValid())
        self.assertFalse(r.color(float('nan')).isValid())

    def testCptCityColorRamp(self):
        """Test Cpt-city color ramp"""
        r = QgsCptCityColorRamp("cb/div/BrBG_", "05")
        self.assertTrue(r)

        # color scheme name
        self.assertEqual(r.schemeName(), "cb/div/BrBG_")
        self.assertEqual(r.variantName(), "05")

        # number of colors
        self.assertEqual(r.count(), 6)

        # color for value
        self.assertEqual(r.color(0), QColor(166, 97, 26))
        self.assertEqual(r.color(1), QColor(1, 133, 113))
        self.assertEqual(r.color(0.5), QColor(245, 245, 245))
        self.assertEqual(r.color(2), QColor(1, 133, 113))
        self.assertEqual(r.color(-1), QColor(166, 97, 26))
        self.assertEqual(r.color(float('nan')), QColor(1, 133, 113))


if __name__ == '__main__':
    unittest.main()
