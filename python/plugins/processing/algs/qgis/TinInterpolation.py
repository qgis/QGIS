# -*- coding: utf-8 -*-

"""
***************************************************************************
    TinInterpolation.py
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

from qgis.core import QgsRectangle
from qgis.analysis import (QgsInterpolator,
                           QgsTINInterpolator,
                           QgsGridFileWriter
                           )

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.core.parameters import (Parameter,
                                        ParameterNumber,
                                        ParameterExtent,
                                        ParameterSelection,
                                        _splitParameterOptions
                                        )
from processing.core.outputs import (OutputRaster,
                                     OutputVector
                                     )
from processing.tools import dataobjects

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class TinInterpolation(GeoAlgorithm):

    INTERPOLATION_DATA = 'INTERPOLATION_DATA'
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
        self.name, self.i18n_name = self.trAlgorithm('TIN interpolation')
        self.group, self.i18n_group = self.trAlgorithm('Interpolation')

        self.METHODS = [self.tr('Linear'),
                        self.tr('Clough-Toucher (cubic)')
                        ]

        class ParameterInterpolationData(Parameter):
            default_metadata = {
                'widget_wrapper': 'processing.algs.qgis.ui.InterpolationDataWidget.InterpolationDataWidgetWrapper'
            }

            def __init__(self, name='', description=''):
                Parameter.__init__(self, name, description)

            def setValue(self, value):
                if value is None:
                    if not self.optional:
                        return False
                    self.value = None
                    return True

                if value == '':
                    if not self.optional:
                        return False

                if isinstance(value, str):
                    self.value = value if value != '' else None
                else:
                    self.value = ParameterInterpolationData.dataToString(value)
                return True

            def getValueAsCommandLineParameter(self):
                return '"{}"'.format(self.value)

            def getAsScriptCode(self):
                param_type = ''
                param_type += 'interpolation data '
                return '##' + self.name + '=' + param_type

            @classmethod
            def fromScriptCode(self, line):
                isOptional, name, definition = _splitParameterOptions(line)
                descName = _createDescriptiveName(name)
                parent = definition.lower().strip()[len('interpolation data') + 1:]
                return ParameterInterpolationData(name, description, parent)

            @staticmethod
            def dataToString(data):
                s = ''
                for d in data:
                    s += '{}, {}, {:d}, {:d};'.format(c[0],
                                                      c[1],
                                                      c[2],
                                                      c[3])
                return s[:-1]

        self.addParameter(ParameterInterpolationData(self.INTERPOLATION_DATA,
                                                     self.tr('Input layer(s)')))
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
                                          self.tr('Cell size X'),
                                          0.0, 999999.000000, 0.0))
        self.addParameter(ParameterNumber(self.CELLSIZE_Y,
                                          self.tr('Cell size Y'),
                                          0.0, 999999.000000, 0.0))
        self.addParameter(ParameterExtent(self.EXTENT,
                                          self.tr('Extent')))
        self.addOutput(OutputRaster(self.OUTPUT_LAYER,
                                    self.tr('Interpolated')))
        self.addOutput(OutputVector(self.TRIANULATION_FILE,
                                    self.tr('Triangulation'),
                                    ))  # datatype=dataobjects.TYPE_VECTOR_LINE))

    def processAlgorithm(self, feedback):
        interpolationData = self.getParameterValue(self.INTERPOLATION_DATA)
        method = self.getParameterValue(self.METHOD)
        columns = self.getParameterValue(self.COLUMNS)
        rows = self.getParameterValue(self.ROWS)
        cellsizeX = self.getParameterValue(self.CELLSIZE_X)
        cellsizeY = self.getParameterValue(self.CELLSIZE_Y)
        extent = self.getParameterValue(self.EXTENT).split(',')
        output = self.getOutputValue(self.OUTPUT_LAYER)
        triangulation = self.getOutputValue(self.TRIANULATION_FILE)

        if interpolationData is None:
            raise GeoAlgorithmExecutionException(
                self.tr('You need to specify at least one input layer.'))

        xMin = float(extent[0])
        xMax = float(extent[1])
        yMin = float(extent[2])
        yMax = float(extent[3])
        bbox = QgsRectangle(xMin, yMin, xMax, yMax)

        layerData = []
        for row in interpolationData.split(';'):
            v = row.split(',')
            data = QgsInterpolator.LayerData()
            data.vectorLayer = dataobjects.getObjectFromUri(v[0])
            data.zCoordInterpolation = bool(v[1])
            data.interpolationAttribute = int(v[2])
            if v[3] == '0':
                data.mInputType = QgsInterpolator.POINTS
            elif v[3] == '1':
                data.mInputType = QgsInterpolator.STRUCTURE_LINES
            else:
                data.mInputType = QgsInterpolator.BREAK_LINES
            layerData.append(data)

        if method == 0:
            interpolationMethod = QgsTINInterpolator.Linear
        else:
            interpolationMethod = QgsTINInterpolator.CloughTocher

        interpolator = QgsTINInterpolator(layerData, interpolationMethod)
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
