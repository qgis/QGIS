import sextante
import unittest
from sextante.tests.TestData import points, points2, polygons, polygons2, lines, union,\
    table, polygonsGeoJson, raster
from sextante.core.QGisLayers import QGisLayers
import os
from osgeo import gdal
from osgeo.gdalconst import GA_ReadOnly
from sextante.core.SextanteUtils import SextanteUtils

class GdalTest(unittest.TestCase):
    
    def test_gdalogrsieve(self):
        outputs=sextante.runalg("gdalogr:sieve",raster(),2,0,None)
        output=outputs['dst_filename']
        self.assertTrue(os.path.isfile(output))
        dataset=gdal.Open(output, GA_ReadOnly)
        strhash=hash(str(dataset.ReadAsArray(0).tolist()))
        self.assertEqual(strhash,-1353696889)
        
    def test_gdalogrsieveWithUnsupportedOutputFormat(self):
        outputs=sextante.runalg("gdalogr:sieve",raster(),2,0, SextanteUtils.getTempFilename("img"))
        output=outputs['dst_filename']
        self.assertTrue(os.path.isfile(output))
        dataset=gdal.Open(output, GA_ReadOnly)
        strhash=hash(str(dataset.ReadAsArray(0).tolist()))
        self.assertEqual(strhash,-1353696889)        

def suite():
    suite = unittest.makeSuite(GdalTest, 'test')
    return suite

def runtests():
    result = unittest.TestResult()
    testsuite = suite()
    testsuite.run(result)
    return result