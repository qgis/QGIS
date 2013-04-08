import unittest
import sextante
from sextante.tests.TestData import points, points2, polygons, polygons2, lines, union,\
    table, raster
from sextante.core.QGisLayers import QGisLayers
import os
from osgeo import gdal
from osgeo.gdalconst import GA_ReadOnly
from sextante.modeler import ModelerAlgorithmProvider
from sextante.modeler.ModelerAlgorithm import ModelerAlgorithm

class ModelerAlgorithmTest(unittest.TestCase):

    def testCreateModel(self):
        pass

    def testRemoveAlgorithm(self):
        folder = os.path.join(os.path.dirname(ModelerAlgorithmProvider.__file__), "models")
        modelfile = os.path.join(folder, "noinputs.model")
        model = ModelerAlgorithm()
        model.openModel(modelfile)
        self.assertTrue(2, len(model.algs))
        self.assertFalse(model.removeAlgorithm(0))
        self.assertTrue(model.removeAlgorithm(len(model.algs) - 1));
        outputs = model.execute(None)
        self.assertEquals(2, len(outputs))
        output=outputs['SAVENAME_ALG0']
        layer=QGisLayers.getObjectFromUri(output, True)
        self.assertIsNone(layer)

    def testRemoveParameter(self):
        folder = os.path.join(os.path.dirname(ModelerAlgorithmProvider.__file__), "models")
        modelfile = os.path.join(folder, "watersheds.model")
        model = ModelerAlgorithm()
        model.openModel(modelfile)
        self.assertTrue(2, len(model.parameters))
        self.assertFalse(model.removeParameter(0))
        self.assertTrue(2, len(model.parameters))

    def testComputingDependecies(self):
        folder = os.path.join(os.path.dirname(ModelerAlgorithmProvider.__file__), "models")
        modelfile = os.path.join(folder, "watersheds.model")
        model = ModelerAlgorithm()
        model.openModel(modelfile)
        self.assertTrue(2, len(model.parameters))
        self.assertTrue(5, len(model.algs))
        dependent = model.getDependentAlgorithms(0)
        self.assertEquals([0,1,2,3,4], dependent)
        dependent = model.getDependentAlgorithms(1)
        self.assertEquals([1,2,3,4], dependent)
        dependent = model.getDependentAlgorithms(2)
        self.assertEquals([2,3,4], dependent)
        dependent = model.getDependentAlgorithms(3)
        self.assertEquals([3,4], dependent)
        dependent = model.getDependentAlgorithms(4)
        self.assertEquals([4], dependent)

        depends = model.getDependsOnAlgorithms(0)
        self.assertEquals([], depends)
        depends = model.getDependsOnAlgorithms(1)
        self.assertEquals([0], depends)
        depends = model.getDependsOnAlgorithms(2)
        self.assertEquals([1,0], depends)
        depends = model.getDependsOnAlgorithms(3)
        self.assertEquals([2,1,0], depends)
        depends = model.getDependsOnAlgorithms(4)
        self.assertEquals([3,2,1,0], depends)




    '''The following tests correspond to example models'''

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

    def test_modelernotinorder(self):
        outputs=sextante.runalg("modeler:notinorder",raster(),None)
        output=outputs['CAREA_ALG0']
        self.assertTrue(os.path.isfile(output))
        dataset=gdal.Open(output, GA_ReadOnly)
        strhash=hash(str(dataset.ReadAsArray(0).tolist()))
        self.assertEqual(strhash,-1557050506)


def suite():
    suite = unittest.makeSuite(ModelerAlgorithmTest, 'test')
    return suite

def runtests():
    result = unittest.TestResult()
    testsuite = suite()
    testsuite.run(result)
    return result

