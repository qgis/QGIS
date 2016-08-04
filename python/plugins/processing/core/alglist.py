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

from qgis.PyQt.QtCore import QObject, pyqtSignal


class AlgorithmList(QObject):

    providerAdded = pyqtSignal(str)
    providerRemoved = pyqtSignal(str)
    providerUpdated = pyqtSignal(str)

    # A dictionary of algorithms. Keys are names of providers
    # and values are list with all algorithms from that provider
    algs = {}

    providers = []

    def removeProvider(self, providerName):
        for p in self.providers:
            if p.getName() == providerName:
                self.providers.remove(p)
                break
        if providerName in self.algs:
            del self.algs[providerName]
        self.providerRemoved.emit(providerName)

    def reloadProvider(self, providerName):
        for p in self.providers:
            if p.getName() == providerName:
                p.loadAlgorithms()
                self.algs[p.getName()] = {a.commandLineName(): a for a in p.algs}
                self.providerUpdated.emit(p.getName())
                break

    def addProvider(self, provider):
        self.providers.append(provider)
        self.algs[provider.getName()] = {a.commandLineName(): a for a in provider.algs}
        self.providerAdded.emit(provider.getName())

    def getProviderFromName(self, name):
        for provider in self.providers:
            if provider.getName() == name:
                return provider

    def getAlgorithm(self, name):
        for provider in self.algs.values():
            if name in provider:
                return provider[name]

    def getAlgorithmFromFullName(self, name):
        for provider in self.algs.values():
            for alg in provider.values():
                if alg.name == name:
                    return alg

algList = AlgorithmList()
