# -*- coding: utf-8 -*-

"""
***************************************************************************
    GrassProviderPlugin.py
    ---------------------
    Date                 : June 2021
    Copyright            : (C) 2021 by Alexander Bruy
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
__date__ = 'June 2021'
__copyright__ = '(C) 2021, Alexander Bruy'


from qgis.core import QgsApplication, QgsRuntimeProfiler

with QgsRuntimeProfiler.profile('Import GRASS Provider'):
    from grassprovider.Grass7AlgorithmProvider import Grass7AlgorithmProvider


class GrassProviderPlugin:

    def __init__(self):
        self.provider = Grass7AlgorithmProvider()

    def initProcessing(self):
        QgsApplication.processingRegistry().addProvider(self.provider)

    def initGui(self):
        self.initProcessing()

    def unload(self):
        QgsApplication.processingRegistry().removeProvider(self.provider)
