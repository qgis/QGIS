# -*- coding: utf-8 -*-

"""
***************************************************************************
    SagaAlgorithmBase.py
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

import os

from qgis.PyQt.QtCore import QCoreApplication
from qgis.PyQt.QtGui import QIcon
from qgis.core import QgsProcessingAlgorithm

pluginPath = os.path.normpath(os.path.join(
    os.path.split(os.path.dirname(__file__))[0], os.pardir))


class SagaAlgorithmBase(QgsProcessingAlgorithm):

    def __init__(self):
        super().__init__()

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'saga.png'))

    def createInstance(self, config={}):
        return self.__class__()

    def tr(self, string):
        return QCoreApplication.translate("SAGAAlgorithm", string)
