# -*- coding: utf-8 -*-

"""
***************************************************************************
    AlgorithmClassification.py
    ---------------------
    Date                 : November 2012
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
__date__ = 'November 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os


class AlgorithmDecorator:

    classification = {}

    @staticmethod
    def loadClassification():
        if not os.path.isfile(AlgorithmDecorator.classificationFile()):
            return
        lines = open(AlgorithmDecorator.classificationFile())
        line = lines.readline().strip('\n')
        while line != '':
            tokens = line.split(',')
            subtokens = tokens[2].split('/')
            try:
                AlgorithmDecorator.classification[tokens[0]] = (subtokens[0],
                        subtokens[1], tokens[1])
            except:
                raise Exception(line)
            line = lines.readline().strip('\n')
        lines.close()

    @staticmethod
    def classificationFile():
        return os.path.join(os.path.dirname(__file__), 'algclasssification.txt')

    @staticmethod
    def getGroupsAndName(alg):
        if alg.commandLineName().lower() in AlgorithmDecorator.classification:
            (group, subgroup, name) = \
                AlgorithmDecorator.classification[alg.commandLineName()]
            if name == 'USE_ORIGINAL_NAME':
                name = alg.name
            return (group, subgroup, name)
        else:
            return (None, None, alg.name)
