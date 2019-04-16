# -*- coding: utf-8 -*-

"""
***************************************************************************
    VariableDistanceBuffer.py
    ---------------------
    Date                 : August 2012
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
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os

from qgis.PyQt.QtGui import QIcon

from qgis.core import (QgsApplication,
                       QgsWkbTypes,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingAlgorithm,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterField,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterFeatureSink)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from . import Buffer as buff

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class VariableDistanceBuffer(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'
    FIELD = 'FIELD'
    SEGMENTS = 'SEGMENTS'
    DISSOLVE = 'DISSOLVE'
    END_CAP_STYLE = 'END_CAP_STYLE'
    JOIN_STYLE = 'JOIN_STYLE'
    MITER_LIMIT = 'MITER_LIMIT'

    def icon(self):
        return QgsApplication.getThemeIcon("/algorithms/mAlgorithmBuffer.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("/algorithms/mAlgorithmBuffer.svg")

    def group(self):
        return self.tr('Vector geometry')

    def groupId(self):
        return 'vectorgeometry'

    def __init__(self):
        super().__init__()

    def flags(self):
        return QgsProcessingAlgorithm.FlagSupportsBatch | QgsProcessingAlgorithm.FlagCanCancel | QgsProcessingAlgorithm.FlagHideFromToolbox

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))

        self.addParameter(QgsProcessingParameterField(self.FIELD,
                                                      self.tr('Distance field'), parentLayerParameterName=self.INPUT))
        self.addParameter(QgsProcessingParameterNumber(self.SEGMENTS,
                                                       self.tr('Segments'), type=QgsProcessingParameterNumber.Integer,
                                                       minValue=1, defaultValue=5))
        self.addParameter(QgsProcessingParameterBoolean(self.DISSOLVE,
                                                        self.tr('Dissolve result'), defaultValue=False))
        self.end_cap_styles = [self.tr('Round'),
                               'Flat',
                               'Square']
        self.addParameter(QgsProcessingParameterEnum(
            self.END_CAP_STYLE,
            self.tr('End cap style'),
            options=self.end_cap_styles, defaultValue=0))
        self.join_styles = [self.tr('Round'),
                            'Miter',
                            'Bevel']
        self.addParameter(QgsProcessingParameterEnum(
            self.JOIN_STYLE,
            self.tr('Join style'),
            options=self.join_styles, defaultValue=0))
        self.addParameter(QgsProcessingParameterNumber(self.MITER_LIMIT,
                                                       self.tr('Miter limit'), type=QgsProcessingParameterNumber.Double,
                                                       minValue=0, defaultValue=2))

        self.addParameter(
            QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Buffer'), QgsProcessing.TypeVectorPolygon))

    def name(self):
        return 'variabledistancebuffer'

    def displayName(self):
        return self.tr('Variable distance buffer')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        dissolve = self.parameterAsBoolean(parameters, self.DISSOLVE, context)
        segments = self.parameterAsInt(parameters, self.SEGMENTS, context)
        end_cap_style = self.parameterAsEnum(parameters, self.END_CAP_STYLE, context) + 1
        join_style = self.parameterAsEnum(parameters, self.JOIN_STYLE, context) + 1
        miter_limit = self.parameterAsDouble(parameters, self.MITER_LIMIT, context)

        field = self.parameterAsString(parameters, self.FIELD, context)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), QgsWkbTypes.Polygon, source.sourceCrs())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        buff.buffering(feedback, context, sink, 0, field, True, source, dissolve, segments, end_cap_style,
                       join_style, miter_limit)

        return {self.OUTPUT: dest_id}
