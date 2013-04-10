import sextante
import unittest
from sextante.tests.TestData import points, points2, polygons, polygons2, lines, union,\
    table, polygonsGeoJson, raster
from sextante.core.QGisLayers import QGisLayers

class GeoAlgorithmTest(unittest.TestCase):
    pass

def suite():
    suite = unittest.makeSuite(GeoAlgorithmTest, 'test')
    return suite

def runtests():
    result = unittest.TestResult()
    testsuite = suite()
    testsuite.run(result)
    return result
