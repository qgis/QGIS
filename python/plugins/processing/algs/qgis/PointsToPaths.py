# -*- coding: utf-8 -*-

"""
***************************************************************************
    PointsToPaths.py
    ---------------------
    Date                 : April 2014
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
__date__ = 'April 2014'
__copyright__ = '(C) 2014, Alexander Bruy'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from datetime import datetime
from datetime import timedelta

from PyQt4.QtCore import *

from qgis.core import *

from processing import interface
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.ProcessingLog import ProcessingLog
from processing.parameters.ParameterVector import ParameterVector
from processing.parameters.ParameterTableField import ParameterTableField
from processing.parameters.ParameterString import ParameterString
from processing.parameters.ParameterNumber import ParameterNumber
from processing.outputs.OutputVector import OutputVector
from processing.tools import dataobjects, vector

class PointsToPaths(GeoAlgorithm):

    VECTOR = 'VECTOR'
    GROUP_FIELD = 'GROUP_FIELD'
    ORDER_FIELD = 'ORDER_FIELD'
    DATE_FORMAT = 'DATE_FORMAT'
    GAP_PERIOD = 'GAP_PERIOD'
    OUTPUT = 'OUTPUT'

    def defineCharacteristics(self):
        self.name = 'Points to path'
        self.group = 'Vector creation tools'
        self.addParameter(ParameterVector(self.VECTOR,
            'Input point layer', [ParameterVector.VECTOR_TYPE_POINT]))
        self.addParameter(
            ParameterTableField(self.GROUP_FIELD, 'Group field', self.VECTOR))
        self.addParameter(
            ParameterTableField(self.ORDER_FIELD, 'Order field', self.VECTOR))
        self.addParameter(ParameterString(self.DATE_FORMAT,
            'Date format (if order field is DateTime)', '', optional=True))
        self.addParameter(ParameterNumber(
            self.GAP_PERIOD,
            'Gap period (if order field is DateTime)', 0, 60, 0))
        self.addOutput(OutputVector(self.OUTPUT, 'Paths'))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.VECTOR))
        groupField = self.getParameterValue(self.GROUP_FIELD)
        orderField = self.getParameterValue(self.ORDER_FIELD)
        dateFormat = unicode(self.getParameterValue(self.DATE_FORMAT))
        gap = int(self.getParameterValue(self.GAP_PERIOD))

        fields = QgsFields()
        fields.append(QgsField('group', QVariant.String, '', 254, 0))
        fields.append(QgsField('begin', QVariant.String, '', 254, 0))
        fields.append(QgsField('end', QVariant.String, '', 254, 0))
        writer = self.getOutputFromName(self.OUTPUT).getVectorWriter(
            fields, QGis.WKBLineString, layer.dataProvider().crs())

        points = dict()
        features = vector.features(layer)
        total = 100.0 / len(features)
        for count, f in enumerate(features):
            point = f.geometry().asPoint()
            group = f[groupField]
            order = f[orderField]
            if dateFormat != '':
                order = datetime.strptime(unicode(order), dateFormat)
            if group in points:
                points[group].append((order, point))
            else:
                points[group] = [(order, point)]

            progress.setPercentage(int(count * total))

        progress.setPercentage(0)

        count = 0
        total = 100.0 / len(points)
        for k, v in points.iteritems():
            v.sort()
            f = QgsFeature()
            f.initAttributes(len(fields))
            f.setFields(fields)
            f['group'] = k
            f['begin'] = v[0][0]
            f['end'] = v[-1][0]
            line = []
            for node in v:
                line.append(node[1])
            f.setGeometry(QgsGeometry.fromPolyline(line))
            writer.addFeature(f)
            count += 1
            progress.setPercentage(int(count * total))

        del writer
