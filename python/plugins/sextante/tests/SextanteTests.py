'''Convenience module to create a test suite will all SEXTANTE tests''' 
import unittest
from sextante.tests import QgisAlgsTest
from sextante.tests import ParametersTest
from sextante.tests import ModelerAlgorithmTest
from sextante.tests import SextanteToolsTest
from sextante.tests import ScriptTest
from sextante.tests import SagaTest
from sextante.tests import GeoAlgorithmTest
from sextante.tests import GdalTest

def suite():
    suite = unittest.TestSuite()
    suite.addTests(QgisAlgsTest.suite())
    suite.addTests(ModelerAlgorithmTest.suite())
    suite.addTests(SagaTest.suite())
    suite.addTests(GdalTest.suite())
    suite.addTests(ScriptTest.suite())
    suite.addTests(SextanteToolsTest.suite())    
    #suite.addTests(ParametersTest.suite())
    suite.addTests(GeoAlgorithmTest.suite())
    return suite

def runtests():
    result = unittest.TestResult()
    testsuite = suite()
    testsuite.run(result)
    return result