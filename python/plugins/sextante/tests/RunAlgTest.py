import sextante
import unittest
from sextante.tests.TestData import points, points2, polygons, polygons2, lines, union,\
    table
from sextante.core.QGisLayers import QGisLayers
from sextante.core.SextanteUtils import SextanteUtils

class ParametrizedTestCase(unittest.TestCase):

    def __init__(self, methodName='runTest', useTempFiles=False):
        super(ParametrizedTestCase, self).__init__(methodName)
        self.useTempFiles = useTempFiles

    @staticmethod
    def parametrize(testcase_klass, useTempFiles):
        testloader = unittest.TestLoader()
        testnames = testloader.getTestCaseNames(testcase_klass)
        suite = unittest.TestSuite()
        for name in testnames:
            suite.addTest(testcase_klass(name, useTempFiles=useTempFiles))
        return suite

class RunAlgTest(ParametrizedTestCase):
    '''This test takes a reduced set of algorithms and executes them in different ways, changing
    parameters such as whether to use temp outputs, the output file format, etc.
    Basically, it uses some algorithms to test other parts of SEXTANTE, not the algorithms themselves'''

    def getOutputFile(self):
        if self.useTempFiles:
            return None
        else:
            return SextanteUtils.getTempFilename('shp')

    def test_qgiscountpointsinpolygon(self):
        outputs=sextante.runalg("qgis:countpointsinpolygon",polygons(),points(),"NUMPOINTS", self.getOutputFile())
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

def suite():
    suite = unittest.TestSuite()
    suite.addTest(ParametrizedTestCase.parametrize(RunAlgTest, False))
    suite.addTest(ParametrizedTestCase.parametrize(RunAlgTest, True))
    return suite

def runtests():
    result = unittest.TestResult()
    testsuite = suite()
    testsuite.run(result)
    return result
