# -*- coding: utf-8 -*-

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

__author__ = 'Matthias Kuhn'
__date__ = 'January 2016'
__copyright__ = '(C) 2016, Matthias Kuhn'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = ':%H$'

import os
import sys
import mock

from qgis.gui import QgisInterface, QgsMapCanvas
from qgis.core import QgsApplication

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

    return my_iface
