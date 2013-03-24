import sextante
import unittest
from sextante.tests.TestData import points, points2, polygons, polygons2, lines, union,\
    table
from sextante.core.QGisLayers import QGisLayers

class AlgTests(unittest.TestCase):
    
    def test_qgiscountpointsinpolygon(self):
        outputs=sextante.runalg("qgis:countpointsinpolygon",polygons(),points(),"NUMPOINTS",None)
        output=outputs['OUTPUT']
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
        
    def test_qgiscountpointsinpolygonweighted(self):
        outputs=sextante.runalg("qgis:countpointsinpolygonweighted",polygons(),points(),"PT_NUM_A","NUMPOINTS",None)
        output=outputs['OUTPUT']
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
        expectedvalues=["1","1.1","string a","48.4"]
        values=[str(attr.toString()) for attr in attrs]
        self.assertEqual(expectedvalues, values)        
    
    def test_qgiscountuniquepointsinpolygon(self):
        outputs=sextante.runalg("qgis:countuniquepointsinpolygon", polygons(),points(),"PT_ST_A","NUMPOINTS",None)
        output=outputs['OUTPUT']
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
        expectedvalues=["1","1.1","string a","3"]
        values=[str(attr.toString()) for attr in attrs]
        self.assertEqual(expectedvalues, values)

    def test_qgisdistancetonearesthub(self):
        outputs=sextante.runalg("qgis:distancetonearesthub",points(),points2(),"ID",1,0,None)
        output=outputs['SAVENAME']
        layer=QGisLayers.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','PT_NUM_A','PT_ST_A','HubName','HubDist']
        expectedtypes=['Integer','Real','String','String','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=sextante.getfeatures(layer)
        self.assertEqual(12, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","a","8","16.449754410816"]
        values=[str(attr.toString()) for attr in attrs]
        self.assertEqual(expectedvalues, values)

    def test_qgismeancoordinates(self):
        outputs=sextante.runalg("qgis:meancoordinates",union(),"POLY_NUM_A","ID_2",None)
        output=outputs['OUTPUT']
        layer=QGisLayers.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['MEAN_X','MEAN_Y','UID']
        expectedtypes=['Real','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=sextante.getfeatures(layer)
        self.assertEqual(3, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["270814.229197286","4458944.20935905","0"]
        values=[str(attr.toString()) for attr in attrs]
        self.assertEqual(expectedvalues, values)

    def test_qgissumlinelengths(self):
        outputs=sextante.runalg("qgis:sumlinelengths",lines(),polygons(),"LENGTH","COUNT",None)
        output=outputs['OUTPUT']
        layer=QGisLayers.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','POLY_NUM_A','POLY_ST_A','LENGTH','COUNT']
        expectedtypes=['Integer','Real','String','Real','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=sextante.getfeatures(layer)
        self.assertEqual(2, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","string a","56.4157223602428","1"]
        values=[str(attr.toString()) for attr in attrs]
        self.assertEqual(expectedvalues, values)

    def test_qgiscreategrid(self):
        outputs=sextante.runalg("qgis:creategrid",10,10,360,180,0,0,0,None)
        output=outputs['SAVENAME']
        layer=QGisLayers.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['longitude','latitude']
        expectedtypes=['Real','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=sextante.getfeatures(layer)
        self.assertEqual(56, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["-180","0"]
        values=[str(attr.toString()) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='LINESTRING(-180.0 -90.0, -180.0 -80.0, -180.0 -70.0, -180.0 -60.0, -180.0 -50.0, -180.0 -40.0, -180.0 -30.0, -180.0 -20.0, -180.0 -10.0, -180.0 0.0, -180.0 10.0, -180.0 20.0, -180.0 30.0, -180.0 40.0, -180.0 50.0, -180.0 60.0, -180.0 70.0, -180.0 80.0, -180.0 90.0)'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgiscreategridhex(self):
        outputs=sextante.runalg("qgis:creategrid",10,10,360,180,0,0,3,None)
        output=outputs['SAVENAME']
        layer=QGisLayers.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['longitude','latitude']
        expectedtypes=['Real','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=sextante.getfeatures(layer)
        self.assertEqual(718, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["-174.226497308104","-85"]
        values=[str(attr.toString()) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POLYGON((-168.45299462 -85.0,-171.33974596 -90.0,-177.11324865 -90.0,-180.0 -85.0,-177.11324865 -80.0,-171.33974596 -80.0,-168.45299462 -85.0))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_qgispointslayerfromtable(self):
        outputs=sextante.runalg("qgis:pointslayerfromtable",table(),"NUM_A","NUM_A",None)
        output=outputs['OUTPUT']
        layer=QGisLayers.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','NUM_A','ST_A']
        expectedtypes=['Integer','Real','String']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=sextante.getfeatures(layer)
        self.assertEqual(8, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","string a"]
        values=[str(attr.toString()) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POINT(1.1 1.1)'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))
        
def suite():
    suite = unittest.makeSuite(AlgTests, 'test')    
    return suite

def runtests():
    result = unittest.TestResult()    
    testsuite = suite()
    testsuite.run(result)
    return result
    