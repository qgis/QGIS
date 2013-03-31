import sextante
import unittest
from sextante.tests.TestData import points, points2, polygons, polygons2, lines, union,\
    table, polygonsGeoJson
from sextante.core.QGisLayers import QGisLayers
from sextante.core.SextanteUtils import SextanteUtils

class SagaTest(unittest.TestCase):
    '''tests for saga algorithms'''


    '''the following tests are not meant to test the algorithms themselves,
    but the algorithm provider, testing things such as the file conversion,
    the selection awareness of SAGA process, etc'''

    def test_SagaVvectorAlgorithmWithSelection(self):
        layer = sextante.getobject(polygons2());
        feature = layer.getFeatures().next()
        selected = [feature.id()]
        layer.setSelectedFeatures(selected)
        outputs=sextante.runalg("saga:polygoncentroids",polygons2(),True,None)
        output=outputs['CENTROIDS']
        layer=QGisLayers.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','POLY_NUM_B','POLY_ST_B']
        expectedtypes=['Real','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=sextante.getfeatures(layer)
        self.assertEqual(1, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["2","1","string a"]
        values=[str(attr.toString()) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POINT(270820.58189697 4458968.73298999)'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_SagaVectorAlgorithWithUnsupportedInputAndOutputFormat(self):
        '''this tests both the exporting to shp and then the format change in the output layer'''
        layer = sextante.getobject(polygonsGeoJson());
        feature = layer.getFeatures().next()
        selected = [feature.id()]
        layer.setSelectedFeatures(selected)
        outputs=sextante.runalg("saga:polygoncentroids",polygonsGeoJson(),True, SextanteUtils.getTempFilename("geojson"))
        output=outputs['CENTROIDS']
        layer=QGisLayers.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','POLY_NUM_A','POLY_ST_A']
        expectedtypes=['Real','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=sextante.getfeatures(layer)
        self.assertEqual(1, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["0","1.1","string a"]
        values=[str(attr.toString()) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POINT(270787.49991451 4458955.46775295)'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))







def suite():
    suite = unittest.makeSuite(SagaTest, 'test')
    return suite

def runtests():
    result = unittest.TestResult()
    testsuite = suite()
    testsuite.run(result)
    return result
