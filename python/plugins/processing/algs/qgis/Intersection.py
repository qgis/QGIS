# -*- coding: utf-8 -*-

"""
***************************************************************************
    Intersection.py
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

from qgis.core import (QgsFeatureRequest,
                       QgsFeature,
                       QgsFields,
                       QgsFeatureSink,
                       QgsGeometry,
                       QgsWkbTypes,
                       QgsProcessingException,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterFeatureSink,
                       QgsSpatialIndex,
                       QgsProcessingParameterField,
                       QgsProcessingUtils)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.tools import vector

pluginPath = os.path.split(os.path.split(os.path.dirname(__file__))[0])[0]


class Intersection(QgisAlgorithm):

    INPUT = 'INPUT'
    OVERLAY = 'OVERLAY'
    OUTPUT = 'OUTPUT'
    INPUT_FIELDS = 'INPUT_FIELDS'
    OVERLAY_FIELDS = 'OVERLAY_FIELDS'

    def icon(self):
        return QIcon(os.path.join(pluginPath, 'images', 'ftools', 'intersect.png'))

    def group(self):
        return self.tr('Vector overlay')

    def __init__(self):
        super().__init__()

    def initAlgorithm(self, config=None):
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT,
                                                              self.tr('Input layer')))
        self.addParameter(QgsProcessingParameterFeatureSource(self.OVERLAY,
                                                              self.tr('Intersection layer')))

        self.addParameter(QgsProcessingParameterField(
            self.INPUT_FIELDS,
            self.tr('Input fields to keep (leave empty to keep all fields)'),
            parentLayerParameterName=self.INPUT,
            optional=True, allowMultiple=True))
        self.addParameter(QgsProcessingParameterField(
            self.OVERLAY_FIELDS,
            self.tr('Intersect fields to keep (leave empty to keep all fields)'),
            parentLayerParameterName=self.OVERLAY,
            optional=True, allowMultiple=True))

        self.addParameter(QgsProcessingParameterFeatureSink(self.OUTPUT, self.tr('Intersection')))

    def name(self):
        return 'intersection'

    def displayName(self):
        return self.tr('Intersection')

    def processAlgorithm(self, parameters, context, feedback):
        sourceA = self.parameterAsSource(parameters, self.INPUT, context)
        sourceB = self.parameterAsSource(parameters, self.OVERLAY, context)

        geomType = QgsWkbTypes.multiType(sourceA.wkbType())

        fieldsA = self.parameterAsFields(parameters, self.INPUT_FIELDS, context)
        fieldsB = self.parameterAsFields(parameters, self.OVERLAY_FIELDS, context)

        fieldListA = QgsFields()
        field_indices_a = []
        if len(fieldsA) > 0:
            for f in fieldsA:
                idxA = sourceA.fields().lookupField(f)
                if idxA >= 0:
                    field_indices_a.append(idxA)
                    fieldListA.append(sourceA.fields()[idxA])
        else:
            fieldListA = sourceA.fields()
            field_indices_a = [i for i in range(0, fieldListA.count())]

        fieldListB = QgsFields()
        field_indices_b = []
        if len(fieldsB) > 0:
            for f in fieldsB:
                idxB = sourceB.fields().lookupField(f)
                if idxB >= 0:
                    field_indices_b.append(idxB)
                    fieldListB.append(sourceB.fields()[idxB])
        else:
            fieldListB = sourceB.fields()
            field_indices_b = [i for i in range(0, fieldListB.count())]

        output_fields = QgsProcessingUtils.combineFields(fieldListA, fieldListB)

        (sink, dest_id) = self.parameterAsSink(parameters, self.OUTPUT, context,
                                               output_fields, geomType, sourceA.sourceCrs())

        outFeat = QgsFeature()
        indexB = QgsSpatialIndex(sourceB.getFeatures(QgsFeatureRequest().setSubsetOfAttributes([]).setDestinationCrs(sourceA.sourceCrs())), feedback)

        total = 100.0 / sourceA.featureCount() if sourceA.featureCount() else 1
        count = 0

        for featA in sourceA.getFeatures(QgsFeatureRequest().setSubsetOfAttributes(field_indices_a)):
            if feedback.isCanceled():
                break

            if not featA.hasGeometry():
                continue

            geom = featA.geometry()
            atMapA = featA.attributes()
            intersects = indexB.intersects(geom.boundingBox())

            request = QgsFeatureRequest().setFilterFids(intersects)
            request.setDestinationCrs(sourceA.sourceCrs())
            request.setSubsetOfAttributes(field_indices_b)

            engine = None
            if len(intersects) > 0:
                # use prepared geometries for faster intersection tests
                engine = QgsGeometry.createGeometryEngine(geom.constGet())
                engine.prepareGeometry()

            for featB in sourceB.getFeatures(request):
                if feedback.isCanceled():
                    break

                tmpGeom = featB.geometry()
                if engine.intersects(tmpGeom.constGet()):
                    out_attributes = [featA.attributes()[i] for i in field_indices_a]
                    out_attributes.extend([featB.attributes()[i] for i in field_indices_b])
                    int_geom = QgsGeometry(geom.intersection(tmpGeom))
                    if int_geom.wkbType() == QgsWkbTypes.Unknown or QgsWkbTypes.flatType(int_geom.wkbType()) == QgsWkbTypes.GeometryCollection:
                        int_com = geom.combine(tmpGeom)
                        int_geom = QgsGeometry()
                        if int_com:
                            int_sym = geom.symDifference(tmpGeom)
                            int_geom = QgsGeometry(int_com.difference(int_sym))
                    if int_geom.isEmpty() or not int_geom.isGeosValid():
                        raise QgsProcessingException(
                            self.tr('GEOS geoprocessing error: One or '
                                    'more input features have invalid '
                                    'geometry.'))
                    try:
                        if QgsWkbTypes.geometryType(int_geom.wkbType()) == QgsWkbTypes.geometryType(geomType):
                            int_geom.convertToMultiType()
                            outFeat.setGeometry(int_geom)
                            outFeat.setAttributes(out_attributes)
                            sink.addFeature(outFeat, QgsFeatureSink.FastInsert)
                    except:
                        raise QgsProcessingException(
                            self.tr('Feature geometry error: One or more '
                                    'output features ignored due to invalid '
                                    'geometry.'))

            count += 1
            feedback.setProgress(int(count * total))

        return {self.OUTPUT: dest_id}
