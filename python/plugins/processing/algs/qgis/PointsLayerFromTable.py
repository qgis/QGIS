# -*- coding: utf-8 -*-

"""
***************************************************************************
    PointsLayerFromTable.py
    ---------------------
    Date                 : January 2013
    Copyright            : (C) 2013 by Victor Olaya
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
__date__ = 'August 2013'
__copyright__ = '(C) 2013, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsApplication,
                       QgsWkbTypes,
                       QgsPointV2,
                       QgsCoordinateReferenceSystem,
                       QgsGeometry,
                       QgsProcessingUtils)
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.parameters import ParameterTable
from processing.core.parameters import ParameterTableField
from processing.core.parameters import ParameterCrs
from processing.core.outputs import OutputVector
from processing.tools import dataobjects, vector


class PointsLayerFromTable(GeoAlgorithm):

    INPUT = 'INPUT'
    XFIELD = 'XFIELD'
    YFIELD = 'YFIELD'
    ZFIELD = 'ZFIELD'
    MFIELD = 'MFIELD'
    OUTPUT = 'OUTPUT'
    TARGET_CRS = 'TARGET_CRS'

    def icon(self):
        return QgsApplication.getThemeIcon("/providerQgis.svg")

    def svgIconPath(self):
        return QgsApplication.iconPath("providerQgis.svg")

    def tags(self):
        return self.tr('points,create,values,attributes').split(',')

    def group(self):
        return self.tr('Vector creation tools')

    def name(self):
        return 'createpointslayerfromtable'

    def displayName(self):
        return self.tr('Create points layer from table')

    def defineCharacteristics(self):
        self.addParameter(ParameterTable(self.INPUT,
                                         self.tr('Input layer')))
        self.addParameter(ParameterTableField(self.XFIELD,
                                              self.tr('X field'), self.INPUT, ParameterTableField.DATA_TYPE_ANY))
        self.addParameter(ParameterTableField(self.YFIELD,
                                              self.tr('Y field'), self.INPUT, ParameterTableField.DATA_TYPE_ANY))
        self.addParameter(ParameterTableField(self.ZFIELD,
                                              self.tr('Z field'), self.INPUT, datatype=ParameterTableField.DATA_TYPE_ANY, optional=True))
        self.addParameter(ParameterTableField(self.MFIELD,
                                              self.tr('M field'), self.INPUT, datatype=ParameterTableField.DATA_TYPE_ANY, optional=True))
        self.addParameter(ParameterCrs(self.TARGET_CRS,
                                       self.tr('Target CRS'), 'EPSG:4326'))
        self.addOutput(OutputVector(self.OUTPUT, self.tr('Points from table'), datatype=[dataobjects.TYPE_VECTOR_POINT]))

    def processAlgorithm(self, context, feedback):
        source = self.getParameterValue(self.INPUT)
        vlayer = dataobjects.getLayerFromString(source)
        output = self.getOutputFromName(self.OUTPUT)

        fields = vlayer.fields()
        x_field_index = fields.lookupField(self.getParameterValue(self.XFIELD))
        y_field_index = fields.lookupField(self.getParameterValue(self.YFIELD))
        z_field_index = None
        if self.getParameterValue(self.ZFIELD):
            z_field_index = fields.lookupField(self.getParameterValue(self.ZFIELD))
        m_field_index = None
        if self.getParameterValue(self.MFIELD):
            m_field_index = fields.lookupField(self.getParameterValue(self.MFIELD))

        wkb_type = QgsWkbTypes.Point
        if z_field_index is not None:
            wkb_type = QgsWkbTypes.addZ(wkb_type)
        if m_field_index is not None:
            wkb_type = QgsWkbTypes.addM(wkb_type)

        crsId = self.getParameterValue(self.TARGET_CRS)
        target_crs = QgsCoordinateReferenceSystem()
        target_crs.createFromUserInput(crsId)

        writer = output.getVectorWriter(fields, wkb_type, target_crs)

        features = QgsProcessingUtils.getFeatures(vlayer, context)
        total = 100.0 / QgsProcessingUtils.featureCount(vlayer, context)

        for current, feature in enumerate(features):
            feedback.setProgress(int(current * total))
            attrs = feature.attributes()

            try:
                x = float(attrs[x_field_index])
                y = float(attrs[y_field_index])

                point = QgsPointV2(x, y)

                if z_field_index is not None:
                    try:
                        point.addZValue(float(attrs[z_field_index]))
                    except:
                        point.addZValue(0.0)

                if m_field_index is not None:
                    try:
                        point.addMValue(float(attrs[m_field_index]))
                    except:
                        point.addMValue(0.0)

                feature.setGeometry(QgsGeometry(point))
            except:
                pass  # no geometry

            writer.addFeature(feature)

        del writer
