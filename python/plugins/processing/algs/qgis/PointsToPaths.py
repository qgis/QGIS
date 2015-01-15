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

import os
from datetime import datetime
from datetime import timedelta

from PyQt4.QtCore import *

from qgis.core import *

from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.ProcessingLog import ProcessingLog
from processing.core.parameters import ParameterVector
from processing.core.parameters import ParameterTableField
from processing.core.parameters import ParameterString
#from processing.core.parameters import ParameterNumber
from processing.core.outputs import OutputVector
from processing.core.outputs import OutputDirectory
from processing.tools import dataobjects, vector, system


class PointsToPaths(GeoAlgorithm):

    VECTOR = 'VECTOR'
    GROUP_FIELD = 'GROUP_FIELD'
    ORDER_FIELD = 'ORDER_FIELD'
    DATE_FORMAT = 'DATE_FORMAT'
    #GAP_PERIOD = 'GAP_PERIOD'
    OUTPUT_LINES = 'OUTPUT_LINES'
    OUTPUT_TEXT = 'OUTPUT_TEXT'

    def defineCharacteristics(self):
        self.name = 'Points to path'
        self.group = 'Vector creation tools'
        self.addParameter(ParameterVector(self.VECTOR,
            self.tr('Input point layer'), [ParameterVector.VECTOR_TYPE_POINT]))
        self.addParameter(ParameterTableField(self.GROUP_FIELD,
            self.tr('Group field'), self.VECTOR))
        self.addParameter(ParameterTableField(self.ORDER_FIELD,
            self.tr('Order field'), self.VECTOR))
        self.addParameter(ParameterString(self.DATE_FORMAT,
            self.tr('Date format (if order field is DateTime)'), '', optional=True))
        #self.addParameter(ParameterNumber(
        #    self.GAP_PERIOD,
        #    'Gap period (if order field is DateTime)', 0, 60, 0))
        self.addOutput(OutputVector(self.OUTPUT_LINES, self.tr('Paths')))
        self.addOutput(OutputDirectory(self.OUTPUT_TEXT, self.tr('Directory')))

    def processAlgorithm(self, progress):
        layer = dataobjects.getObjectFromUri(
            self.getParameterValue(self.VECTOR))
        groupField = self.getParameterValue(self.GROUP_FIELD)
        orderField = self.getParameterValue(self.ORDER_FIELD)
        dateFormat = unicode(self.getParameterValue(self.DATE_FORMAT))
        #gap = int(self.getParameterValue(self.GAP_PERIOD))
        dirName = self.getOutputValue(self.OUTPUT_TEXT)

        fields = QgsFields()
        fields.append(QgsField('group', QVariant.String, '', 254, 0))
        fields.append(QgsField('begin', QVariant.String, '', 254, 0))
        fields.append(QgsField('end', QVariant.String, '', 254, 0))
        writer = self.getOutputFromName(self.OUTPUT_LINES).getVectorWriter(
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

        da = QgsDistanceArea()

        count = 0
        total = 100.0 / len(points)
        for group, vertices in points.iteritems():
            vertices.sort()
            f = QgsFeature()
            f.initAttributes(len(fields))
            f.setFields(fields)
            f['group'] = group
            f['begin'] = vertices[0][0]
            f['end'] = vertices[-1][0]

            fileName = os.path.join(dirName, '%s.txt' % group)

            fl = open(fileName, 'w')
            fl.write('angle=Azimuth\n')
            fl.write('heading=Coordinate_System\n')
            fl.write('dist_units=Default\n')

            line = []
            i = 0
            for node in vertices:
                line.append(node[1])

                if i == 0:
                    fl.write('startAt=%f;%f;90\n' % (node[1].x(), node[1].y()))
                    fl.write('survey=Polygonal\n')
                    fl.write('[data]\n')
                else:
                    angle = line[i-1].azimuth(line[i])
                    distance = da.measureLine(line[i-1], line[i])
                    fl.write('%f;%f;90\n' % (angle, distance))

                i += 1

            f.setGeometry(QgsGeometry.fromPolyline(line))
            writer.addFeature(f)
            count += 1
            progress.setPercentage(int(count * total))

        del writer
        fl.close()
