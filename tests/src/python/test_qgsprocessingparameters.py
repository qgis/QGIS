"""QGIS Unit tests for Processing algorithm runner(s).

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = "David Marteau"
__date__ = "2020-09"
__copyright__ = "Copyright 2020, The QGIS Project"

from processing.core.Processing import Processing
from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtWidgets import QLineEdit

from qgis.core import (
    Qgis,
    QgsApplication,
    QgsProcessingParameterGeometry,
    QgsProcessingParameterString,
    QgsSettings,
    QgsWkbTypes,
)
from qgis.gui import (
    QgsAbstractProcessingParameterWidgetWrapper,
    QgsGui,
    QgsProcessingGuiRegistry,
    QgsProcessingParameterWidgetFactoryInterface,
)
import unittest
from qgis.testing import start_app, QgisTestCase

start_app()


class CustomStringParameterWidgetWrapper(QgsAbstractProcessingParameterWidgetWrapper):
    def __init__(self, *args, **kwds):
        super().__init__(*args, **kwds)
        self._widget = None
        self.setObjectName("CustomWrapper")

    def createWidget(self):
        self._widget = QLineEdit()
        return self._widget

    def setWidgetValue(self, value, context):
        if self._widget is not None and value is not None:
            self._widget.setText(str(value))

    def widgetValue(self, context):
        if self._widget is not None:
            return self._widget.text()
        return None


class CustomStringParameterWidgetFactory(QgsProcessingParameterWidgetFactoryInterface):
    NAME = "MyParameterWidget"

    CREATED_WIDGET_WRAPPERS = 0

    def createWidgetWrapper(self, *args, **kwds):
        w = CustomStringParameterWidgetWrapper(*args, **kwds)
        CustomStringParameterWidgetFactory.CREATED_WIDGET_WRAPPERS += 1
        return w

    def clone(self):
        return CustomStringParameterWidgetFactory()

    def parameterType(self):
        return self.NAME


class TestQgsProcessingParameters(QgisTestCase):

    @classmethod
    def setUpClass(cls):
        """Run before all tests"""
        super().setUpClass()
        QCoreApplication.setOrganizationName("QGIS_Test")
        QCoreApplication.setOrganizationDomain("QGIS_TestPyQgsProcessingParameters.com")
        QCoreApplication.setApplicationName("QGIS_TestPyQgsProcessingParameters")
        QgsSettings().clear()
        Processing.initialize()
        cls.registry = QgsApplication.instance().processingRegistry()

        cls._factory = CustomStringParameterWidgetFactory()
        QgsGui.processingGuiRegistry().addParameterWidgetFactory(cls._factory)

    @classmethod
    def tearDownClass(cls):
        QgsGui.processingGuiRegistry().removeParameterWidgetFactory(cls._factory)

    def test_qgsprocessinggometry(self):  # spellok
        """Test QgsProcessingParameterGeometry initialization"""
        geomtypes = [
            QgsWkbTypes.GeometryType.PointGeometry,
            QgsWkbTypes.GeometryType.PolygonGeometry,
        ]
        param = QgsProcessingParameterGeometry(name="test", geometryTypes=geomtypes)

        types = param.geometryTypes()

        self.assertEqual(param.geometryTypes(), geomtypes)

    def test_widget_wrapper_creation(self):
        """Test creating widget wrapper for custom parameter."""
        # Create parameter

        param = QgsProcessingParameterString("TEST", "custom string", optional=False)

        metadata = param.metadata()
        metadata["widget_wrapper"] = {
            "widget_type": (CustomStringParameterWidgetFactory.NAME)
        }
        param.setMetadata(metadata)
        wrapper_type = Qgis.ProcessingMode.Standard

        # use CustomStringParameterWidgetFactory directly
        self.assertEqual(CustomStringParameterWidgetFactory.CREATED_WIDGET_WRAPPERS, 0)
        wrapper1 = self._factory.createWidgetWrapper(param, wrapper_type)
        self.assertEqual(CustomStringParameterWidgetFactory.CREATED_WIDGET_WRAPPERS, 1)
        self.assertEqual(wrapper1.objectName(), "CustomWrapper")
        self.assertIsInstance(wrapper1, QgsAbstractProcessingParameterWidgetWrapper)
        self.assertIsInstance(wrapper1, CustomStringParameterWidgetWrapper)

        # use the CustomStringParameterWidgetFactory, where we have registered the CustomStringParameterWidgetFactory
        # to
        reg: QgsProcessingGuiRegistry = QgsGui.processingGuiRegistry()

        wrapper2 = reg.createParameterWidgetWrapper(param, wrapper_type)
        self.assertEqual(CustomStringParameterWidgetFactory.CREATED_WIDGET_WRAPPERS, 2)
        self.assertEqual(wrapper2.objectName(), "CustomWrapper")
        self.assertIsInstance(wrapper2, QgsAbstractProcessingParameterWidgetWrapper)

        # this fails, because QgsProcessingGuiRegistry returns the base class
        # QgsAbstractProcessingParameterWidgetWrapper only
        self.assertIsInstance(wrapper2, CustomStringParameterWidgetWrapper)


if __name__ == "__main__":
    unittest.main()
