# -*- coding: utf-8 -*-

"""
***************************************************************************
    Transect.py
    ---------------------
    Date                 : October 2017
    Copyright            : (C) 2017 by Loïc Bartoletti
    Email                : lbartoletti at tuxfamily dot org
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Loïc Bartoletti'
__date__ = 'October 2017'
__copyright__ = '(C) 2017, Loïc Bartoletti'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.PyQt.QtCore import QVariant
from qgis.core import (QgsFeature,
                       QgsFeatureSink,
                       QgsField,
                       QgsGeometry,
                       QgsGeometryUtils,
                       QgsPoint,
                       QgsPointXY,
                       QgsProcessing,
                       QgsProcessingException,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterNumber,
                       QgsWkbTypes
                       )
from processing.algs.qgis.QgisAlgorithm import QgisFeatureBasedAlgorithm
from math import radians, cos, sin

class Transect(QgisFeatureBasedAlgorithm):

    INPUT = 'INPUT'
    DISTANCE = 'DISTANCE'
    ANGLE = 'ANGLE'
    SIDE = 'SIDE'
    OUTPUT = 'OUTPUT'

    def group(self):
        return self.tr('Vector geometry')

    def __init__(self):
        super().__init__()
        self.distance = None
        self.angle = None
        self.side = None

    def initParameters(self, config=None):
        self.sides = [self.tr('Left'),
                      self.tr('Right'),
                      self.tr('Both')]
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterNumber(self.DISTANCE,
                                                       self.tr('Distance'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       defaultValue=5.0))
        self.addParameter(QgsProcessingParameterNumber(self.ANGLE,
                                                       self.tr('Angle'),
                                                       type=QgsProcessingParameterNumber.Double,
                                                       defaultValue=90.0))
        self.addParameter(QgsProcessingParameterEnum(self.SIDE,
                                                     self.tr('Side'), options=self.sides))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT,
                                                            self.tr('Transect')))

    def name(self):
        return 'transect'

    def displayName(self):
        return self.tr('Transect')

    def outputName(self):
        return self.tr('Transect')

    def inputLayerTypes(self):
        return [QgsProcessing.TypeVectorLine]
        
    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        distance = self.parameterAsDouble(parameters, self.DISTANCE, context)
        angle = self.parameterAsDouble(parameters, self.ANGLE, context)
        side = self.parameterAsEnum(parameters, self.SIDE, context)
        fields = source.fields()
        fields.append(QgsField("TR_FID", QVariant.Int))
        fields.append(QgsField("TR_ID",  QVariant.Int))
        fields.append(QgsField("TR_SEGMENT", QVariant.Int))
        fields.append(QgsField("TR_ANGLE", QVariant.Double))
        fields.append(QgsField("TR_LENGTH", QVariant.Double))
        fields.append(QgsField("TR_ORIENT", QVariant.String))
        
        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, QgsWkbTypes.LineString, source.sourceCrs())
        
        features = source.getFeatures()
        total = 100.0 / source.featureCount() if source.featureCount() else 0

        number = 0
        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            if not f.hasGeometry():
                pass
            else:
                input_geometry = f.geometry()
                if input_geometry:
                    if input_geometry.isMultipart():
                        multi = input_geometry.asMultiPolyline()
                    else:
                        multi = [input_geometry.asPolyline()]
                    for id, line in enumerate(multi):
                        length = len(line)
                        if length == 2:
                            feat = QgsFeature()
                            attrs = f.attributes() + [current, number, 1, angle, distance, self.sides[side]]
                            feat.setAttributes(attrs)
                            g = self.calcTransect(line[0], QgsPointXY(QgsGeometryUtils.midpoint(QgsPoint(line[0]), QgsPoint(line[1]))), line[1], distance, side, angle)
                            feat.setGeometry(g)
                            sink.addFeature(feat, QgsFeatureSink.FastInsert)
                            number += 1
                        else:
                            for i in range(length - 2):
                                feat = QgsFeature()
                                attrs = f.attributes() + [current, number, i+1, angle, distance, self.sides[side]]
                                feat.setAttributes(attrs)
                                g = self.calcTransect(line[i], line[i+1], line[i+2], distance, side, angle)
                                feat.setGeometry(g)
                                sink.addFeature(feat, QgsFeatureSink.FastInsert)
                                number += 1
            feedback.setProgress(int(current * total))
        

        return {self.OUTPUT: dest_id}     

    def calcTransect(self, p1, point, p2, length, orientation, angle):
        """
        Return the transect of the point with length, orientation and angle

        :param p1: The first point of the segment
        :param point: Point between p1 and p2
        :param p2: The second point of the segment
        :param length: Length of the station line
        :param orientation: Orientation of the station line
        :param angle: Angle of the station line
        :type p1: QgsPointXY
        :type point: QgsPointXY
        :type p2: QgsPointXY
        :type length: float
        :type orientation: Side
        :type angle: float
        :return: Return the transect
        :rtype: list
        """
        p_l = QgsPointXY() # left point of the line
        p_r = QgsPointXY() # right point of the line

        azimuth = p1.azimuth(p2)
        if orientation == 1 or orientation == 2:
            p_l = QgsPointXY(QgsPoint(point).project(length, angle + azimuth))
            if orientation != 2:
                p_r = point

        if orientation == 0 or orientation == 2:
            p_r = QgsPointXY(QgsPoint(point).project(-length, angle + azimuth))
            if orientation != 2:
                p_l = point

        line = QgsGeometry().fromPolyline([p_l, p_r])
        return line
