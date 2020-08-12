# -*- coding: utf-8 -*-

"""
***************************************************************************
    QgisAlgorithm.py
    ----------------
    Date                 : May 2017
    Copyright            : (C) 2017 by Nyall Dawson
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
__date__ = 'May2017'
__copyright__ = '(C) 2017, Nyall Dawson'

from qgis.core import QgsProcessingAlgorithm, QgsProcessingFeatureBasedAlgorithm
from qgis.PyQt.QtCore import QCoreApplication
from processing.algs.help import shortHelp


class QgisAlgorithm(QgsProcessingAlgorithm):

    def __init__(self):
        super().__init__()

    def shortHelpString(self):
        return shortHelp.get(self.id(), None)

    def tr(self, string, context=''):
        if context == '':
            context = self.__class__.__name__
        return QCoreApplication.translate(context, string)

    def trAlgorithm(self, string, context=''):
        if context == '':
            context = self.__class__.__name__
        return string, QCoreApplication.translate(context, string)

    def createInstance(self):
        return type(self)()


class QgisFeatureBasedAlgorithm(QgsProcessingFeatureBasedAlgorithm):

    def __init__(self):
        super().__init__()

    def shortHelpString(self):
        return shortHelp.get(self.id(), None)

    def tr(self, string, context=''):
        if context == '':
            context = self.__class__.__name__
        return QCoreApplication.translate(context, string)

    def trAlgorithm(self, string, context=''):
        if context == '':
            context = self.__class__.__name__
        return string, QCoreApplication.translate(context, string)

    def createInstance(self):
        return type(self)()
