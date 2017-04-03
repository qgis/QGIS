# -*- coding: utf-8 -*-

"""
***************************************************************************
    removenetwork.py
    ---------------------
    Date                 : February 2017
    Copyright            : (C) 2017 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Alexander Bruy'
__date__ = 'February 2017'
__copyright__ = '(C) 2017, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from osgeo import gdal

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterFile


class RemoveNetwork(GeoAlgorithm):

    NETWORK = 'NETWORK'

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('Remove network')
        self.group, self.i18n_group = self.trAlgorithm('Network analysis')

        self.addParameter(ParameterFile(
            self.NETWORK,
            self.tr('Directory with network'),
            isFolder=True,
            optional=False))

    def processAlgorithm(self, feedback):
        network = self.getParameterValue(self.NETWORK)

        gdal.GetDriverByName('GNMFile').Delete(network)
