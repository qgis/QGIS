import unittest

from qgis.core import (QGis,
                        QgsVectorLayer,
                        QgsFeature,
                        QgsField,
                        QgsGeometry,
                        QgsPoint)

from PyQt4.QtCore import QVariant

from utilities import getQgisTestApp
QGISAPP, CANVAS, IFACE, PARENT = getQgisTestApp()

class TestQgsMemoryProvider(unittest.TestCase):

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
                      (3, provider.fieldCount()))

        assert provider.fieldCount() == 3, myMessage

        ft = QgsFeature()
        ft.setGeometry(QgsGeometry.fromPoint(QgsPoint(10,10)))
        ft.setAttributeMap({0 : QVariant("Johny"),
                            1 : QVariant(20),
                            2 : QVariant(0.3)})
        res, t = provider.addFeatures([ft])

        assert res, "Failed to add feature"

        myMessage = ('Expected: %s\nGot: %s\n' %
                      (1, provider.featureCount()))
        assert provider.featureCount() == 1, myMessage

        f = QgsFeature()
        provider.select()
        while provider.nextFeature(f):
          attrMap = f.attributeMap()
          myMessage = ('Expected: %s\nGot: %s\n' %
                        ("Johny", str(attrMap[0].toString())))

          assert str(attrMap[0].toString()) == "Johny", myMessage

          myMessage = ('Expected: %s\nGot: %s\n' %
                        (20, attrMap[1].toInt()[0]))

          assert attrMap[1].toInt()[0] == 20, myMessage

          myMessage = ('Expected: %s\nGot: %s\n' %
                        (0.3, attrMap[2].toFloat()[0]))

          assert (attrMap[0].toFloat()[0] - 0.3) < 0.0000001, myMessage

          geom = f.geometry()

          myMessage = ('Expected: %s\nGot: %s\n' %
                        ("POINT(10.000000 10.000000)", str(geom.exportToWkt())))

          assert str(geom.exportToWkt()) == "POINT(10.000000 10.000000)", myMessage

if __name__ == '__main__':
    unittest.main()
