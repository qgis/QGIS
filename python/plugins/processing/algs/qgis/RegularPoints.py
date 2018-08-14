# -*- coding: utf-8 -*-

"""
***************************************************************************
    RegularPoints.py
    ---------------------
    Date                 : September 2014
    Copyright            : (C) 2014 by Alexander Bruy
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
__date__ = 'September 2014'
__copyright__ = '(C) 2014, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
from random import seed, uniform
from math import sqrt

from qgis.PyQt.QtGui import QIcon
from qgis.PyQt.QtCore import QVariant
from qgis.core import (QgsApplication,
                       QgsFields,
                       QgsFeatureSink,
                       QgsField,
                       QgsFeature,
                       QgsWkbTypes,
                       QgsGeometry,
                       QgsPointXY,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterDistance,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterFeatureSink)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class RegularPoints(QgisAlgorithm):

    EXTENT = 'EXTENT'
    SPACING = 'SPACING'
    INSET = 'INSET'
    RANDOMIZE = 'RANDOMIZE'
    IS_SPACING = 'IS_SPACING'
    OUTPUT = 'OUTPUT'
    CRS = 'CRS'

    def icon(self):
        return QgsApplication.getThemeIcon("/algorithms/mAlgorithmRegularPoints.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("/algorithms/mAlgorithmRegularPoints.svg")

    def group(self):
        return self.tr('Vector creation')

    def groupId(self):
        return 'vectorcreation'

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterExtent(self.EXTENT,
                                                       self.tr('Input extent'), optional=False))
        self.addParameter(QgsProcessingParameterDistance(self.SPACING,
                                                         self.tr('Point spacing/count'), 100, self.CRS, False, 0.000001, 999999999.999999999))
        self.addParameter(QgsProcessingParameterDistance(self.INSET,
                                                         self.tr('Initial inset from corner (LH side)'), 0.0, self.CRS, False, 0.0, 9999.9999))
        self.addParameter(QgsProcessingParameterBoolean(self.RANDOMIZE,
                                                        self.tr('Apply random offset to point spacing'), False))
        self.addParameter(QgsProcessingParameterBoolean(self.IS_SPACING,
                                                        self.tr('Use point spacing'), True))
        self.addParameter(QgsProcessingParameterCrs(self.CRS,
                                                    self.tr('Output layer CRS'), 'ProjectCrs'))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Regular points'), QgsProcessing.TypeVectorPoint))

    def name(self):
        return 'regularpoints'

    def displayName(self):
        return self.tr('Regular points')

    def processAlgorithm(self, parameters, context, feedback):
        spacing = self.parameterAsDouble(parameters, self.SPACING, context)
        inset = self.parameterAsDouble(parameters, self.INSET, context)
        randomize = self.parameterAsBool(parameters, self.RANDOMIZE, context)
        isSpacing = self.parameterAsBool(parameters, self.IS_SPACING, context)
        crs = self.parameterAsCrs(parameters, self.CRS, context)
        extent = self.parameterAsExtent(parameters, self.EXTENT, context, crs)

        fields = QgsFields()
        fields.append(QgsField('id', QVariant.Int, '', 10, 0))

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.Point, crs)
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        if randomize:
            seed()

        area = extent.width() * extent.height()
        if isSpacing:
            pSpacing = spacing
        else:
            pSpacing = sqrt(area / spacing)

        f = QgsFeature()
        f.initAttributes(1)
        f.setFields(fields)

        count = 0
        id = 0
        total = 100.0 / (area / pSpacing)
        y = extent.yMaximum() - inset

        extent_geom = QgsGeometry.fromRect(extent)
        extent_engine = QgsGeometry.createGeometryEngine(extent_geom.constGet())
        extent_engine.prepareGeometry()

        while y >= extent.yMinimum():
            x = extent.xMinimum() + inset
            while x <= extent.xMaximum():
                if feedback.isCanceled():
                    break

                if randomize:
                    geom = QgsGeometry().fromPointXY(QgsPointXY(
                        uniform(x - (pSpacing / 2.0), x + (pSpacing / 2.0)),
                        uniform(y - (pSpacing / 2.0), y + (pSpacing / 2.0))))
                else:
                    geom = QgsGeometry().fromPointXY(QgsPointXY(x, y))

                if extent_engine.intersects(geom.constGet()):
                    f.setAttributes([id])
                    f.setGeometry(geom)
                    sink.addFeature(f, QgsFeatureSink.FastInsert)
                    x += pSpacing
                    id += 1

                count += 1
                feedback.setProgress(int(count * total))

            y = y - pSpacing

        return {self.OUTPUT: dest_id}
