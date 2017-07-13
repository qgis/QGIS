# -*- coding: utf-8 -*-

"""
***************************************************************************
    LinesToPolygons.py
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

from qgis.core import (QgsFeature,
                       QgsGeometry,
                       QgsWkbTypes,
                       QgsFeatureSink,
                       QgsProcessing,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingUtils)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import dataobjects, vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class LinesToPolygons(QgisAlgorithm):

    INPUT = 'INPUT'
    OUTPUT = 'OUTPUT'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'to_lines.png'))

    def tags(self):
        return self.tr('line,polygon,convert').split(',')

    def group(self):
        return self.tr('Vector geometry tools')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer'),
                                                              [QgsProcessing.TypeVectorLine]))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                            self.tr('Lines to polygons'),
                                                            QgsProcessing.TypeVectorPolygon))

    def name(self):
        return 'linestopolygons'

    def displayName(self):
        return self.tr('Lines to polygons')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)

        if QgsWkbTypes.isMultiType(source.wkbType()):
            geomType = QgsWkbTypes.MultiPolygon
        else:
            geomType = QgsWkbTypes.Polygon

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               source.fields(), geomType, source.sourceCrs())

        outFeat = QgsFeature()

        total = 100.0 / source.featureCount() if source.featureCount() else 0
        count = 0

        for feat in source.getFeatures():
            if feedback.isCanceled():
                break

            if feat.hasGeometry():
                outGeomList = []
                if feat.geometry().isMultipart():
                    outGeomList = feat.geometry().asMultiPolyline()
                else:
                    outGeomList.append(feat.geometry().asPolyline())

                polyGeom = self.removeBadLines(outGeomList)
                if len(polyGeom) != 0:
                    outFeat.setGeometry(QgsGeometry.fromPolygon(polyGeom))
                    attrs = feat.attributes()
                    outFeat.setAttributes(attrs)
                    sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
            else:
                sink.addFeature(feat, QgsFeatureSink.FastInsert)

            count += 1
            feedback.setProgress(int(count * total))

        return {self.OUTPUT: dest_id}

    def removeBadLines(self, lines):
        geom = []
        if len(lines) == 1:
            if len(lines[0]) > 2:
                geom = lines
            else:
                geom = []
        else:
            geom = [elem for elem in lines if len(elem) > 2]
        return geom
