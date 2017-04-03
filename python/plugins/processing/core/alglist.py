# -*- coding: utf-8 -*-

"""
***************************************************************************
    alglist.py
    ---------------------
    Date                 : May 2016
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
__date__ = 'May 2016'
__copyright__ = '(C) 2016, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import QgsApplication
from qgis.PyQt.QtCore import QObject, pyqtSignal


class AlgorithmList(QObject):

    providerUpdated = pyqtSignal(str)

    # A dictionary of algorithms. Keys are names of providers
    # and values are list with all algorithms from that provider
    algs = {}

    def removeProvider(self, provider_id):
        if provider_id in self.algs:
            del self.algs[provider_id]

        QgsApplication.processingRegistry().removeProvider(provider_id)

    def reloadProvider(self, provider_id):
        for p in QgsApplication.processingRegistry().providers():
            if p.id() == provider_id:
                p.refreshAlgorithms()
                self.algs[p.id()] = {a.commandLineName(): a for a in p.algorithms()}
                self.providerUpdated.emit(p.id())
                break

    def addProvider(self, provider):
        if QgsApplication.processingRegistry().addProvider(provider):
            self.algs[provider.id()] = {a.commandLineName(): a for a in provider.algorithms()}

    def getAlgorithm(self, name):
        for provider in list(self.algs.values()):
            if name in provider:
                return provider[name]


algList = AlgorithmList()
