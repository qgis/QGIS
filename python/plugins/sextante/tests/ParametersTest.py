import unittest
from sextante.parameters.ParameterNumber import ParameterNumber
from sextante.parameters.ParameterCrs import ParameterCrs
from sextante.parameters.ParameterExtent import ParameterExtent

class ParametersTest(unittest.TestCase):

    def testParameterNumber(self):
        param = ParameterNumber("name", "desc", 0, 10)
        assert not param.setValue("wrongvalue")
        assert param.value is None
        assert not param.setValue(25)
        assert param.value is None
        assert param.setValue(5)
        assert param.value ==  5
        assert param.setValue(None)
        assert param.value == param.default
        s = param.serialize()
        param2 = ParameterNumber()
        param2.deserialize(s)
        assert param.default == param2.default
        assert param.max == param2.max
        assert param.min == param2.min
        assert param.description == param2.description
        assert param.name == param2.name

    def testParameterCRS(self):
        param = ParameterCrs("name", "desc")
        assert param.setValue("EPSG:12003")
        assert param.value ==  "EPSG:12003"
        assert param.setValue(None)
        assert param.value == param.default
        s = param.serialize()
        param2 = ParameterCrs()
        param2.deserialize(s)
        assert param.default == param2.default
        assert param.description == param2.description
        assert param.name == param2.name

    def testParameterExtent(self):
        param = ParameterExtent("name", "desc")
        assert not param.setValue("0,2,0")
        assert not param.setValue("0,2,0,a")
        assert not param.setValue("0,2,2,4")
        assert param.value ==  "0,2,2,4"
        assert param.setValue(None)
        assert param.value == param.default
        s = param.serialize()
        param2 = ParameterExtent()
        param2.deserialize(s)
        assert param.default == param2.default
        assert param.description == param2.description
        assert param.name == param2.name

def suite():
    suite = unittest.makeSuite(ParametersTest, 'test')
    return suite

def runtests():
    result = unittest.TestResult()
    testsuite = suite()
    testsuite.run(result)
    return result