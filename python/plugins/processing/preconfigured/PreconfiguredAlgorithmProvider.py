# -*- coding: utf-8 -*-

"""
***************************************************************************
    PreconfiguredAlgorithmProvider.py
    ---------------------
    Date                 : April 2016
    Copyright            : (C) 2016 by Victor Olaya
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
__date__ = 'April 2016'
__copyright__ = '(C) 2016, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from processing.preconfigured.PreconfiguredAlgorithm import PreconfiguredAlgorithm
from processing.preconfigured.PreconfiguredUtils import preconfiguredAlgorithmsFolder
from processing.core.AlgorithmProvider import AlgorithmProvider
from processing.preconfigured.NewPreconfiguredAlgorithmAction import NewPreconfiguredAlgorithmAction
from processing.preconfigured.DeletePreconfiguredAlgorithmAction import DeletePreconfiguredAlgorithmAction


class PreconfiguredAlgorithmProvider(AlgorithmProvider):

    def __init__(self):
        AlgorithmProvider.__init__(self)
        self.contextMenuActions = \
            [NewPreconfiguredAlgorithmAction(), DeletePreconfiguredAlgorithmAction()]

    def _loadAlgorithms(self):
        self.algs = []
        folder = preconfiguredAlgorithmsFolder()
        for path, subdirs, files in os.walk(folder):
            for descriptionFile in files:
                if descriptionFile.endswith('json'):
                    fullpath = os.path.join(path, descriptionFile)
                    alg = PreconfiguredAlgorithm(fullpath)
                    self.algs.append(alg)

    def getIcon(self):
        return QIcon(os.path.join(os.path.dirname(os.path.dirname(__file__)), 'images', 'alg.png'))

    def getName(self):
        return 'preconfigured'

    def getDescription(self):
        return self.tr('Preconfigured algorithms', 'PreconfiguredAlgorithmProvider')
