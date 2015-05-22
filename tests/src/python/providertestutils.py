# -*- coding: utf-8 -*-
"""QGIS Unit test utils for provider tests.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Matthias Kuhn'
__date__ = '2015-04-27'
__copyright__ = 'Copyright 2015, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from qgis.core import QgsRectangle, QgsFeatureRequest

def runGetFeatureTests( vl ):
    assert len( [f for f in vl.getFeatures()] ) == 5
    assert len( [f for f in vl.getFeatures( QgsFeatureRequest().setFilterExpression( 'name IS NOT NULL' ) )] ) == 4
    assert len( [f for f in vl.getFeatures( QgsFeatureRequest().setFilterExpression('name LIKE \'Apple\'' ) )] ) == 1
    assert len( [f for f in vl.getFeatures( QgsFeatureRequest().setFilterExpression('name ILIKE \'aPple\'' ) )] ) == 1
    assert len( [f for f in vl.getFeatures( QgsFeatureRequest().setFilterExpression('name ILIKE \'%pp%\'' ) )] ) == 1
    assert len( [f for f in vl.getFeatures( QgsFeatureRequest().setFilterExpression('cnt > 0' ) ) ] ) == 4
    assert len( [f for f in vl.getFeatures( QgsFeatureRequest().setFilterExpression('cnt < 0' ) ) ] ) == 1
    assert len( [f for f in vl.getFeatures( QgsFeatureRequest().setFilterExpression('cnt >= 100' ) ) ] ) == 4
    assert len( [f for f in vl.getFeatures( QgsFeatureRequest().setFilterExpression('cnt <= 100' ) ) ] ) == 2
    assert len( [f for f in vl.getFeatures( QgsFeatureRequest().setFilterExpression('pk IN (1, 2, 4, 8)' ) )] ) == 3

def runGetFilterRectTests( vl ):
    extent = QgsRectangle(-70, 67, -60, 80)
    features = [ f.id() for f in vl.getFeatures( QgsFeatureRequest().setFilterRect( extent ) ) ]
    print (features)
    assert set( features ) == set( [2L, 4L] )
