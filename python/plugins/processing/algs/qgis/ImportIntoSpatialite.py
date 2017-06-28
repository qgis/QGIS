# -*- coding: utf-8 -*-

"""
***************************************************************************
    ImportIntoSpatialite.py
    ---------------------
    Date                 : October 2016
    Copyright            : (C) 2016 by Mathieu Pellerin
    Email                : nirvn dot asia at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Mathieu Pellerin'
__date__ = 'October 2016'
__copyright__ = '(C) 2012, Mathieu Pellerin'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from qgis.core import (QgsDataSourceUri,
                       QgsFeatureSink,
                       QgsVectorLayerExporter,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterVectorLayer,
                       QgsProcessingParameterField,
                       QgsProcessingParameterString,
                       QgsProcessingParameterBoolean,
                       QgsWkbTypes)

from processing.algs.qgis.QgisAlgorithm import QgisAlgorithm
from processing.core.GeoAlgorithmExecutionException import GeoAlgorithmExecutionException
from processing.tools import spatialite


class ImportIntoSpatialite(QgisAlgorithm):

    DATABASE = 'DATABASE'
    TABLENAME = 'TABLENAME'
    INPUT = 'INPUT'
    OVERWRITE = 'OVERWRITE'
    CREATEINDEX = 'CREATEINDEX'
    GEOMETRY_COLUMN = 'GEOMETRY_COLUMN'
    LOWERCASE_NAMES = 'LOWERCASE_NAMES'
    DROP_STRING_LENGTH = 'DROP_STRING_LENGTH'
    FORCE_SINGLEPART = 'FORCE_SINGLEPART'
    PRIMARY_KEY = 'PRIMARY_KEY'
    ENCODING = 'ENCODING'

    def group(self):
        return self.tr('Database')

    def __init__(self):
        super().__init__()
        self.addParameter(QgsProcessingParameterFeatureSource(self.INPUT, self.tr('Layer to import')))
        self.addParameter(QgsProcessingParameterVectorLayer(self.DATABASE, self.tr('File database'), False, False))
        self.addParameter(QgsProcessingParameterString(self.TABLENAME, self.tr('Table to import to (leave blank to use layer name)'), optional=True))
        self.addParameter(QgsProcessingParameterField(self.PRIMARY_KEY, self.tr('Primary key field'), self.INPUT, optional=True))
        self.addParameter(QgsProcessingParameterString(self.GEOMETRY_COLUMN, self.tr('Geometry column'), 'geom'))
        self.addParameter(QgsProcessingParameterString(self.ENCODING, self.tr('Encoding'), 'UTF-8', optional=True))
        self.addParameter(QgsProcessingParameterBoolean(self.OVERWRITE, self.tr('Overwrite'), True))
        self.addParameter(QgsProcessingParameterBoolean(self.CREATEINDEX, self.tr('Create spatial index'), True))
        self.addParameter(QgsProcessingParameterBoolean(self.LOWERCASE_NAMES, self.tr('Convert field names to lowercase'), True))
        self.addParameter(QgsProcessingParameterBoolean(self.DROP_STRING_LENGTH, self.tr('Drop length constraints on character fields'), False))
        self.addParameter(QgsProcessingParameterBoolean(self.FORCE_SINGLEPART, self.tr('Create single-part geometries instead of multi-part'), False))

        self.db = None
        self.overwrite = None
        self.createIndex = None
        self.dropStringLength = None
        self.convertLowerCase = None
        self.forceSinglePart = None
        self.primaryKeyField = None
        self.encoding = None
        self.source = None
        self.table = None
        self.providerName = None
        self.geomColumn = None

    def name(self):
        return 'importintospatialite'

    def displayName(self):
        return self.tr('Import into Spatialite')

    def prepareAlgorithm(self, parameters, context, feedback):
        database = self.parameterAsVectorLayer(parameters, self.DATABASE, context)
        databaseuri = database.dataProvider().dataSourceUri()
        uri = QgsDataSourceUri(databaseuri)
        if uri.database() is '':
            if '|layerid' in databaseuri:
                databaseuri = databaseuri[:databaseuri.find('|layerid')]
            uri = QgsDataSourceUri('dbname=\'%s\'' % (databaseuri))
        self.db = spatialite.GeoDB(uri)

        self.overwrite = self.parameterAsBool(parameters, self.OVERWRITE, context)
        self.createIndex = self.parameterAsBool(parameters, self.CREATEINDEX, context)
        self.convertLowerCase = self.parameterAsBool(parameters, self.LOWERCASE_NAMES, context)
        self.dropStringLength = self.parameterAsBool(parameters, self.DROP_STRING_LENGTH, context)
        self.forceSinglePart = self.parameterAsBool(parameters, self.FORCE_SINGLEPART, context)
        self.primaryKeyField = self.parameterAsString(parameters, self.PRIMARY_KEY, context) or 'id'
        self.encoding = self.parameterAsString(parameters, self.ENCODING, context)

        self.source = self.parameterAsSource(parameters, self.INPUT, context)

        self.table = self.parameterAsString(parameters, self.TABLENAME, context)
        if self.table:
            self.table.strip()
        if not self.table or self.table == '':
            self.table = self.source.sourceName()
            self.table = self.table.replace('.', '_')
        self.table = self.table.replace(' ', '').lower()
        self.providerName = 'spatialite'

        self.geomColumn = self.parameterAsString(parameters, self.GEOMETRY_COLUMN, context)
        if not self.geomColumn:
            self.geomColumn = 'geom'

        return True

    def processAlgorithm(self, context, feedback):
        options = {}
        if self.overwrite:
            options['overwrite'] = True
        if self.convertLowerCase:
            options['lowercaseFieldNames'] = True
            self.geomColumn = self.geomColumn.lower()
        if self.dropStringLength:
            options['dropStringConstraints'] = True
        if self.forceSinglePart:
            options['forceSinglePartGeometryType'] = True

        # Clear geometry column for non-geometry tables
        if self.source.wkbType() == QgsWkbTypes.NoGeometry:
            self.geomColumn = None

        uri = self.db.uri
        uri.setDataSource('', self.table, self.geomColumn, '', self.primaryKeyField)

        if self.encoding:
            options['fileEncoding'] = self.encoding

        exporter = QgsVectorLayerExporter(uri.uri(), self.providerName, self.source.fields(),
                                          self.source.wkbType(), self.source.sourceCrs(), self.overwrite, options)

        if exporter.errorCode() != QgsVectorLayerExporter.NoError:
            raise GeoAlgorithmExecutionException(
                self.tr('Error importing to Spatialite\n{0}').format(exporter.errorMessage()))

        features = self.source.getFeatures()
        total = 100.0 / self.source.featureCount() if self.source.featureCount() else 0
        for current, f in enumerate(features):
            if feedback.isCanceled():
                break

            if not exporter.addFeature(f, QgsFeatureSink.FastInsert):
                feedback.reportError(exporter.errorMessage())

            feedback.setProgress(int(current * total))

        exporter.flushBuffer()
        if exporter.errorCode() != QgsVectorLayerExporter.NoError:
            raise GeoAlgorithmExecutionException(
                self.tr('Error importing to Spatialite\n{0}').format(exporter.errorMessage()))

        if self.geomColumn and self.createIndex:
            self.db.create_spatial_index(self.table, self.geomColumn)
        return True

    def postProcessAlgorithm(self, context, feedback):
        return {}
