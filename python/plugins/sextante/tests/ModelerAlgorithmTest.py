import unittest
import sextante
from sextante.tests.TestData import points, points2, polygons, polygons2, lines, union,\
    table, raster
from sextante.core.QGisLayers import QGisLayers
import os
from osgeo import gdal
from osgeo.gdalconst import GA_ReadOnly

class ModelerAlgorithmTest(unittest.TestCase):
    
    def testCreateModel(self):
        pass
        
    def testRemoveParameter(self):
        pass
        
    def testRemoveAlgorithm(self):
        pass
            
    
    def test_modelersagagrass(self):
        outputs=sextante.runalg("modeler:sagagrass",points(),None)
        output=outputs['CENTROIDS_ALG1']
        layer=QGisLayers.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['CAT']
        expectedtypes=['Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=sextante.getfeatures(layer)
        self.assertEqual(12, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1"]
        values=[str(attr.toString()) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POINT(270839.65586926 4458983.16267036)'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))
    
    def test_modelersimplemodel(self):
        outputs=sextante.runalg("modeler:simplemodel",raster(),None)
        output=outputs['SLOPE_ALG0']
        self.assertTrue(os.path.isfile(output))
        dataset=gdal.Open(output, GA_ReadOnly)
        strhash=hash(str(dataset.ReadAsArray(0).tolist()))
        self.assertEqual(strhash,1891122097)
        
    def test_modelerfieldautoextent(self):
        outputs=sextante.runalg("modeler:fieldautoextent",polygons(),"POLY_NUM_A",None)
        output=outputs['USER_GRID_ALG0']
        self.assertTrue(os.path.isfile(output))
        dataset=gdal.Open(output, GA_ReadOnly)
        strhash=hash(str(dataset.ReadAsArray(0).tolist()))
        self.assertEqual(strhash,2026100494)
        
            

def suite():
    suite = unittest.makeSuite(ModelerAlgorithmTest, 'test')    
    return suite

def runtests():
    result = unittest.TestResult()    
    testsuite = suite()
    testsuite.run(result)
    return result
        
