# -*- coding: utf-8 -*-

"""
***************************************************************************
    ParametersTest
    ---------------------
    Date                 : August 2017
    Copyright            : (C) 2017 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Nyall Dawson'
__date__ = 'August 2017'
__copyright__ = '(C) 2017, Nyall Dawson'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.testing import start_app, unittest
from qgis.core import QgsApplication
from qgis.analysis import QgsNativeAlgorithms

from processing.gui.AlgorithmDialog import AlgorithmDialog
from processing.gui.BatchAlgorithmDialog import BatchAlgorithmDialog
from processing.modeler.ModelerParametersDialog import ModelerParametersDialog
from processing.gui.wrappers import *

start_app()
QgsApplication.processingRegistry().addProvider(QgsNativeAlgorithms())


class AlgorithmDialogTest(unittest.TestCase):

    def testCreation(self):
        alg = QgsApplication.processingRegistry().algorithmById('native:centroids')
        a = AlgorithmDialog(alg)
        self.assertEqual(a.mainWidget.alg, alg)


class WrappersTest(unittest.TestCase):

    def checkConstructWrapper(self, param, expected_wrapper_class):
        alg = QgsApplication.processingRegistry().algorithmById('native:centroids')

        # algorithm dialog
        dlg = AlgorithmDialog(alg)
        wrapper = WidgetWrapperFactory.create_wrapper_from_class(param, dlg)
        self.assertIsNotNone(wrapper)
        self.assertIsInstance(wrapper, expected_wrapper_class)
        self.assertEqual(wrapper.dialog, dlg)
        self.assertIsNotNone(wrapper.widget)

        # batch dialog
        dlg = BatchAlgorithmDialog(alg)
        wrapper = WidgetWrapperFactory.create_wrapper_from_class(param, dlg)
        self.assertIsNotNone(wrapper)
        self.assertIsInstance(wrapper, expected_wrapper_class)
        self.assertEqual(wrapper.dialog, dlg)
        self.assertIsNotNone(wrapper.widget)

        # modeler dialog
        model = QgsProcessingModelAlgorithm()
        dlg = ModelerParametersDialog(alg, model)
        wrapper = WidgetWrapperFactory.create_wrapper_from_class(param, dlg)
        self.assertIsNotNone(wrapper)
        self.assertIsInstance(wrapper, expected_wrapper_class)
        self.assertEqual(wrapper.dialog, dlg)
        self.assertIsNotNone(wrapper.widget)

    def testBoolean(self):
        self.checkConstructWrapper(QgsProcessingParameterBoolean('test'), BooleanWidgetWrapper)

    def testCrs(self):
        self.checkConstructWrapper(QgsProcessingParameterCrs('test'), CrsWidgetWrapper)

    def testExtent(self):
        self.checkConstructWrapper(QgsProcessingParameterExtent('test'), ExtentWidgetWrapper)

    def testPoint(self):
        self.checkConstructWrapper(QgsProcessingParameterPoint('test'), PointWidgetWrapper)

    def testFile(self):
        self.checkConstructWrapper(QgsProcessingParameterFile('test'), FileWidgetWrapper)

    def testMultiInput(self):
        self.checkConstructWrapper(QgsProcessingParameterMultipleLayers('test'), MultipleLayerWidgetWrapper)

    def testRasterInput(self):
        self.checkConstructWrapper(QgsProcessingParameterRasterLayer('test'), RasterWidgetWrapper)

    def testEnum(self):
        self.checkConstructWrapper(QgsProcessingParameterEnum('test'), EnumWidgetWrapper)

    def testString(self):
        self.checkConstructWrapper(QgsProcessingParameterString('test'), StringWidgetWrapper)

    def testExpression(self):
        self.checkConstructWrapper(QgsProcessingParameterExpression('test'), ExpressionWidgetWrapper)

    def testVector(self):
        self.checkConstructWrapper(QgsProcessingParameterVectorLayer('test'), VectorLayerWidgetWrapper)

    def testField(self):
        self.checkConstructWrapper(QgsProcessingParameterField('test'), TableFieldWidgetWrapper)

    def testSource(self):
        self.checkConstructWrapper(QgsProcessingParameterFeatureSource('test'), FeatureSourceWidgetWrapper)

    def testSource(self):
        self.checkConstructWrapper(QgsProcessingParameterBand('test'), BandWidgetWrapper)

    def testMapLayer(self):
        self.checkConstructWrapper(QgsProcessingParameterMapLayer('test'), MapLayerWidgetWrapper)


if __name__ == '__main__':
    unittest.main()
