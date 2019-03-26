# -*- coding: utf-8 -*-

"""
***************************************************************************
    QgisAlgorithmTests.py
    ---------------------
    Date                 : January 2016
    Copyright            : (C) 2016 by Matthias Kuhn
    Email                : matthias@opengis.ch
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Matthias Kuhn'
__date__ = 'January 2016'
__copyright__ = '(C) 2016, Matthias Kuhn'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = ':%H$'

import AlgorithmsTestBase

import nose2
import shutil
import os
import tempfile

from qgis.PyQt.Qt import QDomDocument

from qgis.core import (QgsApplication,
                       QgsProcessing,
                       QgsProcessingAlgorithm,
                       QgsProcessingFeedback,
                       QgsProcessingException,
                       QgsProject,
                       QgsVectorLayer,
                       QgsPrintLayout,
                       QgsReadWriteContext,
                       QgsProcessingContext)
from qgis.analysis import (QgsNativeAlgorithms)
from qgis.testing import start_app, unittest
from processing.tools.dataobjects import createContext
from processing.core.ProcessingConfig import ProcessingConfig
from processing.modeler.ModelerUtils import ModelerUtils


class TestAlg(QgsProcessingAlgorithm):

    def __init__(self):
        super().__init__()

    def name(self):
        return 'testalg'

    def displayName(self):
        return 'testalg'

    def initAlgorithm(self, config=None):
        pass

    def createInstance(self):
        return TestAlg()

    def processAlgorithm(self, parameters, context, feedback):
        raise QgsProcessingException('Exception while processing')
        return {}


class TestQgisAlgorithms(unittest.TestCase, AlgorithmsTestBase.AlgorithmsTest):

    @classmethod
    def setUpClass(cls):
        start_app()
        from processing.core.Processing import Processing
        Processing.initialize()
        ProcessingConfig.setSettingValue(ModelerUtils.MODELS_FOLDER, os.path.join(os.path.dirname(__file__), 'models'))
        QgsApplication.processingRegistry().addProvider(QgsNativeAlgorithms())
        cls.cleanup_paths = []
        cls.in_place_layers = {}
        cls.vector_layer_params = {}
        cls._original_models_folder = ProcessingConfig.getSetting(ModelerUtils.MODELS_FOLDER)

    @classmethod
    def tearDownClass(cls):
        from processing.core.Processing import Processing
        Processing.deinitialize()
        for path in cls.cleanup_paths:
            shutil.rmtree(path)
        ProcessingConfig.setSettingValue(ModelerUtils.MODELS_FOLDER, cls._original_models_folder)

    def test_definition_file(self):
        return 'qgis_algorithm_tests.yaml'

    def testProcessingException(self):
        """
        Test that Python exception is caught when running an alg
        """

        alg = TestAlg()
        context = createContext()
        feedback = QgsProcessingFeedback()
        results, ok = alg.run({}, context, feedback)
        self.assertFalse(ok)

    def testParameterPythonImport(self):
        for t in QgsApplication.processingRegistry().parameterTypes():
            import_string = t.pythonImportString()
            # check that pythonImportString correctly imports
            exec(import_string)
            # and now we should be able to instantiate an object!
            exec('test = {}(\'id\',\'name\')\nself.assertIsNotNone(test)'.format(t.className()))

    def testCreateGridFromLayout(self):

        testDataPath = os.path.join(os.path.dirname(__file__), 'testdata')

        p = QgsProject().instance()
        source = os.path.join(testDataPath, 'lines.gml')
        vl = QgsVectorLayer(source)
        self.assertTrue(vl.isValid())
        p.addMapLayer(vl)

        pl = QgsPrintLayout(p)

        doc = QDomDocument()
        f = open(os.path.join(testDataPath, 'test_layout.qpt'))
        doc.setContent(f.read())
        f.close()

        pl.loadFromTemplate(doc, QgsReadWriteContext())
        layoutname = pl.name()
        mapuuid = pl.referenceMap().uuid()

        lomanager = p.layoutManager()
        lomanager.addLayout(pl)

        alg = QgsApplication.processingRegistry().createAlgorithmById('qgis:createatlasgrid')

        self.assertIsNotNone(alg)

        parameters = {'LAYOUT': layoutname,
                      'MAP': mapuuid,
                      'COVERAGE_LAYER': vl,
                      'COVERAGE_BUFFER': 0,
                      'ONLY_ON_FEATURES': True,
                      'OUTPUT': QgsProcessing.TEMPORARY_OUTPUT}
        context = QgsProcessingContext()
        context.setProject(p)
        feedback = QgsProcessingFeedback()

        results, ok = alg.run(parameters, context, feedback)

        self.assertTrue(ok)

        outlayer = context.getMapLayer(results['OUTPUT'])

        self.assertTrue(outlayer.isValid())
        self.assertEqual(outlayer.featureCount(), 4)

        expectedgeometries = {
            1: 'Polygon ((-1.88946692676713646 0.27437286623174284, 5 0.27437286623174284, 5 5, -1.88946692676713646 5, -1.88946692676713646 0.27437286623174284))',
            2: 'Polygon ((5 0.27437286623174284, 11.88946692676713646 0.27437286623174284, 11.88946692676713646 5, 5 5, 5 0.27437286623174284))',
            3: 'Polygon ((-1.88946692676713646 -4.45125426753651432, 5 -4.45125426753651432, 5 0.27437286623174284, -1.88946692676713646 0.27437286623174284, -1.88946692676713646 -4.45125426753651432))',
            4: 'Polygon ((5 -4.45125426753651432, 11.88946692676713646 -4.45125426753651432, 11.88946692676713646 0.27437286623174284, 5 0.27437286623174284, 5 -4.45125426753651432))'
        }

        for i in range(1, outlayer.featureCount() + 1):
            self.assertEqual(expectedgeometries[i], outlayer.getGeometry(i).asWkt())

        QgsProject.instance().removeMapLayer(vl)

        # Example on PR 9488


if __name__ == '__main__':
    nose2.main()
