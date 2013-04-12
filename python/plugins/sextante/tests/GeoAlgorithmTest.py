import sextante
import unittest
from sextante.tests.TestData import points, points2, polygons, polygons2, lines, union,\
    table, polygonsGeoJson, raster
from sextante.core.QGisLayers import QGisLayers
from sextante.core.SextanteUtils import SextanteUtils

class GeoAlgorithmTest(unittest.TestCase):    
    
    def testWrongformat(self):        
        outputs=sextante.runalg("qgis:countpointsinpolygon",polygons(),points(),"NUMPOINTS",SextanteUtils.getTempFilename("wrongext"))
        output=outputs['OUTPUT']
        self.assertTrue(output.endswith('shp'))        
        layer=QGisLayers.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','POLY_NUM_A','POLY_ST_A','NUMPOINTS']
        expectedtypes=['Integer','Real','String','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=sextante.getfeatures(layer)
        self.assertEqual(2, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","string a","6"]
        values=[str(attr.toString()) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        

def suite():
    suite = unittest.makeSuite(GeoAlgorithmTest, 'test')
    return suite

def runtests():
    result = unittest.TestResult()
    testsuite = suite()
    testsuite.run(result)
    return result
