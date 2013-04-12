import sextante
import unittest
from sextante.tests.TestData import points, points2, polygons, polygons2, lines, union,\
    table
from sextante.core.QGisLayers import QGisLayers

class ScriptTest(unittest.TestCase):
    '''tests that use scripts'''

    def test_scriptcreatetilingfromvectorlayer(self):
        outputs=sextante.runalg("script:createtilingfromvectorlayer",union(),10,None)
        output=outputs['polygons']
        layer=QGisLayers.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['longitude','latitude']
        expectedtypes=['Real','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=sextante.getfeatures(layer)
        self.assertEqual(10, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["270761.415396242","4458948.29588823"]
        values=[str(attr.toString()) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POLYGON((270755.54427424 4458901.23378639,270755.54427424 4458995.35799007,270767.28651824 4458995.35799007,270767.28651824 4458901.23378639,270755.54427424 4458901.23378639))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_scripthexgridfromlayerbounds(self):
        outputs=sextante.runalg("script:hexgridfromlayerbounds",polygons(),10,None)
        output=outputs['grid']
        layer=QGisLayers.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['longitude','latitude']
        expectedtypes=['Real','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=sextante.getfeatures(layer)
        self.assertEqual(117, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["270765.621834001","4458907.27146471"]
        values=[str(attr.toString()) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POLYGON((270771.39533669 4458907.27146471,270768.50858535 4458902.27146471,270762.73508265 4458902.27146471,270759.84833131 4458907.27146471,270762.73508265 4458912.27146471,270768.50858535 4458912.27146471,270771.39533669 4458907.27146471))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))

    def test_scriptascriptthatreturnsanumber(self):
        outputs=sextante.runalg("script:ascriptthatreturnsanumber")
        output=outputs['number']
        self.assertTrue(10, output)
        
        

def suite():
    suite = unittest.makeSuite(ScriptTest, 'test')
    return suite

def runtests():
    result = unittest.TestResult()
    testsuite = suite()
    testsuite.run(result)
    return result