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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'


import os
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.alglist import algList
import json


class PreconfiguredAlgorithm(GeoAlgorithm):

    def __init__(self, descriptionFile):
        self.descriptionFile = descriptionFile
        with open(self.descriptionFile) as f:
            self.description = json.load(f)
        GeoAlgorithm.__init__(self)

    def getCopy(self):
        newone = PreconfiguredAlgorithm(self.descriptionFile)
        newone.outputs = []
        newone.provider = self.provider
        newone.name = self.name
        newone.group = self.group
        return newone

    def commandLineName(self):
        return 'preconfigured:' + os.path.splitext(os.path.basename(self.descriptionFile))[0].lower()

    def defineCharacteristics(self):
        self.name = self.description["name"]
        self.group = self.description["group"]
        self.canRunInBatchMode = False
        self.showInModeler = False

    def execute(self, progress):
        self.alg = algList.getAlgorithm(self.description["algname"]).getCopy()
        for name, value in self.description["parameters"].iteritems():
            self.alg.setParameterValue(name, value)
        for name, value in self.description["outputs"].iteritems():
            self.alg.setOutputValue(name, value)
        self.alg.execute(progress)
        self.outputs = self.alg.outputs
