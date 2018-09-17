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

from qgis.core import (QgsExpression,
                       QgsExpressionContext,
                       QgsFeature,
                       QgsFeatureSink,
                       QgsFields,
                       QgsField,
                       QgsGeometry,
                       QgsDistanceArea,
                       QgsPointXY,
                       QgsLineString,
                       QgsWkbTypes,
                       QgsFeatureRequest,
                       QgsProcessingException,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterExpression,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterField,
                       QgsProcessingParameterString,
                       QgsProcessingFeatureSource,
                       QgsProcessing,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterFolderDestination)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm


class PointsToPaths(QgisAlgorithm):

    INPUT = 'INPUT'
    GROUP_FIELD = 'GROUP_FIELD'
    ORDER_FIELD = 'ORDER_FIELD'
    DATE_FORMAT = 'DATE_FORMAT'
    MVALUE_EXPRESSION = 'MVALUE_EXPRESSION'
    OUTPUT = 'OUTPUT'
    OUTPUT_TEXT_DIR = 'OUTPUT_TEXT_DIR'

    def group(self):
        return self.tr('Vector creation')

    def groupId(self):
        return 'vectorcreation'

    def __init__(self):
        super().__init__()

    def tags(self):
        return self.tr('join,points,lines,connect').split(',')

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input point layer'), [QgsProcessing.TypeVectorPoint]))
        self.addParameter(QgsProcessingParameterField(self.ORDER_FIELD,
                                                      self.tr('Order field'), parentLayerParameterName=self.INPUT))
        self.addParameter(QgsProcessingParameterField(self.GROUP_FIELD,
                                                      self.tr('Group field'), parentLayerParameterName=self.INPUT, optional=True))
        self.addParameter(QgsProcessingParameterString(self.DATE_FORMAT,
                                                       self.tr('Date format (if order field is DateTime)'), optional=True))
        mvalue_param = QgsProcessingParameterExpression(self.MVALUE_EXPRESSION,
                                                        self.tr('M value (if you want an XYM or XYZM output)'), optional=True, parentLayerParameterName=self.INPUT)
        mvalue_param.setFlags(mvalue_param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)
        self.addParameter(mvalue_param)

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Paths'), QgsProcessing.TypeVectorLine))
        output_dir_param = QgsProcessingParameterFolderDestination(self.OUTPUT_TEXT_DIR, self.tr('Directory for text output'), optional=True)
        output_dir_param.setCreateByDefault(False)
        self.addParameter(output_dir_param)

    def name(self):
        return 'pointstopath'

    def displayName(self):
        return self.tr('Points to path')

    def processAlgorithm(self, parameters, context, feedback):
        source = self.parameterAsSource(parameters, self.INPUT, context)
        if source is None:
            raise QgsProcessingException(self.invalidSourceError(parameters, self.INPUT))

        group_field_name = self.parameterAsString(parameters, self.GROUP_FIELD, context)
        order_field_name = self.parameterAsString(parameters, self.ORDER_FIELD, context)
        date_format = self.parameterAsString(parameters, self.DATE_FORMAT, context)
        mvalue_exp = self.parameterAsExpression(parameters, self.MVALUE_EXPRESSION, context)
        text_dir = self.parameterAsString(parameters, self.OUTPUT_TEXT_DIR, context)

        # instantiate the expression if mvalue is set
        if mvalue_exp:
            expression = QgsExpression(mvalue_exp)
            expression_ctx = self.createExpressionContext(parameters, context)
            expression.prepare(expression_ctx)
        else:
            expression = None

        group_field_index = source.fields().lookupField(group_field_name)
        order_field_index = source.fields().lookupField(order_field_name)

        if group_field_index >= 0:
            group_field_def = source.fields().at(group_field_index)
        else:
            group_field_def = None
        order_field_def = source.fields().at(order_field_index)

        fields = QgsFields()
        if group_field_def is not None:
            fields.append(group_field_def)
        begin_field = QgsField(order_field_def)
        begin_field.setName('begin')
        fields.append(begin_field)
        end_field = QgsField(order_field_def)
        end_field.setName('end')
        fields.append(end_field)

        output_wkb = QgsWkbTypes.LineString
        if QgsWkbTypes.hasM(source.wkbType()) or mvalue_exp:
            output_wkb = QgsWkbTypes.addM(output_wkb)
        if QgsWkbTypes.hasZ(source.wkbType()):
            output_wkb = QgsWkbTypes.addZ(output_wkb)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               fields, output_wkb, source.sourceCrs())
        if sink is None:
            raise QgsProcessingException(self.invalidSinkError(parameters, self.OUTPUT))

        points = dict()
        referencedColumns = [group_field_index, order_field_index]
        if expression:
            referencedColumns += expression.referencedAttributeIndexes(source.fields())
        features = source.getFeatures(QgsFeatureRequest().setSubsetOfAttributes(referencedColumns), QgsProcessingFeatureSource.FlagSkipGeometryValidityChecks)
        total = 100.0 / source.featureCount() if source.featureCount() else 0
        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            if not f.hasGeometry():
                continue

            point = f.geometry().constGet().clone()
            if group_field_index >= 0:
                group = f.attributes()[group_field_index]
            else:
                group = 1
            order = f.attributes()[order_field_index]
            if date_format != '':
                order = datetime.strptime(str(order), date_format)
            if group in points:
                points[group].append((order, point, f))
            else:
                points[group] = [(order, point, f)]

            feedback.setProgress(int(current * total))

        feedback.setProgress(0)

        da = QgsDistanceArea()
        da.setSourceCrs(source.sourceCrs(), context.transformContext())
        da.setEllipsoid(context.project().ellipsoid())

        current = 0
        total = 100.0 / len(points) if points else 1
        for group, vertices in list(points.items()):
            if feedback.isCanceled():
                break

            vertices.sort(key=lambda x: (x[0] is None, x[0]))
            f = QgsFeature()
            attributes = []
            if group_field_index >= 0:
                attributes.append(group)
            attributes.extend([vertices[0][0], vertices[-1][0]])
            f.setAttributes(attributes)
            line = [node[1] for node in vertices]

            if text_dir:
                fileName = os.path.join(text_dir, '%s.txt' % group)

                with open(fileName, 'w') as fl:
                    fl.write('angle=Azimuth\n')
                    fl.write('heading=Coordinate_System\n')
                    fl.write('dist_units=Default\n')

                    for i in range(len(line)):
                        if i == 0:
                            fl.write('startAt=%f;%f;90\n' % (line[i].x(), line[i].y()))
                            fl.write('survey=Polygonal\n')
                            fl.write('[data]\n')
                        else:
                            angle = line[i - 1].azimuth(line[i])
                            distance = da.measureLine(QgsPointXY(line[i - 1]), QgsPointXY(line[i]))
                            fl.write('%f;%f;90\n' % (angle, distance))

            linestring = QgsLineString(line)
            if mvalue_exp:
                linestring.addMValue(0)
                for i, node in enumerate(vertices):
                    expression_ctx.setFeature(node[2])
                    mvalue = expression.evaluate(expression_ctx)
                    if expression.hasEvalError():
                        raise QgsProcessingException(
                            self.tr('Evaluation error: {0}').format(expression.evalErrorString()))
                    linestring.setMAt(i, mvalue)
            f.setGeometry(QgsGeometry(linestring))
            sink.addFeature(f, QgsFeatureSink.FastInsert)
            current += 1
            feedback.setProgress(int(current * total))

        return {self.OUTPUT: dest_id}
