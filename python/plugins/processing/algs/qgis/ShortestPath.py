# -*- coding: utf-8 -*-

"""
***************************************************************************
    ShortestPath.py
    ---------------------
    Date                 : November 2016
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
__date__ = 'November 2016'
__copyright__ = '(C) 2016, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.analysis import (QgsLineVectorLayerDirector,
                           QgsDistanceArcProperter,
                           QgsGraphBuilder,
                           QgsGraphAnalyzer
                          )

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import (ParameterVector,
                                        ParameterPoint,
                                        ParameterNumber,
                                        ParameterString,
                                        ParameterTableField,
                                        ParameterSelection
                                       )
from processing.core.outputs import (OutputNumber,
                                     OutputVector
                                    )
from processing.tools import dataobjects

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class ShortestPath(GeoAlgorithm):

    INPUT_VECTOR = 'INPUT_VECTOR'
    START_POINT = 'START_POINT'
    END_POINT = 'END_POINT'
    DIRECTION_FIELD = 'DIRECTION_FIELD'
    VALUE_DIRECT = 'VALUE_DIRECT'
    VALUE_REVERSED = 'VALUE_REVERSED'
    VALUE_BIDIRECTIONAL = 'VALUE_BIDIRECTIONAL'
    DEFAULT_DIRECTION = 'DEFAULT_DIRECTION'
    PATH_LENGTH = 'PATH_LENGTH'
    OUTPUT_LAYER = 'OUTPUT_LAYER'

    def getIcon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'networkanalysis.png'))

    def defineCharacteristics(self):
        self.DIRECTIONS = {1: self.tr('Direct direction'),
                           2: self.tr('Inverse direction'),
                           3: self.tr('Bidirectional')
                          }

        self.UNITS = {0: self.tr('Meters'),
                      1: self.tr('Kilometers')
                     }

        self.name, self.i18n_name = self.trAlgorithm('Shortest path')
        self.group, self.i18n_group = self.trAlgorithm('Network analysis')

        self.addParameter(ParameterVector(self.INPUT_VECTOR,
                                          self.tr('Vector layer representing road network'),
                                          [dataobjects.TYPE_VECTOR_LINE]))
        self.addParameter(ParameterPoint(self.START_POINT,
                                          self.tr('Start point')))
        self.addParameter(ParameterPoint(self.END_POINT,
                                          self.tr('End point')))

        self.addParameter(ParameterTableField(self.DIRECTION_FIELD,
                                              self.tr('Road direction field'),
                                              self.INPUT_VECTOR))
        self.addParameter(ParameterString(self.VALUE_DIRECT,
                                          self.tr('Value for direct direction road'),
                                          ''))
        self.addParameter(ParameterString(self.VALUE_REVERSED,
                                          self.tr('Value for reversed direction road'),
                                          ''))
        self.addParameter(ParameterString(self.VALUE_BIDIRECTIONAL,
                                          self.tr('Value for bidirectional road'),
                                          ''))
        self.addParameter(ParameterSelection(self.DEFAULT_DIRECTION,
                                          self.tr('Default road direction'),
                                          list(self.DIRECTIONS.keys())))

        self.addOutput(OutputNumber(self.PATH_LENGTH,
                                    self.tr('Path length')))
        self.addOutput(OutputVector(self.OUTPUT_LAYER,
                                    self.tr('Shortest path'),
                                    datatype=[dataobjects.TYPE_VECTOR_LINE]))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
                self.getParameterValue(self.INPUT_VECTOR))
        fielName = self.getParameterValue(self.RASTER_BAND)
        columnPrefix = self.getParameterValue(self.COLUMN_PREFIX)
        st = self.getParameterValue(self.STATISTICS)

        vectorLayer = dataobjects.getObjectFromUri(vectorPath)
