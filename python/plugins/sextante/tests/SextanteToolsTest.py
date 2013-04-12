import sextante
import unittest
from sextante.tests.TestData import points, points2, polygons, polygons2, lines, union,\
    table, polygonsGeoJson, raster
from sextante.core import Sextante
from sextante.tools.vector import values
from sextante.tools.general import getfromname

class SextanteToolsTest(unittest.TestCase):
    '''tests the method imported when doing an "import sextante", and also in sextante.tools. 
    They are mostly convenience tools'''
            
    def test_getobject(self):
        layer = sextante.getobject(points());
        self.assertIsNotNone(layer)        
        layer = sextante.getobject("points");
        self.assertIsNotNone(layer)
        
    def test_runandload(self):
        sextante.runandload("qgis:countpointsinpolygon",polygons(),points(),"NUMPOINTS", None)        
        layer = getfromname("Result")
        self.assertIsNotNone(layer)  
        
    def test_featuresWithoutSelection(self):
        layer = sextante.getobject(points())        
        features = sextante.getfeatures(layer)
        self.assertEqual(12, len(features))        
    
    def test_featuresWithSelection(self):
        layer = sextante.getobject(points())
        feature = layer.getFeatures().next()
        selected = [feature.id()]
        layer.setSelectedFeatures(selected)
        features = sextante.getfeatures(layer)        
        self.assertEqual(1, len(features))
        layer.setSelectedFeatures([])
    
    def test_attributeValues(self):
        layer = sextante.getobject(points()) 
        attributeValues = values(layer, "ID")
        i = 1
        for value in attributeValues['ID']:
            self.assertEqual(int(i), int(value))
            i+=1            
        self.assertEquals(13,i)
                
    def test_extent(self):
        pass
          


def suite():
    suite = unittest.makeSuite(SextanteToolsTest, 'test')
    return suite

def runtests():
    result = unittest.TestResult()
    testsuite = suite()
    testsuite.run(result)
    return result
