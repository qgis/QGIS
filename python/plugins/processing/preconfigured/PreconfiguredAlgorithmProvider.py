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

from qgis.core import QgsProcessingProvider
from processing.preconfigured.PreconfiguredAlgorithm import PreconfiguredAlgorithm
from processing.preconfigured.PreconfiguredUtils import preconfiguredAlgorithmsFolder
from processing.preconfigured.NewPreconfiguredAlgorithmAction import NewPreconfiguredAlgorithmAction
from processing.preconfigured.DeletePreconfiguredAlgorithmAction import DeletePreconfiguredAlgorithmAction
from processing.gui.ProviderActions import ProviderContextMenuActions


class PreconfiguredAlgorithmProvider(QgsProcessingProvider):

    def __init__(self):
        super().__init__()
        self.algs = []
        self.contextMenuActions = \
            [NewPreconfiguredAlgorithmAction(), DeletePreconfiguredAlgorithmAction()]

    def loadAlgorithms(self):
        self.algs = []
        folder = preconfiguredAlgorithmsFolder()
        for path, subdirs, files in os.walk(folder):
            for descriptionFile in files:
                if descriptionFile.endswith('json'):
                    fullpath = os.path.join(path, descriptionFile)
                    alg = PreconfiguredAlgorithm(fullpath)
                    self.algs.append(alg)
        for a in self.algs:
            self.addAlgorithm(a)

    def load(self):
        ProviderContextMenuActions.registerProviderContextMenuActions(self.contextMenuActions)
        self.refreshAlgorithms()
        return True

    def unload(self):
        ProviderContextMenuActions.deregisterProviderContextMenuActions(self.contextMenuActions)

    def id(self):
        return 'preconfigured'

    def name(self):
        return self.tr('Preconfigured algorithms', 'PreconfiguredAlgorithmProvider')
