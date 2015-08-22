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
from PyQt4.QtCore import QCoreApplication

displayNames = {}
classification = {}


def loadClassification():
    global classification
    if not os.path.isfile(classificationFile()):
        return
    lines = open(classificationFile())
    line = lines.readline().strip('\n')
    while line != '':
        tokens = line.split(',')
        subtokens = tokens[1].split('/')
        try:
            classification[tokens[0]] = subtokens
        except:
            raise Exception(line)
        line = lines.readline().strip('\n')
    lines.close()


def loadDisplayNames():
    global displayNames
    if not os.path.isfile(displayNamesFile()):
        return
    lines = open(displayNamesFile())
    line = lines.readline().strip('\n')
    while line != '':
        tokens = line.split(',')
        try:
            displayNames[tokens[0]] = tokens[1]
        except:
            raise Exception(line)
        line = lines.readline().strip('\n')
    lines.close()


def classificationFile():
    return os.path.join(os.path.dirname(__file__), 'algclasssification.txt')


def displayNamesFile():
    return os.path.join(os.path.dirname(__file__), 'algnames.txt')


def getClassificationEn(alg):
    if alg.commandLineName().lower() in classification:
        group, subgroup = classification[alg.commandLineName()]
        return group, subgroup
    else:
        return None, None


def getClassification(alg):
    group, subgroup = getClassificationEn(alg)
    if not group and not subgroup:
        return None, None
    return (QCoreApplication.translate('AlgorithmClassification', group),
            QCoreApplication.translate('AlgorithmClassification', subgroup))


def getDisplayNameEn(alg):
    return alg.name


def getDisplayName(alg):
    return alg.i18n_name if alg.i18n_name else "[" + alg.name + "]"
