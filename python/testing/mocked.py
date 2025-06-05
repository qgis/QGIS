"""
***************************************************************************
    mocked
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

__author__ = "Matthias Kuhn"
__date__ = "January 2016"
__copyright__ = "(C) 2016, Matthias Kuhn"

import os
import sys
from unittest import mock

from qgis.gui import QgisInterface, QgsMapCanvas, QgsLayerTreeView
from qgis.core import QgsApplication, QgsProject, QgsLayerTreeModel

from qgis.PyQt.QtWidgets import QMainWindow
from qgis.PyQt.QtCore import QSize

from qgis.testing import start_app


def get_iface():
    """
    Will return a mock QgisInterface object with some methods implemented in a generic way.

    You can further control its behavior
    by using the mock infrastructure. Refer to https://docs.python.org/3/library/unittest.mock.html
    for more details.

        Returns
        -------
        QgisInterface

        A mock QgisInterface
    """

    start_app()

    my_iface = mock.Mock(spec=QgisInterface)

    my_iface.mainWindow.return_value = QMainWindow()

    canvas = QgsMapCanvas(my_iface.mainWindow())
    canvas.resize(QSize(400, 400))
    my_iface.mapCanvas.return_value = canvas

    layer_tree_view = QgsLayerTreeView(my_iface.mainWindow())
    layer_tree_model = QgsLayerTreeModel(
        QgsProject.instance().layerTreeRoot(), layer_tree_view
    )
    layer_tree_view.setModel(layer_tree_model)
    my_iface.layerTreeView.return_value = layer_tree_view

    return my_iface
