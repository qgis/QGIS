# -*- coding: utf-8 -*-
"""QGIS Unit tests for QgsRasterLayerProperties.

.. note:: This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
"""

__author__ = 'Benjamin Jakimow'
__date__ = '14/01/2022'
__copyright__ = 'Copyright 2022, The QGIS Project'

import qgis  # NOQA
import pathlib
import typing
from qgis.PyQt.QtWidgets import QWidget
from qgis.PyQt.QtGui import QIcon
from qgis.gui import QgsMapCanvas, QgsMapLayerConfigWidgetFactory, QgsMapLayerConfigWidget, QgsRasterLayerProperties
from qgis.core import QgsMapLayer, QgsProject, QgsRasterLayer
from utilities import unitTestDataPath
from qgis.testing import start_app, unittest

# Convenience instances in case you may need them
# not used in this test
start_app()


class TestQgsRasterLayerProperties(unittest.TestCase):

    def setUp(self):
        QgsProject.instance().removeAllMapLayers()

    def test_custom_widgets(self):
        """
        This test checks if the apply method of customized QgsMapLayerConfigWidgets will be called
        """

        class MyWidget(QgsMapLayerConfigWidget):
            """
            A custom QgsMapLayerConfigWidget
            """

            COUNT = 0

            def __init__(self, layer: QgsMapLayer, canvas: QgsMapCanvas, parent: QWidget = None):
                super().__init__(layer, canvas, parent=parent)

            def apply(self) -> None:
                MyWidget.COUNT += 1

        class MyFactory(QgsMapLayerConfigWidgetFactory):
            """
            A custom QgsMapLayerConfigWidgetFactory
            """

            COUNT = 0

            def __init__(self, title: str, icon: QIcon):
                super(MyFactory, self).__init__(title, icon)

            def supportsLayer(self, layer):
                return True

            def supportLayerPropertiesDialog(self):
                return True

            def createWidget(self,
                             layer: QgsMapLayer,
                             canvas: QgsMapCanvas,
                             dockWidget: bool = ..., parent:
                             typing.Optional[QWidget] = ...) -> QgsMapLayerConfigWidget:
                MyFactory.COUNT += 1
                w = MyWidget(layer, canvas, parent=parent)
                return w

        myFactory = MyFactory('Dummy Factory', QIcon())
        myCanvas = QgsMapCanvas()
        myPath = pathlib.Path(unitTestDataPath('raster')) / 'band1_float32_noct_epsg4326.tif'
        myRasterLayer = QgsRasterLayer(myPath.as_posix(), myPath.name)

        assert myRasterLayer.isValid(), f'Raster not loaded {myPath}'

        dialog = QgsRasterLayerProperties(myRasterLayer, myCanvas)

        dialog.addPropertiesPageFactory(myFactory)
        dialog.show()

        # this should trigger
        dialog.accept()
        self.assertEqual(MyFactory.COUNT, 1, msg='Custom QgsMapLayerConfigWidget::createWidget(...) not called')
        self.assertEqual(MyWidget.COUNT, 1, msg='Custom QgsMapLayerConfigWidget::apply() not called')


if __name__ == '__main__':
    unittest.main()
