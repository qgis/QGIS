# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsMemoryProvider.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""
__author__ = 'Alexander Bruy'
__date__ = '20/08/2012'
__copyright__ = 'Copyright 2012, The QGIS Project'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4.QtCore import QVariant

from qgis.core import (QGis,
                        QgsVectorLayer,
                        QgsFeature,
                        QgsFeatureRequest,
                        QgsField,
                        QgsGeometry,
                        QgsPoint)

from utilities import (getQgisTestApp,
                       TestCase,
                       unittest
                       #expectedFailure
                       )
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()


class TestQgsMemoryProvider(TestCase):

    def testPointCtor(self):
        layer = QgsVectorLayer("Point", "test", "memory")
        assert layer.isValid(), "Failed to create valid point memory layer"

    def testLayerGeometry(self):
        layer = QgsVectorLayer("Point", "test", "memory")

        myMessage = ('Expected: %s\nGot: %s\n' %
                      (QGis.Point, layer.geometryType()))
        assert layer.geometryType() == QGis.Point, myMessage

        myMessage = ('Expected: %s\nGot: %s\n' %
                      (QGis.WKBPoint, layer.wkbType()))
        assert layer.wkbType() == QGis.WKBPoint, myMessage

    def testAddFeatures(self):
        layer = QgsVectorLayer("Point", "test", "memory")
        provider = layer.dataProvider()

        res = provider.addAttributes([QgsField("name", QVariant.String,),
                                      QgsField("age",  QVariant.Int),
                                      QgsField("size", QVariant.Double)])
        assert res, "Failed to add attributes"

        myMessage = ('Expected: %s\nGot: %s\n' %
                     (3, len(provider.fields())))

        assert len(provider.fields()) == 3, myMessage

        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromPoint(QgsPoint(10,10)))
        ft.setAttributes([ QVariant("Johny"),
                           QVariant(20),
                           QVariant(0.3) ])
        res, t = provider.addFeatures([ft])

        assert res, "Failed to add feature"

        myMessage = ('Expected: %s\nGot: %s\n' %
                      (1, provider.featureCount()))
        assert provider.featureCount() == 1, myMessage

        for f in provider.getFeatures(QgsFeatureRequest()):
            attrs = f.attributes()
            myMessage = ('Expected: %s\nGot: %s\n' %
                         ("Johny", str(attrs[0].toString())))

            assert str(attrs[0].toString()) == "Johny", myMessage

            myMessage = ('Expected: %s\nGot: %s\n' %
                         (20, attrs[1].toInt()[0]))

            assert attrs[1].toInt()[0] == 20, myMessage

            myMessage = ('Expected: %s\nGot: %s\n' %
                         (0.3, attrs[2].toFloat()[0]))

            assert (attrs[0].toFloat()[0] - 0.3) < 0.0000001, myMessage

            geom = f.geometry()

            myMessage = ('Expected: %s\nGot: %s\n' %
                        ("POINT(10.0 10.0)", str(geom.exportToWkt())))

            assert str(geom.exportToWkt()) == "POINT(10.0 10.0)", myMessage

    def testGetFields(self):
        layer = QgsVectorLayer("Point", "test", "memory")
        provider = layer.dataProvider()

        provider.addAttributes([QgsField("name", QVariant.String,),
                                      QgsField("age",  QVariant.Int),
                                      QgsField("size", QVariant.Double)])
        myMessage = ('Expected: %s\nGot: %s\n' %
                     (3, len(provider.fields())))

        assert len(provider.fields()) == 3, myMessage

        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromPoint(QgsPoint(10,10)))
        ft.setAttributes([QVariant("Johny"),
                          QVariant(20),
                          QVariant(0.3)])
        provider.addFeatures([ft])

        for f in provider.getFeatures(QgsFeatureRequest()):
            myMessage = ('Expected: %s\nGot: %s\n' %
                         ("Johny", str(f['name'].toString())))

            self.assertEqual(str(f["name"].toString()), "Johny", myMessage)


    def testFromUri(self):
        """Test we can construct the mem provider from a uri"""
        myMemoryLayer = QgsVectorLayer(
            ('Point?crs=epsg:4326&field=name:string(20)&'
             'field=age:integer&field=size:double&index=yes'),
            'test',
            'memory')

        assert myMemoryLayer is not None, 'Provider not initialised'
        myProvider = myMemoryLayer.dataProvider()
        assert myProvider is not None

if __name__ == '__main__':
    unittest.main()
