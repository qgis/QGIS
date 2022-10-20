# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsOgcUtils.

From build dir, run: ctest -R PyQgsOgcUtils -V


.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

"""
__author__ = 'René-Luc Dhont'
__date__ = '21/06/2019'
__copyright__ = 'Copyright 2019, The QGIS Project'

import qgis  # NOQA switch sip api

from qgis.PyQt.QtCore import QVariant
from qgis.PyQt.QtXml import QDomDocument
from qgis.core import QgsOgcUtils, QgsVectorLayer, QgsField

from qgis.testing import start_app, unittest

start_app()


class TestQgsOgcUtils(unittest.TestCase):

    def test_expressionFromOgcFilterWithInt(self):
        """
        Test expressionFromOgcFilter with Int type field
        """
        vl = QgsVectorLayer('Point', 'vl', 'memory')
        vl.dataProvider().addAttributes([QgsField('id', QVariant.Int)])
        vl.updateFields()

        # Literals are Integer 1 and 3
        f = '''<?xml version="1.0" encoding="UTF-8"?>
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
              <ogc:And>
                <ogc:PropertyIsGreaterThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>1</ogc:Literal>
                </ogc:PropertyIsGreaterThan>
                <ogc:PropertyIsLessThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>3</ogc:Literal>
                </ogc:PropertyIsLessThan>
              </ogc:And>
            </ogc:Filter>
        '''
        d = QDomDocument('filter')
        d.setContent(f, True)

        e = QgsOgcUtils.expressionFromOgcFilter(d.documentElement(), vl)
        self.assertEqual(e.expression(), 'id > 1 AND id < 3')

        # Literals are Double 1.0 and 3.0
        f = '''<?xml version="1.0" encoding="UTF-8"?>
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
              <ogc:And>
                <ogc:PropertyIsGreaterThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>1.0</ogc:Literal>
                </ogc:PropertyIsGreaterThan>
                <ogc:PropertyIsLessThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>3.0</ogc:Literal>
                </ogc:PropertyIsLessThan>
              </ogc:And>
            </ogc:Filter>
        '''
        d = QDomDocument('filter')
        d.setContent(f, True)

        e = QgsOgcUtils.expressionFromOgcFilter(d.documentElement(), vl)
        self.assertEqual(e.expression(), 'id > 1 AND id < 3')

        # Literals are Double 1.5 and 3.5
        f = '''<?xml version="1.0" encoding="UTF-8"?>
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
              <ogc:And>
                <ogc:PropertyIsGreaterThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>1.5</ogc:Literal>
                </ogc:PropertyIsGreaterThan>
                <ogc:PropertyIsLessThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>3.5</ogc:Literal>
                </ogc:PropertyIsLessThan>
              </ogc:And>
            </ogc:Filter>
        '''
        d = QDomDocument('filter')
        d.setContent(f, True)

        e = QgsOgcUtils.expressionFromOgcFilter(d.documentElement(), vl)
        self.assertEqual(e.expression(), 'id > 2 AND id < 4')

        # Literals are Scientific notation 15e-01 and 35e-01
        f = '''<?xml version="1.0" encoding="UTF-8"?>
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
              <ogc:And>
                <ogc:PropertyIsGreaterThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>15e-01</ogc:Literal>
                </ogc:PropertyIsGreaterThan>
                <ogc:PropertyIsLessThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>35e-01</ogc:Literal>
                </ogc:PropertyIsLessThan>
              </ogc:And>
            </ogc:Filter>
        '''
        d = QDomDocument('filter')
        d.setContent(f, True)

        e = QgsOgcUtils.expressionFromOgcFilter(d.documentElement(), vl)
        self.assertEqual(e.expression(), 'id > 2 AND id < 4')

    def test_expressionFromOgcFilterWithLonglong(self):
        """
        Test expressionFromOgcFilter with LongLong type field
        """
        vl = QgsVectorLayer('Point', 'vl', 'memory')
        vl.dataProvider().addAttributes([QgsField('id', QVariant.LongLong)])
        vl.updateFields()

        # Literals are Integer 1 and 3
        f = '''<?xml version="1.0" encoding="UTF-8"?>
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
              <ogc:And>
                <ogc:PropertyIsGreaterThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>1</ogc:Literal>
                </ogc:PropertyIsGreaterThan>
                <ogc:PropertyIsLessThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>3</ogc:Literal>
                </ogc:PropertyIsLessThan>
              </ogc:And>
            </ogc:Filter>
        '''
        d = QDomDocument('filter')
        d.setContent(f, True)

        e = QgsOgcUtils.expressionFromOgcFilter(d.documentElement(), vl)
        self.assertEqual(e.expression(), 'id > 1 AND id < 3')
        # Literals are Double 1.0 and 3.0
        f = '''<?xml version="1.0" encoding="UTF-8"?>
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
              <ogc:And>
                <ogc:PropertyIsGreaterThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>1.0</ogc:Literal>
                </ogc:PropertyIsGreaterThan>
                <ogc:PropertyIsLessThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>3.0</ogc:Literal>
                </ogc:PropertyIsLessThan>
              </ogc:And>
            </ogc:Filter>
        '''
        d = QDomDocument('filter')
        d.setContent(f, True)

        e = QgsOgcUtils.expressionFromOgcFilter(d.documentElement(), vl)
        self.assertEqual(e.expression(), 'id > 1 AND id < 3')

        # Literals are Double 1.5 and 3.5
        f = '''<?xml version="1.0" encoding="UTF-8"?>
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
              <ogc:And>
                <ogc:PropertyIsGreaterThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>1.5</ogc:Literal>
                </ogc:PropertyIsGreaterThan>
                <ogc:PropertyIsLessThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>3.5</ogc:Literal>
                </ogc:PropertyIsLessThan>
              </ogc:And>
            </ogc:Filter>
        '''
        d = QDomDocument('filter')
        d.setContent(f, True)

        e = QgsOgcUtils.expressionFromOgcFilter(d.documentElement(), vl)
        self.assertEqual(e.expression(), 'id > 2 AND id < 4')

        # Literals are Scientific notation 15e-01 and 35e-01
        f = '''<?xml version="1.0" encoding="UTF-8"?>
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
              <ogc:And>
                <ogc:PropertyIsGreaterThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>15e-01</ogc:Literal>
                </ogc:PropertyIsGreaterThan>
                <ogc:PropertyIsLessThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>35e-01</ogc:Literal>
                </ogc:PropertyIsLessThan>
              </ogc:And>
            </ogc:Filter>
        '''
        d = QDomDocument('filter')
        d.setContent(f, True)

        e = QgsOgcUtils.expressionFromOgcFilter(d.documentElement(), vl)
        self.assertEqual(e.expression(), 'id > 2 AND id < 4')

        # Literal is empty
        f = '''<?xml version="1.0" encoding="UTF-8"?>
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
              <ogc:PropertyIsEqualTo>
                <ogc:PropertyName>id</ogc:PropertyName>
                <ogc:Literal></ogc:Literal>
              </ogc:PropertyIsEqualTo>
            </ogc:Filter>
        '''
        d = QDomDocument('filter')
        d.setContent(f, True)

        e = QgsOgcUtils.expressionFromOgcFilter(d.documentElement(), vl)
        self.assertEqual(e.expression(), 'id = \'\'')

        f = '''<?xml version="1.0" encoding="UTF-8"?>
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
              <ogc:PropertyIsEqualTo>
                <ogc:PropertyName>id</ogc:PropertyName>
                <ogc:Literal/>
              </ogc:PropertyIsEqualTo>
            </ogc:Filter>
        '''
        d = QDomDocument('filter')
        d.setContent(f, True)

        e = QgsOgcUtils.expressionFromOgcFilter(d.documentElement(), vl)
        self.assertEqual(e.expression(), 'id = \'\'')

    def test_expressionFromOgcFilterWithDouble(self):
        """
        Test expressionFromOgcFilter with Double type field
        """
        vl = QgsVectorLayer('Point', 'vl', 'memory')
        vl.dataProvider().addAttributes([QgsField('id', QVariant.Double)])
        vl.updateFields()

        # Literals are Integer 1 and 3
        f = '''<?xml version="1.0" encoding="UTF-8"?>
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
              <ogc:And>
                <ogc:PropertyIsGreaterThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>1</ogc:Literal>
                </ogc:PropertyIsGreaterThan>
                <ogc:PropertyIsLessThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>3</ogc:Literal>
                </ogc:PropertyIsLessThan>
              </ogc:And>
            </ogc:Filter>
        '''
        d = QDomDocument('filter')
        d.setContent(f, True)

        e = QgsOgcUtils.expressionFromOgcFilter(d.documentElement(), vl)
        self.assertEqual(e.expression(), 'id > 1 AND id < 3')

        # Literals are Double 1.0 and 3.0
        f = '''<?xml version="1.0" encoding="UTF-8"?>
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
              <ogc:And>
                <ogc:PropertyIsGreaterThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>1.0</ogc:Literal>
                </ogc:PropertyIsGreaterThan>
                <ogc:PropertyIsLessThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>3.0</ogc:Literal>
                </ogc:PropertyIsLessThan>
              </ogc:And>
            </ogc:Filter>
        '''
        d = QDomDocument('filter')
        d.setContent(f, True)

        e = QgsOgcUtils.expressionFromOgcFilter(d.documentElement(), vl)
        self.assertEqual(e.expression(), 'id > 1 AND id < 3')

        # Literals are Double 1.5 and 3.5
        f = '''<?xml version="1.0" encoding="UTF-8"?>
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
              <ogc:And>
                <ogc:PropertyIsGreaterThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>1.5</ogc:Literal>
                </ogc:PropertyIsGreaterThan>
                <ogc:PropertyIsLessThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>3.5</ogc:Literal>
                </ogc:PropertyIsLessThan>
              </ogc:And>
            </ogc:Filter>
        '''
        d = QDomDocument('filter')
        d.setContent(f, True)

        e = QgsOgcUtils.expressionFromOgcFilter(d.documentElement(), vl)
        self.assertEqual(e.expression(), 'id > 1.5 AND id < 3.5')

        # Literals are Scientific notation 15e-01 and 35e-01
        f = '''<?xml version="1.0" encoding="UTF-8"?>
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
              <ogc:And>
                <ogc:PropertyIsGreaterThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>15e-01</ogc:Literal>
                </ogc:PropertyIsGreaterThan>
                <ogc:PropertyIsLessThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>35e-01</ogc:Literal>
                </ogc:PropertyIsLessThan>
              </ogc:And>
            </ogc:Filter>
        '''
        d = QDomDocument('filter')
        d.setContent(f, True)

        e = QgsOgcUtils.expressionFromOgcFilter(d.documentElement(), vl)
        self.assertEqual(e.expression(), 'id > 1.5 AND id < 3.5')

        # Literal is empty
        f = '''<?xml version="1.0" encoding="UTF-8"?>
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
              <ogc:PropertyIsEqualTo>
                <ogc:PropertyName>id</ogc:PropertyName>
                <ogc:Literal></ogc:Literal>
              </ogc:PropertyIsEqualTo>
            </ogc:Filter>
        '''
        d = QDomDocument('filter')
        d.setContent(f, True)

        e = QgsOgcUtils.expressionFromOgcFilter(d.documentElement(), vl)
        self.assertEqual(e.expression(), 'id = \'\'')

        f = '''<?xml version="1.0" encoding="UTF-8"?>
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
              <ogc:PropertyIsEqualTo>
                <ogc:PropertyName>id</ogc:PropertyName>
                <ogc:Literal/>
              </ogc:PropertyIsEqualTo>
            </ogc:Filter>
        '''
        d = QDomDocument('filter')
        d.setContent(f, True)

        e = QgsOgcUtils.expressionFromOgcFilter(d.documentElement(), vl)
        self.assertEqual(e.expression(), 'id = \'\'')

    def test_expressionFromOgcFilterWithString(self):
        """
        Test expressionFromOgcFilter with String type field
        """
        vl = QgsVectorLayer('Point', 'vl', 'memory')
        vl.dataProvider().addAttributes([QgsField('id', QVariant.String)])
        vl.updateFields()

        # Literals are Integer 1 and 3
        f = '''<?xml version="1.0" encoding="UTF-8"?>
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
              <ogc:And>
                <ogc:PropertyIsGreaterThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>1</ogc:Literal>
                </ogc:PropertyIsGreaterThan>
                <ogc:PropertyIsLessThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>3</ogc:Literal>
                </ogc:PropertyIsLessThan>
              </ogc:And>
            </ogc:Filter>
        '''
        d = QDomDocument('filter')
        d.setContent(f, True)

        e = QgsOgcUtils.expressionFromOgcFilter(d.documentElement(), vl)
        self.assertEqual(e.expression(), 'id > \'1\' AND id < \'3\'')

        # Literals are Double 1.0 and 3.0
        f = '''<?xml version="1.0" encoding="UTF-8"?>
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
              <ogc:And>
                <ogc:PropertyIsGreaterThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>1.0</ogc:Literal>
                </ogc:PropertyIsGreaterThan>
                <ogc:PropertyIsLessThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>3.0</ogc:Literal>
                </ogc:PropertyIsLessThan>
              </ogc:And>
            </ogc:Filter>
        '''
        d = QDomDocument('filter')
        d.setContent(f, True)

        e = QgsOgcUtils.expressionFromOgcFilter(d.documentElement(), vl)
        self.assertEqual(e.expression(), 'id > \'1.0\' AND id < \'3.0\'')

        # Literals are Double 1.5 and 3.5
        f = '''<?xml version="1.0" encoding="UTF-8"?>
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
              <ogc:And>
                <ogc:PropertyIsGreaterThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>1.5</ogc:Literal>
                </ogc:PropertyIsGreaterThan>
                <ogc:PropertyIsLessThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>3.5</ogc:Literal>
                </ogc:PropertyIsLessThan>
              </ogc:And>
            </ogc:Filter>
        '''
        d = QDomDocument('filter')
        d.setContent(f, True)

        e = QgsOgcUtils.expressionFromOgcFilter(d.documentElement(), vl)
        self.assertEqual(e.expression(), 'id > \'1.5\' AND id < \'3.5\'')

        # Literals are Scientific notation 15e-01 and 35e-01
        f = '''<?xml version="1.0" encoding="UTF-8"?>
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
              <ogc:And>
                <ogc:PropertyIsGreaterThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>15e-01</ogc:Literal>
                </ogc:PropertyIsGreaterThan>
                <ogc:PropertyIsLessThan>
                  <ogc:PropertyName>id</ogc:PropertyName>
                  <ogc:Literal>35e-01</ogc:Literal>
                </ogc:PropertyIsLessThan>
              </ogc:And>
            </ogc:Filter>
        '''
        d = QDomDocument('filter')
        d.setContent(f, True)

        e = QgsOgcUtils.expressionFromOgcFilter(d.documentElement(), vl)
        self.assertEqual(e.expression(), 'id > \'15e-01\' AND id < \'35e-01\'')

        # Literal is empty
        f = '''<?xml version="1.0" encoding="UTF-8"?>
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
              <ogc:PropertyIsEqualTo>
                <ogc:PropertyName>id</ogc:PropertyName>
                <ogc:Literal></ogc:Literal>
              </ogc:PropertyIsEqualTo>
            </ogc:Filter>
        '''
        d = QDomDocument('filter')
        d.setContent(f, True)

        e = QgsOgcUtils.expressionFromOgcFilter(d.documentElement(), vl)
        self.assertEqual(e.expression(), 'id = \'\'')

        f = '''<?xml version="1.0" encoding="UTF-8"?>
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
              <ogc:PropertyIsEqualTo>
                <ogc:PropertyName>id</ogc:PropertyName>
                <ogc:Literal/>
              </ogc:PropertyIsEqualTo>
            </ogc:Filter>
        '''
        d = QDomDocument('filter')
        d.setContent(f, True)

        e = QgsOgcUtils.expressionFromOgcFilter(d.documentElement(), vl)
        self.assertEqual(e.expression(), 'id = \'\'')

    def test_expressionFromOgcFilterWithAndOrPropertyIsLike(self):
        """
        Test expressionFromOgcFilter with And, Or and PropertyIsLike with wildCard
        """

        vl = QgsVectorLayer('Point', 'vl', 'memory')
        vl.dataProvider().addAttributes([QgsField('id', QVariant.LongLong), QgsField('THEME', QVariant.String), QgsField('PROGRAMME', QVariant.String)])
        vl.updateFields()

        f = '''<?xml version="1.0" encoding="UTF-8"?>
            <ogc:Filter xmlns:ogc="http://www.opengis.net/ogc">
              <ogc:And>
                <ogc:PropertyIsLike escape="\\" wildCard="%" singleChar="_">
                  <ogc:PropertyName>THEME</ogc:PropertyName>
                  <ogc:Literal>%Phytoplancton total%</ogc:Literal>
                </ogc:PropertyIsLike>
                <ogc:Or>
                  <ogc:PropertyIsLike escapeChar="\\" matchCase="false" singleChar="?" wildCard="*">
                    <ogc:PropertyName>PROGRAMME</ogc:PropertyName>
                    <ogc:Literal>REPHY;*</ogc:Literal>
                  </ogc:PropertyIsLike>
                  <ogc:PropertyIsLike escapeChar="\\" matchCase="false" singleChar="?" wildCard="*">
                    <ogc:PropertyName>PROGRAMME</ogc:PropertyName>
                    <ogc:Literal>*;REPHY</ogc:Literal>
                  </ogc:PropertyIsLike>
                  <ogc:PropertyIsLike escapeChar="\\" matchCase="false" singleChar="?" wildCard="*">
                    <ogc:PropertyName>PROGRAMME</ogc:PropertyName>
                    <ogc:Literal>*;REPHY;*</ogc:Literal>
                  </ogc:PropertyIsLike>
                  <ogc:PropertyIsLike escapeChar="\\" matchCase="false" singleChar="?" wildCard="*">
                    <ogc:PropertyName>PROGRAMME</ogc:PropertyName>
                    <ogc:Literal>^REPHY$</ogc:Literal>
                  </ogc:PropertyIsLike>
                </ogc:Or>
              </ogc:And>
            </ogc:Filter>
        '''
        d = QDomDocument('filter')
        d.setContent(f, True)

        e = QgsOgcUtils.expressionFromOgcFilter(d.documentElement(), vl)
        self.assertEqual(e.expression(), 'THEME LIKE \'%Phytoplancton total%\' AND (PROGRAMME ILIKE \'REPHY;%\' OR PROGRAMME ILIKE \'%;REPHY\' OR PROGRAMME ILIKE \'%;REPHY;%\' OR PROGRAMME ILIKE \'^REPHY$\')')


if __name__ == '__main__':
    unittest.main()
