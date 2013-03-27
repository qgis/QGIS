from sextante.core.Sextante import Sextante
from sextante.modeler.ModelerAlgorithm import ModelerAlgorithm

def testAlg(algname, *args):

    #test simple execution
    alg = Sextante.runAlgorithm(algname, None, *args)
    assert alg is not None

    out = alg.getOutputValuesAsDictionary()

    return out

    #test execution in a model

    #===========================================================================
    # model = ModelerAlgorithm()
    # model.addAlgorithm(alg, parametersMap, valuesMap, outputsMap, dependencies)
    #===========================================================================

    #test