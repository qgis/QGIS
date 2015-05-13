'''
Tests to ensure that a QGIS installation contains Processing dependencies
and they are correctly configured by default
'''
import unittest
import sys
from processing.algs.saga.SagaUtils import *
from processing.core.ProcessingConfig import ProcessingConfig
from processing.algs.grass.GrassUtils import GrassUtils

class PackageTests(unittest.TestCase):

    def testSaga(self):
        folder = ProcessingConfig.getSetting(SAGA_FOLDER)
        ProcessingConfig.removeSetting(SAGA_FOLDER)
        self.assertEqual("2.1.4", getSagaInstalledVersion(True))
        ProcessingConfig.setSettingValue(SAGA_FOLDER, folder)

    def testGrass(self):
        folder = ProcessingConfig.getSetting(GrassUtils.GRASS_FOLDER)
        ProcessingConfig.removeSetting(GrassUtils.GRASS_FOLDER)
        msg = GrassUtils.checkGrassIsInstalled()
        self.assertIsNone(msg)
        ProcessingConfig.setSettingValue(GrassUtils.GRASS_FOLDER, folder)

def runTests():
    t = unittest.TestLoader().loadTestsFromTestCase(PackageTests)
    unittest.TextTestRunner(verbosity=3, stream=sys.stdout).run(t)