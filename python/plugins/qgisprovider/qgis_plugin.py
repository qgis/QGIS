"""
***************************************************************************
    qgis_plugin.py
    ---------------------
    Date                 : August  26
    Copyright            : (C) 2026 by Nyall Dawson
    Email                : nyall.dawson@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

from qgis.core import QgsApplication, QgsRuntimeProfiler

with QgsRuntimeProfiler.profile("Import QGIS Python Provider"):
    from qgisprovider.qgis_provider import QgisAlgorithmProvider


class QgisProviderPlugin:
    def __init__(self):
        self.provider = QgisAlgorithmProvider()

    def initProcessing(self):
        QgsApplication.processingRegistry().addProvider(self.provider)

    def initGui(self):
        self.initProcessing()

    def unload(self):
        QgsApplication.processingRegistry().removeProvider(self.provider)
