# -*- coding: utf-8 -*-

"""
***************************************************************************
    markerlinesymbollayer.py
    ---------------------
    Date                 : March 2019
    Copyright            : (C) 2019 by Nyall Dawson
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
from qgis._core import (
    QgsMarkerLineSymbolLayer,
    QgsTemplatedLineSymbolLayerBase)


# monkey patch deprecated enum values to maintain API
# TODO - remove for QGIS 4.0
QgsMarkerLineSymbolLayer.Interval = QgsTemplatedLineSymbolLayerBase.Interval
QgsMarkerLineSymbolLayer.Vertex = QgsTemplatedLineSymbolLayerBase.Vertex
QgsMarkerLineSymbolLayer.LastVertex = QgsTemplatedLineSymbolLayerBase.LastVertex
QgsMarkerLineSymbolLayer.FirstVertex = QgsTemplatedLineSymbolLayerBase.FirstVertex
QgsMarkerLineSymbolLayer.CentralPoint = QgsTemplatedLineSymbolLayerBase.CentralPoint
QgsMarkerLineSymbolLayer.CurvePoint = QgsTemplatedLineSymbolLayerBase.CurvePoint
