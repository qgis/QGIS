# -*- coding: utf-8 -*-

"""
***************************************************************************
    SextanteToolbox.py
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
from sextante.core.SextanteUtils import SextanteUtils

__author__ = 'Victor Olaya'
__date__ = 'November 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import os

class AlgorithmDecorator():

    classification = {};

    @staticmethod
    def loadClassification():
        if not os.path.isfile(AlgorithmDecorator.classificationFile()):
            return
        lines = open(AlgorithmDecorator.classificationFile())
        line = lines.readline().strip("\n")
        while line != "":
            tokens = line.split("\t")
            AlgorithmDecorator.classification[tokens[0]] = (tokens[1], tokens[2], tokens[3]);
            line = lines.readline().strip("\n")
        lines.close()

    @staticmethod
    def classificationFile():
        return os.path.join(SextanteUtils.userFolder(), "sextante_qgis_algclass.txt")

    @staticmethod
    def getGroupsAndName(alg):
        if alg.commandLineName() in AlgorithmDecorator.classification:
            group, subgroup, name = AlgorithmDecorator.classification[alg.commandLineName]
            if name == "USE_ORIGINAL_NAME":
                name = alg.name
            return (group, subgroup, name)
        else:
            return ("Uncategorized", alg.group, alg.name)




