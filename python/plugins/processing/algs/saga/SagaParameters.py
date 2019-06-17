# -*- coding: utf-8 -*-

"""
***************************************************************************
    SagaParameters.py
    ---------------------
    Date                 : December 2018
    Copyright            : (C) 2018 by Nyall Dawson
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
__date__ = 'December 2018'
__copyright__ = '(C) 2018, Nyall Dawson'

from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import QgsProcessingParameterRasterDestination
from processing.core.parameters import getParameterFromString


class SagaImageOutputParam(QgsProcessingParameterRasterDestination):

    """
    Custom raster destination parameter for SAGA algorithms which create a raster image
    output, instead of SAGA's usual 'sdat' raster grid outputs.

    These outputs differ from the usual SAGA outputs and are always generated as TIF files instead
    of sdat.
    """

    def defaultFileExtension(self):
        return 'tif'

    def supportedOutputRasterLayerExtensions(self):
        return ['tif']


class Parameters:

    @staticmethod
    def is_parameter_line(line):
        """
        Returns true if the given line corresponds to a SAGA parameter definition
        """
        return line.startswith('SagaImageOutput') or line.startswith('QgsProcessingParameter') or line.startswith('Parameter') or line.startswith('*QgsProcessingParameter')

    @staticmethod
    def create_parameter_from_line(line):
        """
        Creates a parameter from a definition line.
        """
        if line.startswith('SagaImageOutput'):
            tokens = line.split("|")
            params = [t if str(t) != str(None) else None for t in tokens[1:]]
            if len(params) > 3:
                params[3] = True if params[3].lower() == 'true' else False
            if len(params) > 4:
                params[4] = True if params[4].lower() == 'true' else False
            param = SagaImageOutputParam(*params)
            param.setDescription(QCoreApplication.translate("SAGAAlgorithm", param.description()))
            return param
        else:
            return getParameterFromString(line, "SAGAAlgorithm")
