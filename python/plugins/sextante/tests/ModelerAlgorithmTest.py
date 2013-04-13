# -*- coding: utf-8 -*-

"""
***************************************************************************
    ModelerAlgorithmTest.py
    ---------------------
    Date                 : March 2013
    Copyright            : (C) 2013 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'March 2013'
__copyright__ = '(C) 2013, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

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
from sextante.modeler.Providers import Providers

class ModelerAlgorithmTest(unittest.TestCase):

    def testCreateModel(self):
        pass
    
    def testEditModelParameter(self):
        pass
    
    def testEditModelAlgorithm(self):
        pass

    def testRemoveAlgorithm(self):
        folder = os.path.join(os.path.dirname(ModelerAlgorithmProvider.__file__), "models")
        modelfile = os.path.join(folder, "noinputs.model")
        model = ModelerAlgorithm()
        model.openModel(modelfile)
        model.provider = Providers.providers['model']
        self.assertTrue(2, len(model.algs))
        self.assertFalse(model.removeAlgorithm(0))
        self.assertTrue(model.removeAlgorithm(len(model.algs) - 1));        
        model.execute(None)
        outputs = model.outputs
        self.assertEquals(1, len(outputs))
        output=outputs[0].value
        self.assertTrue(os.path.exists(output))        

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
        
    def test_modeleroptionalfield(self):
        outputs=sextante.runalg("modeler:optionalfield",points(),None)
        output=outputs['OUTPUT_ALG0']
        layer=QGisLayers.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['id','value','area','perim']
        expectedtypes=['Integer','String','Real','Real']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=sextante.getfeatures(layer)
        self.assertEqual(1, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["0","all","3592.818848","230.989919"]
        values=[str(attr.toString()) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POLYGON((270839.46818665 4458921.97813894,270778.60197966 4458935.96883677,270786.54279065 4458980.04784113,270803.15756434 4458983.84880322,270839.65586926 4458983.16267036,270855.74530134 4458940.79948673,270839.46818665 4458921.97813894))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt())) 

    def test_modeleremptystring(self):
        outputs=sextante.runalg("modeler:emptystring",union(),None)
        output=outputs['OUTPUT_LAYER_ALG0']
        layer=QGisLayers.getObjectFromUri(output, True)
        fields=layer.pendingFields()
        expectednames=['ID','POLY_NUM_A','POLY_ST_A','ID_2','POLY_NUM_B','POLY_ST_B','NewField']
        expectedtypes=['Integer','Real','String','Integer','Real','String','Integer']
        names=[str(f.name()) for f in fields]
        types=[str(f.typeName()) for f in fields]
        self.assertEqual(expectednames, names)
        self.assertEqual(expectedtypes, types)
        features=sextante.getfeatures(layer)
        self.assertEqual(8, len(features))
        feature=features.next()
        attrs=feature.attributes()
        expectedvalues=["1","1.1","string a","2","1","string a","10"]
        values=[str(attr.toString()) for attr in attrs]
        self.assertEqual(expectedvalues, values)
        wkt='POLYGON((270807.08580285 4458940.1594565,270798.42294527 4458914.62661676,270780.81854858 4458914.21983449,270763.52289518 4458920.715993,270760.3449542 4458926.6570575,270763.78234766 4458958.22561242,270794.30290024 4458942.16424502,270807.08580285 4458940.1594565))'
        self.assertEqual(wkt, str(feature.geometry().exportToWkt()))      


def suite():
    suite = unittest.makeSuite(ModelerAlgorithmTest, 'test')
    return suite

def runtests():
    result = unittest.TestResult()
    testsuite = suite()
    testsuite.run(result)
    return result

