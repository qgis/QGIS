# -*- coding: utf-8 -*-

"""
***************************************************************************
    ModelerOnlyAlgorithmProvider.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4 import QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from sextante.core.AlgorithmProvider import AlgorithmProvider
from sextante.modeler.CalculatorModelerAlgorithm import CalculatorModelerAlgorithm
from sextante.modeler.RasterLayerBoundsAlgorithm import RasterLayerBoundsAlgorithm
from sextante.modeler.VectorLayerBoundsAlgorithm import VectorLayerBoundsAlgorithm

import os.path

class ModelerOnlyAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)

    def getName(self):
        return "modelertools"

    def getDescription(self):
        return "Modeler-only tools"

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/model.png")

    def _loadAlgorithms(self):
        self.algs = [CalculatorModelerAlgorithm(), RasterLayerBoundsAlgorithm(), VectorLayerBoundsAlgorithm()]
        for alg in self.algs:
            alg.provider = self