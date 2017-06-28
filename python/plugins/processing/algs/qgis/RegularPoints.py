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
from builtins import str

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
from qgis.core import (QgsRectangle, QgsFields, QgsFeatureSink, QgsField, QgsFeature, QgsWkbTypes,
                       QgsGeometry, QgsPointXY, QgsCoordinateReferenceSystem,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterNumber,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterDefinition,
                       QgsProcessingOutputVectorLayer)

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
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'regular_points.png'))

    def group(self):
        return self.tr('Vector creation tools')

    def __init__(self):
        super().__init__()
        self.addParameter(QgsProcessingParameterExtent(self.EXTENT,
                                                       self.tr('Input extent'), optional=False))
        self.addParameter(QgsProcessingParameterNumber(self.SPACING,
                                                       self.tr('Point spacing/count'), QgsProcessingParameterNumber.Double, 100, False, 0.000001, 999999999.999999999))
        self.addParameter(QgsProcessingParameterNumber(self.INSET,
                                                       self.tr('Initial inset from corner (LH side)'), QgsProcessingParameterNumber.Double, 0.0, False, 0.0, 9999.9999))
        self.addParameter(QgsProcessingParameterBoolean(self.RANDOMIZE,
                                                        self.tr('Apply random offset to point spacing'), False))
        self.addParameter(QgsProcessingParameterBoolean(self.IS_SPACING,
                                                        self.tr('Use point spacing'), True))
        self.addParameter(QgsProcessingParameterCrs(self.CRS,
                                                    self.tr('Output layer CRS'), 'ProjectCrs'))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Regular points'), QgsProcessingParameterDefinition.TypeVectorPoint))
        self.addOutput(QgsProcessingOutputVectorLayer(self.OUTPUT, self.tr('Regular points'), QgsProcessingParameterDefinition.TypeVectorPoint))

        self.extent = None
        self.spacing = None
        self.inset = None
        self.randomize = None
        self.isSpacing = None
        self.fields = None
        self.sink = None
        self.dest_id = None

    def name(self):
        return 'regularpoints'

    def displayName(self):
        return self.tr('Regular points')

    def prepareAlgorithm(self, parameters, context, feedback):
        self.extent = self.parameterAsExtent(parameters, self.EXTENT, context)

        self.spacing = self.parameterAsDouble(parameters, self.SPACING, context)
        self.inset = self.parameterAsDouble(parameters, self.INSET, context)
        self.randomize = self.parameterAsBool(parameters, self.RANDOMIZE, context)
        self.isSpacing = self.parameterAsBool(parameters, self.IS_SPACING, context)
        crs = self.parameterAsCrs(parameters, self.CRS, context)

        self.fields = QgsFields()
        self.fields.append(QgsField('id', QVariant.Int, '', 10, 0))

        (self.sink, self.dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                                         self.fields, QgsWkbTypes.Point, crs)
        return True

    def processAlgorithm(self, context, feedback):
        if self.randomize:
            seed()

        area = self.extent.width() * self.extent.height()
        if self.isSpacing:
            pSpacing = self.spacing
        else:
            pSpacing = sqrt(area / self.spacing)

        f = QgsFeature()
        f.initAttributes(1)
        f.setFields(self.fields)

        count = 0
        total = 100.0 / (area / pSpacing)
        y = self.extent.yMaximum() - self.inset

        extent_geom = QgsGeometry.fromRect(self.extent)
        extent_engine = QgsGeometry.createGeometryEngine(extent_geom.geometry())
        extent_engine.prepareGeometry()

        while y >= self.extent.yMinimum():
            x = self.extent.xMinimum() + self.inset
            while x <= self.extent.xMaximum():
                if feedback.isCanceled():
                    break

                if self.randomize:
                    geom = QgsGeometry().fromPoint(QgsPointXY(
                        uniform(x - (pSpacing / 2.0), x + (pSpacing / 2.0)),
                        uniform(y - (pSpacing / 2.0), y + (pSpacing / 2.0))))
                else:
                    geom = QgsGeometry().fromPoint(QgsPointXY(x, y))

                if extent_engine.intersects(geom.geometry()):
                    f.setAttribute('id', count)
                    f.setGeometry(geom)
                    self.sink.addFeature(f, QgsFeatureSink.FastInsert)
                    x += pSpacing
                    count += 1
                    feedback.setProgress(int(count * total))
            y = y - pSpacing

        return True

    def postProcessAlgorithm(self, context, feedback):
        return {self.OUTPUT: self.dest_id}
