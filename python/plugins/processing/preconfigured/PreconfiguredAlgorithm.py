# -*- coding: utf-8 -*-

"""
***************************************************************************
    PreconfiguredAlgorithm.py
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


import os
from qgis.core import (QgsProcessingAlgorithm,
                       QgsApplication)
from processing.core.GeoAlgorithm import GeoAlgorithm
from copy import deepcopy
import json


class PreconfiguredAlgorithm(GeoAlgorithm):

    def __init__(self, descriptionFile):
        self.descriptionFile = descriptionFile
        with open(self.descriptionFile) as f:
            self.description = json.load(f)
        GeoAlgorithm.__init__(self)

        self._name = self.description["name"]
        self._group = self.description["group"]

    def group(self):
        return self._group

    def displayName(self):
        return self._name

    def name(self):
        return os.path.splitext(os.path.basename(self.descriptionFile))[0].lower()

    def flags(self):
        return QgsProcessingAlgorithm.FlagHideFromModeler

    def execute(self, parameters, context=None, feedback=None, model=None):
        new_parameters = deepcopy(parameters)
        self.alg = QgsApplication.processingRegistry().createAlgorithmById(self.description["algname"])
        for name, value in list(self.description["parameters"].items()):
            new_parameters[name] = value
        for name, value in list(self.description["outputs"].items()):
            self.alg.setOutputValue(name, value)
        self.alg.execute(new_parameters, feedback)
        self.outputs = self.alg.outputs
