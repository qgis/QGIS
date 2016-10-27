# -*- coding: utf-8 -*-

"""
***************************************************************************
    TinInterpolationZValue.py
    ---------------------
    Date                 : October 2016
    Copyright            : (C) 2016 by Alexander Bruy
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
__date__ = 'October 2016'
__copyright__ = '(C) 2016, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import QgsRectangle, QgsWkbTypes
from qgis.analysis import (QgsInterpolator,
                           QgsTINInterpolator,
                           QgsGridFileWriter
                           )

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterSelection
from processing.core.parameters import ParameterNumber
from processing.core.parameters import ParameterExtent
from processing.core.outputs import OutputRaster
from processing.core.outputs import OutputVector
from processing.tools import dataobjects

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class TinInterpolationZValue(GeoAlgorithm):

    INPUT_LAYER = 'INPUT_LAYER'
    LAYER_TYPE = 'LAYER_TYPE'
    METHOD = 'METHOD'
    COLUMNS = 'COLUMNS'
    ROWS = 'ROWS'
    CELLSIZE_X = 'CELLSIZE_X'
    CELLSIZE_Y = 'CELLSIZE_Y'
    EXTENT = 'EXTENT'
    OUTPUT_LAYER = 'OUTPUT_LAYER'
    TRIANULATION_FILE = 'TRIANULATION_FILE'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'interpolation.png'))

    def defineCharacteristics(self):
        self.name, self.i18n_name = self.trAlgorithm('TIN interpolation (using Z-values)')
        self.group, self.i18n_group = self.trAlgorithm('Interpolation')

        self.TYPES = [self.tr('Points'),
                      self.tr('Structure lines'),
                      self.tr('Break lines')
                      ]
        self.METHODS = [self.tr('Linear'),
                        self.tr('Clough-Toucher (cubic)')
                        ]

        self.addParameter(ParameterVector(self.INPUT_LAYER,
                                          self.tr('Vector layer')))
        self.addParameter(ParameterSelection(self.LAYER_TYPE,
                                             self.tr('Type'),
                                             self.TYPES,
                                             0))
        self.addParameter(ParameterSelection(self.METHOD,
                                             self.tr('Interpolation method'),
                                             self.METHODS,
                                             0))
        self.addParameter(ParameterNumber(self.COLUMNS,
                                          self.tr('Number of columns'),
                                          0, 10000000, 300))
        self.addParameter(ParameterNumber(self.ROWS,
                                          self.tr('Number of rows'),
                                          0, 10000000, 300))
        self.addParameter(ParameterNumber(self.CELLSIZE_X,
                                          self.tr('Cellsize X'),
                                          0.0, 999999.000000, 0.0))
        self.addParameter(ParameterNumber(self.CELLSIZE_Y,
                                          self.tr('Cellsize Y'),
                                          0.0, 999999.000000, 0.0))
        self.addParameter(ParameterExtent(self.EXTENT,
                                          self.tr('Extent')))
        self.addOutput(OutputRaster(self.OUTPUT_LAYER,
                                    self.tr('Interpolated')))
        self.addOutput(OutputVector(self.TRIANULATION_FILE,
                                    self.tr('Triangulation'),
                                    ))  # datatype=dataobjects.TYPE_VECTOR_LINE))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.INPUT_LAYER))
        layerType = self.getParameterValue(self.LAYER_TYPE)
        method = self.getParameterValue(self.METHOD)
        columns = self.getParameterValue(self.COLUMNS)
        rows = self.getParameterValue(self.ROWS)
        cellsizeX = self.getParameterValue(self.CELLSIZE_X)
        cellsizeY = self.getParameterValue(self.CELLSIZE_Y)
        extent = self.getParameterValue(self.EXTENT).split(',')
        output = self.getOutputValue(self.OUTPUT_LAYER)
        triangulation = self.getOutputValue(self.TRIANULATION_FILE)

        if not QgsWkbTypes.hasZ(layer.wkbType()):
            raise GeoAlgorithmExecutionException(
                self.tr('Geometries in input layer does not have Z coordinates.'))

        xMin = float(extent[0])
        xMax = float(extent[1])
        yMin = float(extent[2])
        yMax = float(extent[3])
        bbox = QgsRectangle(xMin, yMin, xMax, yMax)

        layerData = QgsInterpolator.LayerData()
        layerData.vectorLayer = layer
        layerData.zCoordInterpolation = True
        layerData.interpolationAttribute = -1

        if layerType == 0:
            layerData.mInputType = QgsInterpolator.POINTS
        elif layerType == 1:
            layerData.mInputType = QgsInterpolator.STRUCTURE_LINES
        else:
            layerData.mInputType = QgsInterpolator.BREAK_LINES

        if method == 0:
            interpolationMethod = QgsTINInterpolator.Linear
        else:
            interpolationMethod = QgsTINInterpolator.CloughTocher

        interpolator = QgsTINInterpolator([layerData], interpolationMethod)
        interpolator.setExportTriangulationToFile(True)
        interpolator.setTriangulationFilePath(triangulation)

        writer = QgsGridFileWriter(interpolator,
                                   output,
                                   bbox,
                                   columns,
                                   rows,
                                   cellsizeX,
                                   cellsizeY)

        writer.writeFile()
