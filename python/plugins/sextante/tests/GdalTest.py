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
        
    def test_gdalogrwarpreproject(self):
        outputs=sextante.runalg("gdalogr:warpreproject",raster(),"EPSG:23030","EPSG:4326",0,0,"",None)
        output=outputs['OUTPUT']
        self.assertTrue(os.path.isfile(output))
        dataset=gdal.Open(output, GA_ReadOnly)
        strhash=hash(str(dataset.ReadAsArray(0).tolist()))
        self.assertEqual(strhash,-2021328784)        
        
    def test_gdalogrmerge(self):
        outputs=sextante.runalg("gdalogr:merge",raster(),False,False,None)
        output=outputs['OUTPUT']
        self.assertTrue(os.path.isfile(output))
        dataset=gdal.Open(output, GA_ReadOnly)
        strhash=hash(str(dataset.ReadAsArray(0).tolist()))
        self.assertEqual(strhash,-1353696889)
                    
    def test_gdalogrogr2ogr(self):
        outputs=sextante.runalg("gdalogr:ogr2ogr",union(),3,"",None)
        output=outputs['OUTPUT_LAYER']
        layer=QGisLayers.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['id','poly_num_a','poly_st_a','id_2','poly_num_b','poly_st_b']
        expectedtypes=['Integer','Real','String','Integer','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=sextante.getfeatures(layer)
        self.assertEqual(8, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","string a","2","1","string a"]
        values=[str(attr.toString()) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POLYGON((270807.08580285 4458940.1594565,270798.42294527 4458914.62661676,270780.81854858 4458914.21983449,270763.52289518 4458920.715993,270760.3449542 4458926.6570575,270763.78234766 4458958.22561242,270794.30290024 4458942.16424502,270807.08580285 4458940.1594565))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))   
        
    def test_gdalogrogr2ogrWrongExtension(self):
            outputs=sextante.runalg("gdalogr:ogr2ogr",union(),3,"",SextanteUtils.getTempFilename("wrongext"))
            output=outputs['OUTPUT_LAYER']
            layer=QGisLayers.getObjectFromUri(output, True)
            fields=layer.pendingFields()
            expectednames=['id','poly_num_a','poly_st_a','id_2','poly_num_b','poly_st_b']
            expectedtypes=['Integer','Real','String','Integer','Real','String']
            names=[str(f.name()) for f in fields]
            types=[str(f.typeName()) for f in fields]
            self.assertEqual(expectednames, names)
            self.assertEqual(expectedtypes, types)
            features=sextante.getfeatures(layer)
            self.assertEqual(8, len(features))
            feature=features.next()
            attrs=feature.attributes()
            expectedvalues=["1","1.1","string a","2","1","string a"]
            values=[str(attr.toString()) for attr in attrs]
            self.assertEqual(expectedvalues, values)
            wkt='POLYGON((270807.08580285 4458940.1594565,270798.42294527 4458914.62661676,270780.81854858 4458914.21983449,270763.52289518 4458920.715993,270760.3449542 4458926.6570575,270763.78234766 4458958.22561242,270794.30290024 4458942.16424502,270807.08580285 4458940.1594565))'
            self.assertEqual(wkt, str(feature.geometry().exportToWkt()))                       

def suite():
    suite = unittest.makeSuite(GdalTest, 'test')
    return suite

def runtests():
    result = unittest.TestResult()
    testsuite = suite()
    testsuite.run(result)
    return result